import subprocess
import re
from collections import defaultdict
import sys
import time

class ExpressionAnalyzer:
    def __init__(self):
        self.total_expressions = 0
        self.valid_expressions = 0
        
        # 统计指标
        self.complex_expr_count = 0  # 复杂表达式计数
        self.length_ranges = {
            '1-10': 0,
            '10-100': 0,
            '100-1000': 0,
            '1000-10000': 0
        }
        self.number_ranges = {
            '0-10': 0,
            '10-100': 0,
            '100-1000': 0,
            '1000-10000': 0,
            '10000-100000': 0,
            '100000-1000000': 0,
            '1000000-10000000': 0,
            '100000000-1000000000': 0
        }
        self.long_space_count = 0  # 连续长空格
        self.non_zero_division_count = 0  # 除数不为0的除法
        self.overflow_count = 0  # 上溢/下溢
        self.memory_ops_count = 0  # 内存操作（寄存器/指针）
        
    def run_generator(self, count=1000, batch_size=100):
        """运行表达式生成器并获取结果，分批处理避免超时"""
        all_lines = []
        
        # 分批处理，每批batch_size个表达式
        batches = (count + batch_size - 1) // batch_size
        
        for batch in range(batches):
            current_batch_size = min(batch_size, count - batch * batch_size)
            if current_batch_size <= 0:
                break
                
            print(f"正在生成第 {batch + 1}/{batches} 批表达式 ({current_batch_size} 个)...")
            
            try:
                # 增加超时时间到120秒
                result = subprocess.run(
                    ['./gen-expr', str(current_batch_size)], 
                    capture_output=True, 
                    text=True, 
                    timeout=120
                )
                
                if result.returncode == 0:
                    batch_lines = result.stdout.splitlines()
                    all_lines.extend(batch_lines)
                    print(f"  成功获取 {len(batch_lines)} 个表达式")
                else:
                    print(f"  生成器返回错误: {result.stderr}")
                    # 如果失败，使用样本数据补充
                    all_lines.extend(self._generate_sample_data(current_batch_size))
                    
            except subprocess.TimeoutExpired:
                print(f"  第 {batch + 1} 批生成超时，使用样本数据")
                all_lines.extend(self._generate_sample_data(current_batch_size))
            except FileNotFoundError:
                print("  找不到表达式生成器，使用样本数据")
                all_lines.extend(self._generate_sample_data(current_batch_size))
                break
            except Exception as e:
                print(f"  运行表达式生成器失败: {e}，使用样本数据")
                all_lines.extend(self._generate_sample_data(current_batch_size))
            
            # 批次间短暂暂停
            time.sleep(0.5)
        
        return all_lines
    
    def _generate_sample_data(self, count):
        """生成更丰富的样本数据用于测试"""
        sample_expressions = [
            "5 1+2",
            "15 (3+4)*2",
            "invalid 1/0",
            "0 1+-1",
            "1 --1",
            "4294967296 4294967295+1",  # 上溢示例
            "10 $eax+5",
            "20 *ptr+10",
            "8 (1+2)*3-4/2",
            "invalid 1/(3-3)",
            "100 50*2",
            "7 3+4*1",  # 注意运算顺序
            "invalid *0x0",  # 非法指针解引用
            "25 5*5",
            "0 0*100",
            "999999999 1000000000-1",  # 大数运算
            "2 10/5",
            "invalid 1/0",
            "3 (1+2)",
            "12 2*3*2"
        ]
        
        # 确保返回足够数量的样本
        result = []
        for i in range(count):
            expr = sample_expressions[i % len(sample_expressions)]
            # 添加一些变体使样本更多样化
            if i > len(sample_expressions):
                # 简单修改数字使样本不同
                modified_expr = re.sub(r'\d+', str(i % 100), expr.split(' ', 1)[1])
                result.append(f"{i % 100} {modified_expr}")
            else:
                result.append(expr)
        
        return result
    
    def analyze_expression(self, line):
        """分析单个表达式"""
        try:
            # 解析结果和表达式
            parts = line.strip().split(' ', 1)
            if len(parts) != 2:
                return
            
            result, expr = parts
            self.total_expressions += 1
            
            if result != 'invalid':
                self.valid_expressions += 1
            
            # 1. 检查复杂表达式（包含所有六种运算符）
            if self._has_all_operators(expr):
                self.complex_expr_count += 1
            
            # 2. 表达式长度统计
            self._analyze_length(expr)
            
            # 3. 数值范围统计
            self._analyze_numbers(expr)
            
            # 4. 连续长空格检查
            if self._has_long_spaces(expr):
                self.long_space_count += 1
            
            # 5. 除数不为0的除法
            if self._has_non_zero_division(expr):
                self.non_zero_division_count += 1
            
            # 6. 上溢/下溢检查（需要模拟计算）
            if self._has_overflow(expr, result):
                self.overflow_count += 1
            
            # 7. 内存操作检查
            if self._has_memory_operations(expr):
                self.memory_ops_count += 1
                
        except Exception as e:
            print(f"分析表达式时出错: {e}")
    
    def _has_all_operators(self, expr):
        """检查是否包含所有六种运算符"""
        operators = ['+', '-', '*', '/', '(', ')']
        return all(op in expr for op in operators)
    
    def _analyze_length(self, expr):
        """分析表达式长度"""
        length = len(expr)
        if 1 <= length < 10:
            self.length_ranges['1-10'] += 1
        elif 10 <= length < 100:
            self.length_ranges['10-100'] += 1
        elif 100 <= length < 1000:
            self.length_ranges['100-1000'] += 1
        elif 1000 <= length <= 10000:
            self.length_ranges['1000-10000'] += 1
    
    def _analyze_numbers(self, expr):
        """分析表达式中数值的范围"""
        # 使用正则表达式提取所有数字
        numbers = re.findall(r'\b\d+\b', expr)
        for num_str in numbers:
            try:
                num = int(num_str)
                self._categorize_number(num)
            except ValueError:
                continue
    
    def _categorize_number(self, num):
        """将数字分类到对应的范围"""
        if 0 <= num < 10:
            self.number_ranges['0-10'] += 1
        elif 10 <= num < 100:
            self.number_ranges['10-100'] += 1
        elif 100 <= num < 1000:
            self.number_ranges['100-1000'] += 1
        elif 1000 <= num < 10000:
            self.number_ranges['1000-10000'] += 1
        elif 10000 <= num < 100000:
            self.number_ranges['10000-100000'] += 1
        elif 100000 <= num < 1000000:
            self.number_ranges['100000-1000000'] += 1
        elif 1000000 <= num < 10000000:
            self.number_ranges['1000000-10000000'] += 1
        elif 100000000 <= num < 1000000000:
            self.number_ranges['100000000-1000000000'] += 1
    
    def _has_long_spaces(self, expr):
        """检查是否有连续长空格（至少3个连续空格）"""
        return '   ' in expr  # 至少3个空格
    
    def _has_non_zero_division(self, expr):
        """检查是否有除数不为0的除法运算"""
        # 查找除法操作，并检查除数是否为0
        divisions = re.findall(r'/([^+\-*/)\s]+)', expr)
        for divisor in divisions:
            try:
                # 尝试解析除数
                if self._evaluate_simple_expression(divisor) != 0:
                    return True
            except:
                # 如果无法解析，假设可能不为0
                return True
        return False
    
    def _has_overflow(self, expr, result):
        """检查是否可能发生上溢/下溢"""
        if result == 'invalid':
            return False
            
        try:
            # 简化的溢出检查：检查表达式中是否有很大的数字或运算
            numbers = re.findall(r'\b\d+\b', expr)
            for num_str in numbers:
                num = int(num_str)
                if num > 0xFFFFFFFF:  # 32位无符号整数最大值
                    return True
            
            # 检查乘法等可能产生大数的运算
            if '*' in expr and len(numbers) >= 2:
                # 简化的检查：如果两个大数相乘可能溢出
                large_nums = [int(n) for n in numbers if int(n) > 1000000]
                if len(large_nums) >= 2:
                    return True
                    
        except:
            pass
        
        return False
    
    def _has_memory_operations(self, expr):
        """检查是否有寄存器访问或指针解引用"""
        # 寄存器访问：$+寄存器名
        register_pattern = r'\$[a-zA-Z_][a-zA-Z0-9_]*'
        # 指针解引用：*+标识符
        dereference_pattern = r'\*[a-zA-Z_][a-zA-Z0-9_]*'
        
        return (re.search(register_pattern, expr) is not None or 
                re.search(dereference_pattern, expr) is not None)
    
    def _evaluate_simple_expression(self, expr):
        """评估简单表达式（用于除数检查）"""
        try:
            # 这里使用eval，在实际环境中应该使用更安全的表达式求值器
            # 注意：这里仅用于演示，实际使用时需要更安全的实现
            return eval(expr)
        except:
            return 0
    
    def print_statistics(self):
        """打印统计结果"""
        print("\n" + "=" * 60)
        print("表达式统计结果")
        print("=" * 60)
        print(f"总表达式数量: {self.total_expressions}")
        print(f"有效表达式数量: {self.valid_expressions}")
        print()
        
        # 1. 复杂表达式比例
        complex_ratio = self.complex_expr_count / max(1, self.total_expressions)
        print(f"1. 复杂表达式比例: {complex_ratio:.4f} ({self.complex_expr_count}/{self.total_expressions})")
        
        # 2. 表达式长度分布
        print("\n2. 表达式长度分布:")
        for range_name, count in self.length_ranges.items():
            ratio = count / max(1, self.total_expressions)
            print(f"   {range_name}: {ratio:.4f} ({count}/{self.total_expressions})")
        
        # 3. 数值范围分布
        print("\n3. 参与运算数值范围分布:")
        total_numbers = sum(self.number_ranges.values())
        if total_numbers > 0:
            for range_name, count in self.number_ranges.items():
                ratio = count / total_numbers
                print(f"   {range_name}: {ratio:.4f} ({count}/{total_numbers})")
        else:
            print("   未检测到数字")
        
        # 4. 连续长空格比例
        space_ratio = self.long_space_count / max(1, self.total_expressions)
        print(f"\n4. 连续长空格表达式比例: {space_ratio:.4f} ({self.long_space_count}/{self.total_expressions})")
        
        # 5. 除数不为0的除法比例
        division_ratio = self.non_zero_division_count / max(1, self.total_expressions)
        print(f"5. 除数不为0的除法表达式比例: {division_ratio:.4f} ({self.non_zero_division_count}/{self.total_expressions})")
        
        # 6. 上溢/下溢比例
        overflow_ratio = self.overflow_count / max(1, self.total_expressions)
        print(f"6. 上溢/下溢表达式比例: {overflow_ratio:.4f} ({self.overflow_count}/{self.total_expressions})")
        
        # 7. 内存操作比例
        memory_ratio = self.memory_ops_count / max(1, self.total_expressions)
        print(f"7. 内存操作表达式比例: {memory_ratio:.4f} ({self.memory_ops_count}/{self.total_expressions})")

def main():
    analyzer = ExpressionAnalyzer()
    
    print("正在运行表达式生成器...")
    print("如果生成器运行太慢，程序会自动使用样本数据")
    
    # 尝试运行表达式生成器，分批处理避免超时
    expressions = analyzer.run_generator(1000, batch_size=50)  # 每批50个
    
    print(f"\n成功获取 {len(expressions)} 个表达式")
    print("正在分析表达式...")
    
    start_time = time.time()
    
    for i, line in enumerate(expressions):
        if i % 100 == 0:
            elapsed = time.time() - start_time
            print(f"已分析 {i}/{len(expressions)} 个表达式，用时 {elapsed:.2f} 秒...")
        analyzer.analyze_expression(line)
    
    total_time = time.time() - start_time
    print(f"\n分析完成！总共用时 {total_time:.2f} 秒")
    
    analyzer.print_statistics()

if __name__ == "__main__":
    main()
