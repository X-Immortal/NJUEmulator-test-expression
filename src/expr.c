#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/expr.h"

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

__attribute__((unused))
static inline u_int32_t random_int_range(u_int32_t begin, u_int32_t end) {
    if (end < begin) {
        printf("Invalid range: from %u to %u\n", begin, end);
        return 0;
    }
    return (rand() % (end - begin)) + begin;
}

__attribute__((unused))
static inline u_int32_t random_int(u_int32_t n) {
    return random_int_range(0, n);
}

__attribute__((unused))
static int gen_decimal(char *buf, size_t buf_size, size_t offset) {
    static const u_int32_t range[] = {
        1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
    };
    int max = random_int_range(1, ARRLEN(range));
    int min = max - 1;
    if (buf_size - offset - 1 < max) {
        return 0;
    }
    u_int32_t num = random_int_range(range[min], range[max]);
    sprintf(buf + offset, "%d", num);
    return max;
}

__attribute__((unused))
static int gen_hex(char *buf, size_t buf_size, size_t offset) {
    static const u_int32_t range[] = {
        1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
    };
    int max = random_int_range(1, ARRLEN(range));
    int min = max - 1;
    u_int32_t num = random_int_range(range[min], range[max]);
    char temp[ARRLEN(range) + 3];
    sprintf(temp, "0x%x", num);
    int len = strlen(temp);
    if (buf_size - offset - 1 < len) {
        return 0;
    }
    memcpy(buf + offset, temp, len * sizeof(*buf));
    return len;
}

__attribute__((unused))
static bool gen_op(char *buf, size_t buf_size, size_t offset) {
    return false;
}

__attribute__((unused))
static bool gen_reg(char *buf, size_t buf_size, size_t offset) {

    return false;
}

__attribute__((unused))
static bool gen_addr(char *buf, size_t buf_size, size_t offset) {

    return false;
}

__attribute__((unused))
static bool gen_left_paran(char *buf, size_t buf_size, size_t offset) {

    return false;
}

__attribute__((unused))
static bool gen_right_paran(char *buf, size_t buf_size, size_t offset) {

    return false;
}

__attribute__((unused))
static bool gen(char *buf, size_t buf_size, size_t offset) {
    
    return false;
}

void gen_rand_expr(char *buf, size_t buf_size) {
    // 生成随机表达式的逻辑（原始代码未实现，可后续补充）
    int index = gen_decimal(buf, buf_size, 0);
    // gen_hex(buf, buf_size, 0);
}
