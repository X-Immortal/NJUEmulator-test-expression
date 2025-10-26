#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "../include/expr.h"

#define DEBUG 0

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

static void gen();

char *buf;
size_t buf_size;
size_t offset;
char *calc_buf;
size_t calc_buf_size;
size_t calc_offset;
bool success = true;

__attribute__((unused))
static inline u_int32_t random_int_range(u_int32_t begin, u_int32_t end) {
    if (begin == end) return begin;
    if (end < begin) {
        printf("Invalid range: from %u to %u\n", begin, end);
        return 0;
    }
    return ((u_int32_t) rand() % (end - begin)) + begin;
}

__attribute__((unused))
static inline u_int32_t random_int(u_int32_t n) {
    return random_int_range(0, n);
}

__attribute__((unused))
static void gen_space() {
    int size = buf_size - offset - 2;
    if (size <= 0) return;

    size = size > 5 ? 5 : size;
    int len = 0;
    if (random_int(2) == 1) {
        len = random_int_range(1, size);
    }
#if DEBUG == 1
    // printf("size: %lu\n", buf_size);
    printf("gen space: %d\n", len);
#endif
    for (int i = 0; i < len; i++) {
        buf[offset++] = ' ';
    }
    buf[offset] = '\0';
}

__attribute__((unused))
static void gen_decimal() {
    static const u_int32_t range[] = {
        0, 10, 100, 1000, 
        10000, 100000, 1000000, 10000000, 
        100000000, 1000000000, UINT_MAX
    };

    size_t empty_size = buf_size - offset - 1;
    if (empty_size == 0) {
        success = false;
        return;
    }

    int max = random_int_range(1, ARRLEN(range));
    max = max > empty_size ? empty_size : max;
    int min = max - 1;

    u_int32_t num = random_int_range(range[min], range[max]);
    sprintf(buf + offset, "%u", num);
    offset += max;
    sprintf(calc_buf + calc_offset, "(uint32_t)%u", num);
    calc_offset += max + 10;
}

__attribute__((unused))
static void gen_hex() {
    static const u_int32_t range[] = {
        0x0, 0x10, 0x100, 
        0x1000, 0x10000, 0x100000, 
        0x1000000, 0x10000000, 0xffffffff
    };
    
    int empty_size = buf_size - offset - 1;
    if (empty_size < 5) {
        success = false;
        return;
    }

    int max = random_int_range(1, ARRLEN(range));
    max = max + 4 > empty_size ? empty_size - 4 : max;
    int min = max - 1;

    u_int32_t num = random_int_range(range[min], range[max]);
    sprintf(buf + offset, "(0x%x)", num);
    sprintf(calc_buf + calc_offset, "(uint32_t)(0x%x)", num);
    offset += max + 4;
    calc_offset += max + 14;
}

__attribute__((unused))
static void gen_op() {
    static const char *ops[] = {"+", "-", "*", "==", "!=", "&&"};

    size_t empty_size = buf_size - offset - 1;
    if (empty_size == 0) {
        success = false;
        return;
    }

    int index; 
    if (empty_size >= 2) {
        index = random_int(ARRLEN(ops));
    } else if (empty_size == 1) {
        index = random_int(3);
    }

    size_t op_len = strlen(ops[index]);
    strcpy(buf + offset, ops[index]);
    offset += op_len;
    strcpy(calc_buf + calc_offset, ops[index]);
    calc_offset += op_len;
}

__attribute__((unused))
static void gen_left_paren() {
    if (buf_size - offset - 1 < 1) {
        success = false;
        return;
    }

    sprintf(buf + offset++, "(");
    sprintf(calc_buf + calc_offset++, "(");
}

__attribute__((unused))
static void gen_right_paren() {
    if (buf_size - offset - 1 < 1) {
        success = false;
        return;
    }

    sprintf(buf + offset++, ")");
    sprintf(calc_buf + calc_offset++, ")");
}
__attribute__((unused))

