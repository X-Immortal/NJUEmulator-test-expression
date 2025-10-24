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
  unsigned result = (vaddr_read((uint32_t)211+vaddr_read(divide((uint32_t)(0x1975c449),(uint32_t)933))*vaddr_read(divide((uint32_t)(0x5370cd7b),((uint32_t)370825)))+(uint32_t)962293&&(uint32_t)(0x17)*(uint32_t)0));
  printf("%u", result);
  return 0;
}