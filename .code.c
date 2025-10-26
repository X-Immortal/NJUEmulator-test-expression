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
uint32_t invalid() {
  printf("invalid");
  exit(0);
}int main() {
  unsigned result = (vaddr_read((uint32_t)1190))-((uint32_t)(0x5b2f58))&&((uint32_t)2185904742)-((uint32_t)(0xeb);
  printf("%u", result);
  return 0;
}