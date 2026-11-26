#!/usr/bin/env python3
"""Unified Lab1 test runner: auto-builds gen-expr, fetches calc, checks diversity & correctness."""

import argparse
import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import time
import urllib.request
from collections import Counter, defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Dict, List, Optional, Tuple

LAB_DIR = Path(__file__).resolve().parent
CALC_URL = "https://box.nju.edu.cn/f/ad350ac326214429a57d/?dl=1"

VALID_REGS = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6",
}

TOKEN_SPEC = [
    ("HEX", r"0[xX][0-9a-fA-F]+"),
    ("DEC", r"\d+"),
    ("REG", r"\$\$?[a-zA-Z0-9_]+"),
    ("EQ", r"==|!="),
    ("AND", r"&&"),
    ("OP", r"[+\-*/()]"),
    ("WS", r"\s+"),
]
TOKEN_RE = re.compile("|".join(f"(?P<{name}>{pat})" for name, pat in TOKEN_SPEC))

LENGTH_BUCKETS = [
    ("1-10", 1, 10),
    ("10-100", 10, 100),
    ("100-1000", 100, 1000),
    ("1000-10000", 1000, 10000),
    ("10000-65536", 10000, 65537),
]

NUMBER_BUCKETS = [
    ("0-10", 0, 10),
    ("10-100", 10, 100),
    ("100-1000", 100, 1000),
    ("1000-10000", 1000, 10000),
    ("10000-100000", 10000, 100000),
    ("100000-1000000", 100000, 1000000),
    ("1000000-10000000", 1000000, 10000000),
    ("10000000-100000000", 10000000, 100000000),
    ("100000000-1000000000", 100000000, 1000000000),
]

DEREF_PATTERN = re.compile(r"(?:^|[+\-*/(&!=])\s*\*\s*(?:\(|0[xX][0-9a-fA-F]+|\d+|\$)")
REG_PATTERN = re.compile(r"\$\$?[a-zA-Z0-9_]+")


def _sources_newer_than(target: Path) -> bool:
    """Return True if any C/H source or Makefile is newer than target."""
    target_mtime = target.stat().st_mtime
    for pattern in ("src/*.c", "include/*.h", "Makefile"):
        for path in LAB_DIR.glob(pattern):
            if path.stat().st_mtime > target_mtime:
                return True
    return False


def ensure_gen_expr() -> bool:
    """Build ./gen-expr via `make all` (skipping git commit hook) if missing or stale."""
    gen_expr = LAB_DIR / "gen-expr"
    if gen_expr.is_file() and os.access(gen_expr, os.X_OK) and not _sources_newer_than(gen_expr):
        return True

    print("正在执行 `make all` 编译 ./gen-expr ...")
    try:
        subprocess.run(["make", "all"], cwd=LAB_DIR, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError) as exc:
        print(f"✗ 编译 ./gen-expr 失败: {exc}")
        return False

    return gen_expr.is_file() and os.access(gen_expr, os.X_OK)


def ensure_calc(force: bool = False) -> bool:
    """Download reference ./calc binary from course NJU Box URL if missing or forced."""
    calc = LAB_DIR / "calc"
    if not force and calc.is_file() and os.access(calc, os.X_OK):
        return True

    print("正在从课程网盘拉取参考程序 ./calc ...")
    fd, tmp_name = tempfile.mkstemp(prefix=".calc.tmp.", dir=LAB_DIR)
    tmp_path = Path(tmp_name)
    try:
        with os.fdopen(fd, "wb") as out_f:
            with urllib.request.urlopen(CALC_URL, timeout=30) as resp:
                shutil.copyfileobj(resp, out_f)
        if tmp_path.stat().st_size == 0:
            raise RuntimeError("downloaded file is empty")
        os.chmod(tmp_path, stat.S_IRWXU | stat.S_IRGRP | stat.S_IXGRP | stat.S_IROTH | stat.S_IXOTH)
        tmp_path.replace(calc)
        print(f"✓ 已保存 ./calc ({calc.stat().st_size} bytes)")
        return True
    except Exception as exc:
        print(f"✗ 拉取 ./calc 失败: {exc}")
        if tmp_path.exists():
            tmp_path.unlink()
        return False