static void gen_reg() {
    static const char *regs[] = {
        "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    int index = random_int(ARRLEN(regs));
    int len = strlen(regs[index]) + 1;

    if (len > buf_size - offset - 1) {
        success = false;
        return;
    }

    sprintf(buf + offset, "$%s", regs[index]);
    offset += len;
    sprintf(calc_buf + calc_offset, "(uint32_t)0");
    calc_offset += 11;
}

__attribute__((unused))
static void gen_addr() {
    static const u_int32_t min = 0x80000000;
    static const u_int32_t max = 0x87ffffff; 

    if (buf_size - offset - 1 < 13) {
        success = false;
        return;
    }

    if (random_int(2) == 0) {
        u_int32_t addr = random_int_range(min, max + 1);
        sprintf(buf + offset, "*0x%x", addr);
        offset += 11;
        sprintf(calc_buf + calc_offset, "(uint32_t)0");
        calc_offset += 11;
    } else {
        sprintf(buf + offset++, "*");
        sprintf(calc_buf + calc_offset, "vaddr_read");
        calc_offset += 10;

        gen_space();
        gen_left_paren();
        if (!success) return;

        gen_space();
        gen();
        if (!success) return;

        gen_space();
        gen_right_paren();
    }
}

__attribute__((unused))
static void gen_div() {
    if (buf_size - offset - 1 < 5) {
        success = false;
        return;
    }

    sprintf(calc_buf + calc_offset, "divide");
    calc_offset += 6;

    gen_left_paren();
    if (!success) return;

    gen_space();
    gen_left_paren();
    if (!success) return;

    gen_space();
    gen();
    if (!success) return;

    gen_space();
    gen_right_paren();
    if (!success) return;

    gen_space();
    if (buf_size - offset - 1 < 1) {
        success = false;
        return;
    }
    sprintf(buf + offset++, "/");
    sprintf(calc_buf + calc_offset++, ",");

    gen_space();
    if (random_int(2) == 0) {
        gen_decimal();
    } else {
        gen_left_paren();
        if (!success) return;

        gen_space();
        gen();
        if (!success) return;

        gen_space();
        gen_right_paren();
    }
    if (!success) return;

    gen_space();
    gen_right_paren();
}

__attribute__((unused))
static void gen_minus() {
    if (buf_size - offset - 1 < 2) {
        success = false;
        return;
    }

    sprintf(buf + offset++, "-");
    sprintf(calc_buf + calc_offset, "minus(");
    calc_offset += 6;

    if (random_int(2) == 0) {
        gen_decimal();
        if (!success) return;
    } else {
        gen_space();
        gen_left_paren();
        if (!success) return;

        gen_space();
        gen();
        if (!success) return;

        gen_space();
        gen_right_paren();
    }

    sprintf(calc_buf + calc_offset++, ")");
}

__attribute__((unused))
static void gen_max() {
    if (buf_size - offset - 1 < 14) {
        success = false;
        return;
    }

    sprintf(buf + offset, "(%u+1)", UINT_MAX);
    sprintf(calc_buf + calc_offset, "((uint32_t)%u+(uint32_t)1)", UINT_MAX);
    offset += 14;
    calc_offset += 34;
}

__attribute__((unused))
static void gen_min() {
    if (buf_size - offset - 1 < 5) {
        success = false;
        return;
    }

    sprintf(buf + offset, "(0-1)");
    sprintf(calc_buf + calc_offset, "((uint32_t)0-(uint32_t)1)");
    offset += 5;
    calc_offset += 25;
}

__attribute__((unused))
static void gen_minus_minus() {
    if (buf_size - offset - 1 < 3) {
        success = false;
        return;
    }

    sprintf(buf + offset, "--1");
    offset += 3;
    sprintf(calc_buf + calc_offset++, "1");
}

#define CHECK() do{if (!success) {depth = 0;return;}}while(0)

__attribute__((unused))
static void gen() {
    static int depth = 0;

    depth++;
    gen_space();
    u_int32_t choose = random_int(20);
    
#if DEBUG == 1
    printf("choose: %u\n", choose);
#endif

    switch (choose) {
        case 0:  case 11: case 12:
            gen_decimal(); 
            break;
        case 1:  case 13: case 14:
            gen_hex();
            break;
        case 2:
            gen_left_paren();
            CHECK();

            gen_space();
            gen();
            CHECK();

            gen_space();
            gen_right_paren();
            break;
        case 3:
            gen_minus_minus();
            break;
        case 4:
            gen_minus();
            break;
        case 5:  case 15: case 16:
        case 17: case 18: case 19:
            gen();
            CHECK();

            gen_space();
            gen_op();
            CHECK();

            gen_space();
            gen();
            break;
        case 6:
            gen_reg();
            break;
        case 7:
            gen_addr();
            break;
        case 8:
            gen_div();
            break;
        case 9:
            gen_min();
            break;
        case 10:
            gen_max();
            break;
        default:
            break;
    }
#if DEBUG == 1
    puts(buf);
    printf("offset: %lu\n", offset);
    printf("size: %lu\n", buf_size);
#endif
    CHECK();
    gen_space();
    depth -= depth == 0 ? 0 : 1;

    if (depth == 0 && buf_size - offset - 1 > 20) {
        CHECK();

        gen_space();
        gen_op();
        CHECK();

        gen_space();
        gen();
        CHECK();

        gen_space();
    }
}

__attribute__((unused))
static void break_paren() {
    if (buf_size - offset - 1 > 0) {
        sprintf(buf + offset++, ")");
    } else {
        char *p = strchr(buf, ')');
        if (p == NULL) {
            return;
        }
        *p = ' ';
    }
    sprintf(calc_buf, "invalid()");
    calc_offset = 9;
}

__attribute__((unused))
static void make_invalid_op() {
    if (buf_size - offset - 1 > 0) {
        sprintf(buf + offset++, "+");
    } else {
        char *p = strchr(buf, ' ');
        if (p == NULL) {
            return;
        }
        *p = '/';
    }
    sprintf(calc_buf, "invalid()");
    calc_offset = 9;
}

__attribute__((unused))
static void make_invalid_char() {
    char *p = NULL;
    for (int i = 0; i < 10 && !p; i++) {
        p = strchr(buf, '0' + i);
    }
    if (p == NULL) {
        return;
    }
    *p = '_';
    sprintf(calc_buf, "invalid()");
    calc_offset = 9;
}

__attribute__((unused))
static void make_invalid() {
    switch (random_int(3)) {
        case 0:
            break_paren();
            break;
        case 1:
            make_invalid_op();
            break;
        case 2:
            make_invalid_char();
            break;
        default:
            break;
    }
}

static void init(char *buf_, size_t buf_size_, char *calc_buf_, size_t calc_buf_size_) {
    buf = buf_;
    buf_size = buf_size_;
    offset = 0;
    calc_buf = calc_buf_;
    calc_buf_size = calc_buf_size_;
    calc_offset = 0;
    success = true;
}

void gen_rand_expr(char *buf_, size_t buf_size_, char *calc_buf_, size_t calc_buf_size_) {
    // 生成随机表达式的逻辑（原始代码未实现，可后续补充）
    static const size_t lens[] = {2, 11, 101, 1001, 10001};

    init(buf_, buf_size_, calc_buf_, calc_buf_size_);

    int max;
    do {
        max = random_int_range(1, ARRLEN(lens));
    } while (lens[max] > buf_size);

    size_t size = random_int_range(lens[max - 1], lens[max] + 1);

    do {
#if DEBUG == 1
        printf("start\n");
#endif
        success = true;
        offset = 0;
        calc_offset = 0;
        gen();
    } while (!success);

    if (random_int(8) == 0) {
        make_invalid(buf, buf_size, &offset, calc_buf, calc_buf_size, &calc_offset);
    }
}
