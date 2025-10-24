#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "../include/expr.h"

static char buf[65536] = {};
static char code_buf[65536 + 512] = {};
static char calc_buf[65536] = {};
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"uint32_t vaddr_read(uint32_t addr) {\n"
"  if (addr >= 0x80000000 && addr <= 0x87ffffff) {\n"
"    return 0;\n"
"  }\n"
"  printf(\"invalid\\n\");\n"
"  exit(0);\n"
"}\n"
"uint32_t divide(uint32_t a, uint32_t b) {\n"
"  if (b == 0) {\n"
"    printf(\"invalid\\n\");\n"
"    exit(0);\n"
"  }\n"
"  return a / b;\n"
"}\n"
"int main() {\n"
"  unsigned result = %s;\n"
"  printf(\"%%u\", result);\n"
"  return 0;\n"
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr(buf, sizeof(buf), calc_buf, sizeof(calc_buf));

    sprintf(code_buf, code_format, calc_buf);

    FILE *fp = fopen("./.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc ./.code.c -o ./.expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