def generate_expressions(count: int = 1000, batch_size: int = 100, timeout: int = 120) -> List[str]:
    """Run ./gen-expr in batches and return all generated output lines."""
    lines: List[str] = []
    batches = (count + batch_size - 1) // batch_size

    for batch in range(batches):
        current = min(batch_size, count - batch * batch_size)
        if current <= 0:
            break
        t0 = time.monotonic()
        res = subprocess.run(
            [str(LAB_DIR / "gen-expr"), str(current)],
            cwd=LAB_DIR,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
        if res.returncode != 0:
            raise RuntimeError(f"./gen-expr exited with {res.returncode}: {res.stderr.strip()}")
        lines.extend(res.stdout.splitlines())
        print(f"  [{batch + 1}/{batches}] 已生成 {len(lines)}/{count} 条 ({time.monotonic() - t0:.1f}s)")

    return lines


def parse_generated_line(line: str) -> Optional[Tuple[str, str]]:
    """Split `<expected> <expr>` into `(expected, expr)`, or None if malformed."""
    parts = line.strip().split(maxsplit=1)
    if len(parts) != 2:
        return None
    return parts[0], parts[1].strip()


def _tokenize(expr: str) -> Optional[List[Tuple[str, str]]]:
    """Tokenize expr into (kind, value) pairs; return None on unknown character."""
    tokens: List[Tuple[str, str]] = []
    pos = 0
    while pos < len(expr):
        m = TOKEN_RE.match(expr, pos)
        if not m:
            return None
        pos = m.end()
        if m.lastgroup != "WS":
            tokens.append((m.lastgroup, m.group()))
    return tokens


class _ExprChecker:
    """Recursive-descent uint32 evaluator tracking overflow/underflow and non-zero div."""

    def __init__(self, tokens: List[Tuple[str, str]]) -> None:
        self.tokens = tokens
        self.pos = 0
        self.saw_overflow = False
        self.saw_nonzero_div = False
        self._skip_depth = 0

    def _peek(self) -> Optional[Tuple[str, str]]:
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def _consume(self) -> Tuple[str, str]:
        tok = self.tokens[self.pos]
        self.pos += 1
        return tok

    def eval_all(self) -> int:
        val = self._parse_and()
        if self.pos != len(self.tokens):
            raise ValueError("trailing tokens")
        return val

    def _parse_and(self) -> int:
        lhs = self._parse_eq()
        while (tok := self._peek()) and tok[0] == "AND":
            self._consume()
            if lhs == 0:
                self._skip_depth += 1
                try:
                    self._parse_eq()
                finally:
                    self._skip_depth -= 1
                lhs = 0
            else:
                lhs = 1 if self._parse_eq() != 0 else 0
        return lhs

    def _parse_eq(self) -> int:
        lhs = self._parse_add()
        while (tok := self._peek()) and tok[1] in ("==", "!="):
            op = self._consume()[1]
            rhs = self._parse_add()
            lhs = int(lhs == rhs) if op == "==" else int(lhs != rhs)
        return lhs

    def _parse_add(self) -> int:
        lhs = self._parse_mul()
        while (tok := self._peek()) and tok[1] in ("+", "-"):
            op = self._consume()[1]
            rhs = self._parse_mul()
            raw = lhs + rhs if op == "+" else lhs - rhs
            if self._skip_depth == 0 and (raw < 0 or raw > 0xFFFFFFFF):
                self.saw_overflow = True
            lhs = raw & 0xFFFFFFFF
        return lhs

    def _parse_mul(self) -> int:
        lhs = self._parse_unary()
        while (tok := self._peek()) and tok[0] == "OP" and tok[1] in ("*", "/"):
            op = self._consume()[1]
            rhs = self._parse_unary()
            if op == "*":
                raw = lhs * rhs
                if self._skip_depth == 0 and raw > 0xFFFFFFFF:
                    self.saw_overflow = True
                lhs = raw & 0xFFFFFFFF
            else:
                if rhs == 0:
                    if self._skip_depth > 0:
                        lhs = 0
                        continue
                    raise ZeroDivisionError
                if self._skip_depth == 0:
                    self.saw_nonzero_div = True
                lhs = lhs // rhs
        return lhs

    def _parse_unary(self) -> int:
        tok = self._peek()
        if tok and tok[1] == "-":
            self._consume()
            val = self._parse_unary()
            if self._skip_depth == 0 and val != 0:
                self.saw_overflow = True
            return (-val) & 0xFFFFFFFF
        if tok and tok[1] == "*":
            self._consume()
            addr = self._parse_unary()
            if self._skip_depth == 0 and not (0x80000000 <= addr <= 0x87FFFFFF):
                raise ValueError("out of bound")
            return 0
        return self._parse_primary()

    def _parse_primary(self) -> int:
        tok = self._peek()
        if not tok:
            raise ValueError("unexpected eof")
        if tok[1] == "(":
            self._consume()
            val = self._parse_and()
            closing = self._peek()
            if not closing or closing[1] != ")":
                raise ValueError("unclosed paren")
            self._consume()
            return val
        if tok[0] in ("HEX", "DEC"):
            self._consume()
            val = int(tok[1], 16 if tok[0] == "HEX" else 10)
            if val > 0xFFFFFFFF:
                raise ValueError("literal overflow")
            return val
        if tok[0] == "REG":
            self._consume()
            if tok[1][1:] not in VALID_REGS:
                raise ValueError("invalid reg")
            return 0
        raise ValueError(f"unexpected token {tok}")


class ExpressionAnalyzer:
    """Collect Lab1 §2.3 diversity statistics over generated expressions."""

    def __init__(self) -> None:
        self.total = 0
        self.valid = 0
        self.invalid = 0
        self.seen_exprs: Counter = Counter()
        self.complex_count = 0
        self.long_space_count = 0
        self.nonzero_div_count = 0
        self.overflow_count = 0
        self.mem_or_reg_count = 0
        self.length_counts: Dict[str, int] = {name: 0 for name, _, _ in LENGTH_BUCKETS}
        self.number_counts: Dict[str, int] = {name: 0 for name, _, _ in NUMBER_BUCKETS}

    def analyze_line(self, line: str) -> None:
        parsed = parse_generated_line(line)
        if not parsed:
            return
        result, expr = parsed
        self.total += 1
        self.seen_exprs[expr] += 1
        if result == "invalid":
            self.invalid += 1
        else:
            self.valid += 1

        if all(op in expr for op in ("+", "-", "*", "/", "(", ")")):
            self.complex_count += 1

        n = len(expr)
        for name, lo, hi in LENGTH_BUCKETS:
            if lo <= n < hi:
                self.length_counts[name] += 1
                break

        if "   " in expr:
            self.long_space_count += 1

        if REG_PATTERN.search(expr) or DEREF_PATTERN.search(expr):
            self.mem_or_reg_count += 1

        tokens = _tokenize(expr)
        if not tokens:
            return

        for kind, raw in tokens:
            if kind in ("HEX", "DEC"):
                val = int(raw, 16 if kind == "HEX" else 10)
                for name, lo, hi in NUMBER_BUCKETS:
                    if lo <= val < hi:
                        self.number_counts[name] += 1
                        break

        if result != "invalid":
            try:
                checker = _ExprChecker(tokens)
                checker.eval_all()
                if checker.saw_nonzero_div:
                    self.nonzero_div_count += 1
                if checker.saw_overflow:
                    self.overflow_count += 1
            except (ValueError, ZeroDivisionError, RecursionError):
                pass

    def print_report(self) -> None:
        t = max(1, self.total)
        unique_count = len(self.seen_exprs)
        print("\n" + "=" * 64)
        print("1) Lab1 表达式多样性统计")
        print("=" * 64)
        print(f"总数量: {self.total} | 合法: {self.valid} | 非法: {self.invalid} | 唯一: {unique_count} (重复 {self.total - unique_count})")
        print(f"- 同时含 + - * / () 复杂表达式: {self.complex_count}/{self.total} ({self.complex_count / t:.1%})")
        print("- 长度分布:")
        for name, _, _ in LENGTH_BUCKETS:
            c = self.length_counts[name]
            print(f"    {name:12s}: {c:4d}/{self.total} ({c / t:.1%})")

        total_nums = max(1, sum(self.number_counts.values()))
        print("- 数值范围分布:")
        for name, _, _ in NUMBER_BUCKETS:
            c = self.number_counts[name]
            print(f"    {name:20s}: {c:5d} ({c / total_nums:.1%})")

        print(f"- 含连续长空格 (>=3空格) : {self.long_space_count}/{self.total} ({self.long_space_count / t:.1%})")
        print(f"- 含非零除法的有效表达式  : {self.nonzero_div_count}/{self.total} ({self.nonzero_div_count / t:.1%})  [要求 >=10%]")
        print(f"- 中间步骤触发溢出的表达式: {self.overflow_count}/{self.total} ({self.overflow_count / t:.1%})  [要求 >=10%]")
        print(f"- 含寄存器或指针解引用    : {self.mem_or_reg_count}/{self.total} ({self.mem_or_reg_count / t:.1%})  [要求 >=10%]")


def evaluate_with_calc(expr: str, timeout: float = 2.0) -> Tuple[object, Optional[str], str, str]:
    """Evaluate one expression with ./calc."""
    try:
        proc = subprocess.run(
            [str(LAB_DIR / "calc")],
            cwd=LAB_DIR,
            input=expr + "\n",
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return None, "计算超时", "", ""
    except OSError as exc:
        return None, f"调用 ./calc 失败: {exc}", "", ""

    stdout, stderr = proc.stdout, proc.stderr
    stdout_lines = [ln.strip() for ln in stdout.splitlines() if ln.strip()]
    has_stderr = bool(stderr.strip())

    if not stdout_lines and not has_stderr:
        return "unknown", None, stdout, stderr
    if has_stderr or proc.returncode != 0 or len(stdout_lines) != 1:
        return "invalid", None, stdout, stderr

    try:
        return int(stdout_lines[0]), None, stdout, stderr
    except ValueError:
        return "invalid", None, stdout, stderr


def _validate_one(line: str) -> Dict[str, object]:
    parsed = parse_generated_line(line)
    if not parsed:
        return {"kind": "malformed", "line": line}

    actual_str, expr = parsed
    expected, err, stdout, stderr = evaluate_with_calc(expr)

    if err is not None:
        return {"kind": "incorrect", "reason": err, "expr": expr, "expected": "N/A", "actual": actual_str, "stdout": stdout, "stderr": stderr}
    if expected == "unknown":
        return {"kind": "unknown", "reason": "calc 无任何输出", "expr": expr, "actual": actual_str}
    if expected == "invalid":
        if actual_str == "invalid":
            return {"kind": "correct"}
        return {"kind": "incorrect", "reason": "应为 invalid 但 gen-expr 给出数字", "expr": expr, "expected": "invalid", "actual": actual_str, "stdout": stdout, "stderr": stderr}

    try:
        actual_num = int(actual_str)
    except ValueError:
        return {"kind": "incorrect", "reason": "应为数字但 gen-expr 给出 invalid", "expr": expr, "expected": expected, "actual": actual_str, "stdout": stdout, "stderr": stderr}

    if actual_num == expected:
        return {"kind": "correct"}
    return {"kind": "incorrect", "reason": "数值不匹配", "expr": expr, "expected": expected, "actual": actual_num, "stdout": stdout, "stderr": stderr}


def run_correctness_check(lines: List[str], workers: int) -> bool:
    """Validate lines against ./calc in parallel; return True if zero incorrect."""
    print(f"\n开始并行调用 ./calc 对拍 ({workers} 线程)...")
    t0 = time.monotonic()
    with ThreadPoolExecutor(max_workers=workers) as pool:
        results = list(pool.map(_validate_one, lines))
    elapsed = time.monotonic() - t0

    correct = sum(1 for r in results if r["kind"] == "correct")
    incorrect = [r for r in results if r["kind"] == "incorrect"]
    unknown = [r for r in results if r["kind"] == "unknown"]
    malformed = [r for r in results if r["kind"] == "malformed"]
    total = len(results)
    accuracy = (correct / total * 100) if total else 0.0

    print("\n" + "=" * 64)
    print(f"2) 正确性对拍结果 (calc 用时 {elapsed:.2f}s)")
    print("=" * 64)
    print(f"总数: {total} | 正确: {correct} | 错误: {len(incorrect)} | 未知: {len(unknown)} | 格式异常: {len(malformed)} | 正确率: {accuracy:.2f}%")

    if incorrect:
        grouped: Dict[str, List[Dict[str, object]]] = defaultdict(list)
        for item in incorrect:
            grouped[str(item["reason"])].append(item)
        print("- 错误分类:")
        for reason, items in grouped.items():
            print(f"    {reason}: {len(items)} 条")
            for sample in items[:3]:
                print(f"      预期={sample['expected']} 实际={sample['actual']} | expr={str(sample['expr'])[:80]}")

    if incorrect or unknown:
        report_path = LAB_DIR / f"validation_results_{int(time.time())}.txt"
        with open(report_path, "w", encoding="utf-8") as f:
            f.write(f"总计: {total}, 正确: {correct}, 错误: {len(incorrect)}, 未知: {len(unknown)}, 正确率: {accuracy:.2f}%\n\n")
            for label, items in (("未知结果", unknown), ("错误详情", incorrect)):
                if not items:
                    continue
                f.write(f"=== {label} ({len(items)}) ===\n")
                for idx, item in enumerate(items, 1):
                    f.write(f"{idx}. [{item['reason']}] expr={item['expr']}\n")
                    f.write(f"   expected={item.get('expected', 'unknown')} actual={item['actual']}\n")
                    if item.get("stderr"):
                        f.write(f"   calc stderr={item['stderr']!r}\n")
                f.write("\n")
        print(f"- 详细记录已保存至: {report_path.name}")

    return len(incorrect) == 0


def main() -> int:
    sys.setrecursionlimit(max(sys.getrecursionlimit(), 4000))
    parser = argparse.ArgumentParser(description="Lab1 unified expression diversity and correctness test suite.")
    parser.add_argument("-n", "--count", type=int, default=1000, help="Number of expressions to generate")
    parser.add_argument("-b", "--batch-size", type=int, default=100, help="Batch size per ./gen-expr run")
    parser.add_argument("-j", "--workers", type=int, default=8, help="Concurrent ./calc worker threads")
    parser.add_argument("--only", choices=("all", "diversity", "correctness", "fetch"), default="all", help="Run a subset of steps")
    parser.add_argument("--refresh-calc", action="store_true", help="Force re-download of ./calc")
    args = parser.parse_args()

    if args.only == "fetch":
        return 0 if ensure_calc(force=True) else 1

    if args.only in ("all", "correctness") and not ensure_calc(force=args.refresh_calc):
        return 1
    if not ensure_gen_expr():
        return 1

    print(f"正在生成 {args.count} 条表达式...")
    try:
        lines = generate_expressions(count=args.count, batch_size=args.batch_size)
    except Exception as exc:
        print(f"✗ 生成表达式失败: {exc}")
        return 1

    if args.only in ("all", "diversity"):
        analyzer = ExpressionAnalyzer()
        for line in lines:
            analyzer.analyze_line(line)
        analyzer.print_report()

    ok = True
    if args.only in ("all", "correctness"):
        ok = run_correctness_check(lines, workers=args.workers)

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
