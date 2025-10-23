#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "../include/expr.h"

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

__attribute__((unused))
static inline u_int32_t random_int_range(u_int32_t begin, u_int32_t end) {
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
static size_t gen_decimal(char *buf, size_t buf_size, size_t offset) {
    static const u_int32_t range[] = {
        0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, UINT_MAX
    };

    int max = random_int(ARRLEN(range));
    size_t empty_size = buf_size - offset - 1;
    max = max > empty_size ? empty_size : max;
    int min = max - 1;

    u_int32_t num = random_int_range(range[min], range[max]);
    // printf("decimal: %u\n", num);
    sprintf(buf + offset, "%u", num);
    return max;
}

__attribute__((unused))
static size_t gen_hex(char *buf, size_t buf_size, size_t offset) {
    static const u_int32_t range[] = {
        0x0, 0x10, 0x100, 0x1000, 0x10000, 0x100000, 0x1000000, 0x10000000, 0xffffffff
    };
    
    int max = random_int(ARRLEN(range));
    size_t empty_size = buf_size - offset - 1;
    max = max + 4 > empty_size ? empty_size - 4 : max;
    int min = max - 1;

    u_int32_t num = random_int_range(range[min], range[max]);
    sprintf(buf + offset, "(0x%x)", num);
    return strlen(buf + offset);
}

__attribute__((unused))
static size_t gen_op(char *buf, size_t buf_size, size_t offset) {
    static const char *ops[] = {"+", "-", "*", "/", "==", "!=", "&&"};
    int index; 
    size_t empty_space = buf_size - offset - 1;
    if (empty_space >= 2) {
        index = random_int(ARRLEN(ops));
    } else if (empty_space == 1) {
        index = random_int(4);
    } else {
        return 0;
    }
    strcpy(buf + offset, ops[index]);
    return strlen(ops[index]);
}

__attribute__((unused))
static size_t gen_reg(char *buf, size_t buf_size, size_t offset) {
    static const char *regs[] = {
        "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };
    int index = random_int(ARRLEN(regs));
    int len = strlen(regs[index]) + 1;
    if (buf_size - offset - 1 < len) {
        return 0;
    }
    sprintf(buf + offset, "$%s", regs[index]);
    return len;
}

__attribute__((unused))
static size_t gen_addr(char *buf, size_t buf_size, size_t offset) {
    static const u_int32_t min = 0x80000000;
    static const u_int32_t max = 0x87ffffff;
    static const int len = 10;
    if (buf_size - offset - 1 < len) {
        return 0;
    }
    sprintf(buf + offset, "0x%x", random_int_range(min, max + 1));
    return len;
}

__attribute__((unused))
static size_t gen_left_paran(char *buf, size_t buf_size, size_t offset) {
    if (buf_size - offset - 1 < 1) {
        return 0;
    }
    buf[offset] = '(';
    buf[offset + 1] = '\0';
    return 1;
}

__attribute__((unused))
static int gen_right_paran(char *buf, size_t buf_size, size_t offset) {
    if (buf_size - offset - 1 < 1) {
        return 0;
    }
    buf[offset] = ')';
    buf[offset + 1] = '\0';
    return 1;
}

__attribute__((unused))
static size_t gen_space(char *buf, size_t buf_size, size_t offset) {
    int size = buf_size - offset;
    size = size > 10 ? 10 : size;
    int len = random_int(size);
    for (int i = 0; i < len; i++) {
        buf[offset++] = ' ';
    }
    buf[offset] = '\0';
    return len;
}

__attribute__((unused))
static size_t gen(char *buf, size_t buf_size, size_t offset) {
    if (offset >= buf_size) {
        gen(buf, buf_size, 0);
    }

    size_t len = 0;
    switch (random_int(4)) {
        case 0:
            // printf("choose: 0\n");
            return gen_decimal(buf, buf_size, offset);
            break;
        case 1:
            // printf("choose: 1\n");
            return gen_hex(buf, buf_size, offset);
            break;
        case 2:
            // printf("choose: 2\n");
            len += gen_left_paran(buf, buf_size, offset + len);
            len += gen(buf, buf_size, offset + len);
            len += gen_right_paran(buf, buf_size, offset + len);
            return len;
            break;
        case 3:
            // printf("choose: 3\n");
            len += gen(buf, buf_size, offset + len);
            len += gen_op(buf, buf_size, offset + len);
            len += gen(buf, buf_size, offset + len);
            return len;
            break;
        default:
            printf("error\n");
            return 0;
    }
}

void gen_rand_expr(char *buf, size_t buf_size) {
    // 生成随机表达式的逻辑（原始代码未实现，可后续补充）
    static const size_t lens[] = {1, 10, 100, 1000, 10000};
    int max;
    do {
        max = random_int_range(2, ARRLEN(lens));
    } while (lens[max] > buf_size);

    size_t size = random_int_range(lens[max - 1], lens[max] + 1);
    gen(buf, size, 0);
}
