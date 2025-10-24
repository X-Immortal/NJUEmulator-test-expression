#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "../include/expr.h"

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

static void gen(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset, bool *success);

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
static void gen_decimal(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset) {
    static const u_int32_t range[] = {
        0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, UINT_MAX
    };

    if (buf_size - 1 == *offset) {
        printf("error\n");
        exit(1);
    }

    int max = random_int_range(1, ARRLEN(range));
    size_t empty_size = buf_size - *offset - 1;
    max = max > empty_size ? empty_size : max;
    int min = max - 1;

    u_int32_t num = random_int_range(range[min], range[max]);
    // printf("decimal: %u\n", num);
    sprintf(buf + *offset, "%u", num);
    *offset += max;
    if (calc_buf_size - *calc_offset - 1 < max + 10) {
        printf("calc full\n");
        exit(1);
    }
    sprintf(calc_buf + *calc_offset, "(uint32_t)%u", num);
    *calc_offset += max + 10;
}

__attribute__((unused))
static void gen_hex(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset) {
    static const u_int32_t range[] = {
        0x0, 0x10, 0x100, 0x1000, 0x10000, 0x100000, 0x1000000, 0x10000000, 0xffffffff
    };
    
    int max = random_int_range(1, ARRLEN(range));
    int empty_size = buf_size - *offset - 1;
    if (empty_size < 4) {
        buf[*offset] = '0';
        buf[*offset + 1] = '\0';
        return;
    }
    max = max + 4 > empty_size ? empty_size - 4 : max;
    int min = max - 1;

    u_int32_t num = random_int_range(range[min], range[max]);
    // printf("hex: %x\n", num);
    sprintf(buf + *offset, "(0x%x)", num);
    if (calc_buf_size - *calc_offset - 1 < max + 14) {
        printf("calc full\n");
        exit(1);
    }
    sprintf(calc_buf + *calc_offset, "(uint32_t)(0x%x)", num);
    *offset += max + 4;
    *calc_offset += max + 14;
}

__attribute__((unused))
static void gen_op(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset) {
    static const char *ops[] = {"+", "-", "*", "==", "!=", "&&"};
    int index; 
    size_t empty_space = buf_size - *offset - 1;
    if (empty_space >= 2) {
        index = random_int(ARRLEN(ops));
    } else if (empty_space == 1) {
        index = random_int(4);
    } else {
        return;
    }
    strcpy(buf + *offset, ops[index]);
    size_t op_len = strlen(ops[index]);
    *offset += op_len;
    strcpy(calc_buf + *calc_offset, ops[index]);
    *calc_offset += op_len;
}

__attribute__((unused))
static void gen_reg(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset, bool *success) {
    static const char *regs[] = {
        "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    int index = random_int(ARRLEN(regs));
    int len = strlen(regs[index]) + 1;

    if (len > buf_size - *offset - 1) {
        *success = false;
        return;
    }

    sprintf(buf + *offset, "$%s", regs[index]);
    *offset += len;
    sprintf(calc_buf + *calc_offset, "(uint32_t)0");
    *calc_offset += 11;
}

__attribute__((unused))
static void gen_addr(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset, bool *success) {
    if (buf_size - *offset - 1 < 4) {
        *success = false;
        return;
    }

    sprintf(buf + *offset, "*(");
    *offset += strlen(buf + *offset);
    sprintf(calc_buf + *calc_offset, "vaddr_read(");
    *calc_offset += strlen(calc_buf + *calc_offset);

    *success = true;
    do {
        gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
    } while (!*success);
    buf[(*offset)++] = ')';
    calc_buf[(*calc_offset)++] = ')';
    (*offset)++;
    (*calc_offset)++;
}

__attribute__((unused))
static void gen_left_paran(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset, bool *success) {
    if (buf_size - *offset - 1 < 1) {
        *success = false;
        return;
    }
    sprintf(buf + (*offset)++, "(");
    sprintf(calc_buf + (*calc_offset)++, "(");
}

__attribute__((unused))
static void gen_right_paran(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t clac_buf_size, size_t *calc_offset, bool *success) {
    if (buf_size - *offset - 1 < 1) {
        *success = false;
        return;
    }
    sprintf(buf + (*offset)++, ")");
    sprintf(calc_buf + (*calc_offset)++, ")");
}

__attribute__((unused))
static void gen_space(char *buf, size_t buf_size, size_t *offset) {
    int size = buf_size - *offset;
    size = size > 5 ? 5 : size;
    int len = 0;
    if (random_int(2) == 1) {
        len = random_int_range(1, size);
    }
    // printf("gen space: %d\n", len);
    for (int i = 0; i < len; i++) {
        buf[(*offset)++] = ' ';
    }
    buf[*offset] = '\0';
}

__attribute__((unused))
static void gen(char *buf, size_t buf_size, size_t *offset, char *calc_buf, size_t calc_buf_size, size_t *calc_offset, bool *success) {
    static int depth = 0;

    if (*offset >= buf_size - 1) {
        *success = false;
        return;
    }

    size_t len = 0;
    switch (random_int(6)) {
        case 0:
            // printf("choose: 0\n");
            if (depth < 2) {
                gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
                return;
            }
            depth -= depth == 0 ? 0 : 1;
            gen_space(buf, buf_size, offset);
            gen_decimal(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset); 
            gen_space(buf, buf_size, offset);
            // puts(buf);
            break;
        case 1:
            // printf("choose: 1\n");
            if (depth < 2) {
                gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
                return;
            }
            depth -= depth == 0 ? 0 : 1;
            gen_space(buf, buf_size, offset);
            gen_hex(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset);
            gen_space(buf, buf_size, offset);
            // puts(buf);
            break;
        case 2:
            // printf("choose: 2\n");
            gen_space(buf, buf_size, offset);
            gen_left_paran(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            if (!*success) return;
            gen_space(buf, buf_size, offset);
            depth++;
            gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            if (!*success) return;
            gen_space(buf, buf_size, offset);
            gen_right_paran(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            if (!*success) return;
            gen_space(buf, buf_size, offset);
            depth -= depth == 0 ? 0 : 1;
            // puts(buf);
            break;
        case 3:
            // printf("choose: 3\n");
            depth++;
            gen_space(buf, buf_size, offset);
            gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            gen_space(buf, buf_size, offset);
            gen_op(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset);
            gen_space(buf, buf_size, offset);
            depth++;
            gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            gen_space(buf, buf_size, offset);
            depth -= depth == 0 ? 0 : 1;
            // puts(buf);
            break;
        case 4:
            if (depth < 2) {
                gen(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
                return;
            }
            gen_reg(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            break;
        case 5:
            printf("choose: 5\n");
            gen_addr(buf, buf_size, offset, calc_buf, calc_buf_size, calc_offset, success);
            break;
        default:
            break;
    }
}

void gen_rand_expr(char *buf, size_t buf_size, char *calc_buf, size_t calc_buf_size) {
    // 生成随机表达式的逻辑（原始代码未实现，可后续补充）
    static const size_t lens[] = {1, 10, 100, 1000, 10000};
    int max;
    do {
        max = random_int_range(1, ARRLEN(lens));
    } while (lens[max] > buf_size);

    size_t size = random_int_range(lens[max - 1], lens[max] + 1);
    bool success = true;
    size_t offset;
    size_t calc_offset;
    do {
        offset = 0;
        calc_offset = 0;
        gen(buf, buf_size, &offset, calc_buf, calc_buf_size, &calc_offset, &success);
    } while (!success);
}
