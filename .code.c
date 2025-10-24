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
int main() {
  unsigned result = divide(divide((uint32_t)4884!=((uint32_t)0),(uint32_t)0),vaddr_read(divide(vaddr_read(divide((uint32_t)0,divide((vaddr_read((uint32_t)0)==(uint32_t)0-divide(vaddr_read((uint32_t)0)!=divide((uint32_t)(0x7930),((uint32_t)0)),(uint32_t)(0x1eb03))),(uint32_t)(0xa8e)))),(uint32_t)0)))&&(uint32_t)0&&(uint32_t)0&&(uint32_t)(0x6188ad42);
  printf("%u", result);
  return 0;
}