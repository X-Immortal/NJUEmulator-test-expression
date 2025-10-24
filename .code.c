#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
uint32_t vaddr_read(uint32_t addr) {
  if (addr >= 0x80000000 && addr <= 0x87ffffff) {
    return 0;
  }
  printf("invalid");
  exit(0);
}
uint32_t divide(uint32_t a, uint32_t b) {
  if (b == 0) {
    printf("invalid");
    exit(0);
  }
  return a / b;
}
uint32_t minus(uint32_t num) {
  return -num;
}
int main() {
  unsigned result = vaddr_read((uint32_t)4294967295);
  printf("%u", result);
  return 0;
}