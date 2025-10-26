import subprocess
import re
import time
import sys
import os
from collections import defaultdict

class ExpressionValidator:
    def __init__(self):
        self.total_expressions = 0
        self.correct_expressions = 0
        self.incorrect_expressions = []
        self.unknown_expressions = []  # 新增：记录未知结果
        
    def evaluate_with_calc(self, expr):
        """使用calc程序计算表达式 - 处理过长表达式无输出的情况"""
        try:
            # 启动calc进程
            process = subprocess.Popen(
                ['./calc'],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            
            # 发送表达式并立即刷新
            process.stdin.write(expr + "\n")
            process.stdin.flush()
            
            # 等待一小段时间让calc开始处理
            time.sleep(0.05)
            
            # 关闭stdin发送EOF
            process.stdin.close()
            
            # 设置读取输出的超时
            stdout_data = ""
            stderr_data = ""
            start_time = time.time()
            timeout = 2  # 2秒超时
            
            # 循环读取输出，直到进程结束或超时
            while process.poll() is None and (time.time() - start_time) < timeout:
                # 尝试读取stdout
                try:
                    chunk = process.stdout.read()
                    if chunk:
                        stdout_data += chunk
                except:
                    pass
                
                # 尝试读取stderr
                try:
                    chunk = process.stderr.read()
                    if chunk:
                        stderr_data += chunk
                except:
                    pass
                
                # 短暂休眠避免过度占用CPU
                time.sleep(0.01)
            
            # 如果进程还在运行，强制终止
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=1)
                except:
                    process.kill()
                return None, "计算超时", stdout_data, stderr_data
            
            # 进程已结束，尝试读取剩余输出
            try:
                remaining_stdout, remaining_stderr = process.communicate(timeout=0.5)
                stdout_data += remaining_stdout if remaining_stdout else ""
                stderr_data += remaining_stderr if remaining_stderr else ""
            except:
                pass
            
            # 检查是否有任何输出（包括标准输出和标准错误）
            has_stdout = bool(stdout_data and stdout_data.strip())
            has_stderr = bool(stderr_data and stderr_data.strip())
            
            # 关键修改：如果没有输出，标记为未知
            if not has_stdout and not has_stderr:
                return "unknown", "无任何输出", stdout_data, stderr_data
            
            # 处理输出 - 按行分割并过滤空行
            stdout_lines = [line.strip() for line in stdout_data.strip().split('\n') if line.strip()]
            stderr_lines = [line.strip() for line in stderr_data.strip().split('\n') if line.strip()]
            
            # 解析结果
            if has_stderr:
                # 有标准错误输出，视为非法表达式
                return "invalid", None, stdout_data, stderr_data
            elif len(stdout_lines) == 0:
                # 无标准输出，视为非法表达式
                return "invalid", None, stdout_data, stderr_data
            elif len(stdout_lines) == 1:
                # 只有一行标准输出
                try:
                    # 尝试解析为数字
                    calc_result = int(stdout_lines[0])
                    return calc_result, None, stdout_data, stderr_data
                except ValueError:
                    # 不是数字，视为非法表达式
                    return "invalid", None, stdout_data, stderr_data
            else:
                # 多行标准输出，视为非法表达式
                return "invalid", None, stdout_data, stderr_data
                
        except Exception as e:
            return None, f"调用calc失败: {str(e)}", "", ""
    
    def run_generator(self, count=1000, batch_size=100):
        """运行表达式生成器并获取结果，分批处理避免超时"""
        all_lines = []
        
        # 检查生成器是否存在
        if not os.path.exists('./gen-expr'):
            print("✗ 找不到表达式生成器 (./gen-expr)")
            return None
        
        # 分批处理，每批batch_size个表达式
        batches = (count + batch_size - 1) // batch_size
        
        for batch in range(batches):
            current_batch_size = min(batch_size, count - batch * batch_size)
            if current_batch_size <= 0:
                break
                
            print(f"[{batch + 1}/{batches}] 正在生成 {current_batch_size} 个表达式...")
            
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
                    print(f"  ✓ 成功获取 {len(batch_lines)} 个表达式")
                else:
                    print(f"  ✗ 生成器返回错误: {result.stderr}")
                    return None
                    
            except subprocess.TimeoutExpired:
                print(f"  ✗ 第 {batch + 1} 批生成超时")
                return None
            except FileNotFoundError:
                print("  ✗ 找不到表达式生成器")
                return None
            except Exception as e:
                print(f"  ✗ 运行表达式生成器失败: {e}")
                return None
            
            # 批次间短暂暂停
            time.sleep(0.1)
        
        return all_lines
    
    def validate_expressions(self, lines):
        """验证所有表达式的正确性"""
        print(f"开始验证 {len(lines)} 个表达式...")
        start_time = time.time()
        
        for i, line in enumerate(lines):
            self.total_expressions += 1
            
            # 每20个表达式打印一次进度，减少进度打印频率
            if (i + 1) % 20 == 0:
                elapsed = time.time() - start_time
                progress = (i + 1) / len(lines) * 100
                avg_time = elapsed / (i + 1) if (i + 1) > 0 else 0
                remaining = avg_time * (len(lines) - i - 1)
                print(f"[{i+1}/{len(lines)}] 进度: {progress:.1f}% - 已用时: {elapsed:.1f}秒 - 预计剩余: {remaining:.1f}秒")
            
            # 解析生成器的输出
            parts = line.strip().split(' ', 1)
            if len(parts) != 2:
                print(f"警告: 无法解析行: {line}")
                continue
                
            gen_expr_result, expr = parts
            expr = expr.strip()  # 去除表达式前后的空格
            
            # 使用calc计算表达式的实际结果
            calc_result, error, raw_output, stderr_output = self.evaluate_with_calc(expr)
            
            if error:
                # 计算出错
                self.incorrect_expressions.append({
                    'expression': expr,
                    'expected': calc_result if calc_result is not None else "N/A",
                    'actual': gen_expr_result,
                    'error': error,
                    'calc_stdout': raw_output,
                    'calc_stderr': stderr_output
                })
                continue
            
            # 处理未知结果
            if calc_result == "unknown":
                self.unknown_expressions.append({
                    'expression': expr,
                    'gen_expr_result': gen_expr_result,
                    'calc_stdout': raw_output,
                    'calc_stderr': stderr_output,
                    'reason': error  # 保存原因，例如"无任何输出"
                })
                continue
            
            # 比较结果 - 注意：预期结果是calc的输出，实际结果是gen-expr的结果
            if calc_result == "invalid":
                if gen_expr_result == "invalid":
                    self.correct_expressions += 1
                else:
                    self.incorrect_expressions.append({
                        'expression': expr,
                        'expected': "invalid",
                        'actual': gen_expr_result,
                        'calc_stdout': raw_output,
                        'calc_stderr': stderr_output
                    })
            else:
                # calc结果是数字
                try:
                    gen_expr_num = int(gen_expr_result)
                    if calc_result == gen_expr_num:
                        self.correct_expressions += 1
                    else:
                        self.incorrect_expressions.append({
                            'expression': expr,
                            'expected': calc_result,
                            'actual': gen_expr_num,
                            'calc_stdout': raw_output,
                            'calc_stderr': stderr_output
                        })
                except ValueError:
                    # gen-expr的结果不是数字，但calc结果是数字
                    self.incorrect_expressions.append({
                        'expression': expr,
                        'expected': calc_result,
                        'actual': gen_expr_result,
                        'error': "gen-expr结果不是数字但calc结果是数字",
                        'calc_stdout': raw_output,
                        'calc_stderr': stderr_output
                    })
    
    def print_results(self):
        """打印验证结果"""
        print("\n" + "=" * 80)
        print("表达式验证结果")
        print("=" * 80)
        print(f"总表达式数量: {self.total_expressions}")
        print(f"正确表达式数量: {self.correct_expressions}")
        print(f"错误表达式数量: {len(self.incorrect_expressions)}")
        print(f"未知结果数量: {len(self.unknown_expressions)}")
        accuracy = self.correct_expressions / self.total_expressions * 100 if self.total_expressions > 0 else 0
        print(f"正确率: {accuracy:.2f}%")
        
        if self.unknown_expressions:
            print("\n" + "=" * 80)
            print("未知结果表达式详情")
            print("=" * 80)
            
            for i, item in enumerate(self.unknown_expressions[:10]):  # 只显示前10个
                print(f"  {i+1}. 表达式: {item['expression']}")
                print(f"     gen-expr结果: {item['gen_expr_result']}")
                print(f"     原因: {item['reason']}")
                if 'calc_stdout' in item and item['calc_stdout']:
                    print(f"     Calc标准输出: {repr(item['calc_stdout'])}")
                if 'calc_stderr' in item and item['calc_stderr']:
                    print(f"     Calc错误输出: {repr(item['calc_stderr'])}")
                print()  # 空行分隔
            
            if len(self.unknown_expressions) > 10:
                print(f"  ... 还有 {len(self.unknown_expressions) - 10} 个未知结果未显示")
        
        if self.incorrect_expressions:
            print("\n" + "=" * 80)
            print("错误表达式详情")
            print("=" * 80)
            
            # 按错误类型分类
            error_types = defaultdict(list)
            for item in self.incorrect_expressions:
                if 'error' in item:
                    error_types[item['error']].append(item)
                elif item['expected'] == 'invalid' and item['actual'] != 'invalid':
                    error_types["应为非法但gen-expr认为有效"].append(item)
                elif item['expected'] != 'invalid' and item['actual'] == 'invalid':
                    error_types["应为有效但gen-expr认为非法"].append(item)
                elif isinstance(item['expected'], int) and isinstance(item['actual'], int):
                    error_types["数值不匹配"].append(item)
                else:
                    error_types["其他错误"].append(item)
            
            # 打印每种错误类型的统计
            print("\n错误类型统计:")
            for error_type, items in error_types.items():
                print(f"  {error_type}: {len(items)} 个")
            
            # 打印每种错误类型的详细信息
            for error_type, items in error_types.items():
                print(f"\n{error_type} 的详细错误:")
                for i, item in enumerate(items[:10]):  # 只显示前10个
                    print(f"  {i+1}. 表达式: {item['expression']}")
                    print(f"     预期结果 (calc): {item['expected']}")
                    print(f"     实际结果 (gen-expr): {item['actual']}")
                    if 'calc_stdout' in item and item['calc_stdout']:
                        print(f"     Calc标准输出: {repr(item['calc_stdout'])}")
                    if 'calc_stderr' in item and item['calc_stderr']:
                        print(f"     Calc错误输出: {repr(item['calc_stderr'])}")
                    if 'error' in item:
                        print(f"     错误: {item['error']}")
                    print()  # 空行分隔
                if len(items) > 10:
                    print(f"  ... 还有 {len(items) - 10} 个错误未显示")
        
        # 保存结果到文件
        if self.incorrect_expressions or self.unknown_expressions:
            filename = f"validation_results_{int(time.time())}.txt"
            with open(filename, "w") as f:
                f.write("表达式验证结果\n")
                f.write("=" * 50 + "\n")
                f.write(f"总表达式数量: {self.total_expressions}\n")
                f.write(f"正确表达式数量: {self.correct_expressions}\n")
                f.write(f"错误表达式数量: {len(self.incorrect_expressions)}\n")
                f.write(f"未知结果数量: {len(self.unknown_expressions)}\n")
                f.write(f"正确率: {accuracy:.2f}%\n\n")
                
                if self.unknown_expressions:
                    f.write("未知结果表达式:\n")
                    f.write("-" * 30 + "\n")
                    for i, item in enumerate(self.unknown_expressions):
                        f.write(f"{i+1}. 表达式: {item['expression']}\n")
                        f.write(f"   gen-expr结果: {item['gen_expr_result']}\n")
                        f.write(f"   原因: {item['reason']}\n")
                        if 'calc_stdout' in item and item['calc_stdout']:
                            f.write(f"   Calc标准输出: {repr(item['calc_stdout'])}\n")
                        if 'calc_stderr' in item and item['calc_stderr']:
                            f.write(f"   Calc错误输出: {repr(item['calc_stderr'])}\n")
                        f.write("\n")
                
                if self.incorrect_expressions:
                    f.write("错误表达式:\n")
                    f.write("-" * 30 + "\n")
                    for i, item in enumerate(self.incorrect_expressions):
                        f.write(f"{i+1}. 表达式: {item['expression']}\n")
                        f.write(f"   预期结果 (calc): {item['expected']}\n")
                        f.write(f"   实际结果 (gen-expr): {item['actual']}\n")
                        if 'calc_stdout' in item and item['calc_stdout']:
                            f.write(f"   Calc标准输出: {repr(item['calc_stdout'])}\n")
                        if 'calc_stderr' in item and item['calc_stderr']:
                            f.write(f"   Calc错误输出: {repr(item['calc_stderr'])}\n")
                        if 'error' in item:
                            f.write(f"   错误: {item['error']}\n")
                        f.write("\n")
            print(f"\n详细结果已保存到文件: {filename}")

def main():
    print("=" * 60)
    print("表达式验证程序")
    print("=" * 60)
    
    # 检查必要文件
    if not os.path.exists('./calc'):
        print("✗ 找不到计算器程序 (./calc)")
        return
    
    if not os.path.exists('./gen-expr'):
        print("✗ 找不到表达式生成器 (./gen-expr)")
        return
    
    validator = ExpressionValidator()
    
    print("正在运行表达式生成器...")
    lines = validator.run_generator(1000, batch_size=100)
    
    if not lines:
        print("无法获取表达式，程序退出")
        return
    
    print(f"✓ 成功获取 {len(lines)} 个表达式")
    print("开始验证表达式正确性...")
    
    start_time = time.time()
    validator.validate_expressions(lines)
    total_time = time.time() - start_time
    
    print(f"\n✓ 验证完成！总共用时 {total_time:.2f} 秒")
    validator.print_results()

if __name__ == "__main__":
    main()
