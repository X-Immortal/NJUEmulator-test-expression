#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
uint32_t vaddr_read(uint32_t addr) {
  if (addr >= 0x80000000 && addr <= 0x87ffffff) {
    return 0;
  }
  printf("invalid\n");
  exit(0);
}
uint32_t divide(uint32_t a, uint32_t b) {
  if (b == 0) {
    printf("invalid\n");
    exit(0);
  }
  return a / b;
}
int main() {
  unsigned result = (uint32_t)71(uint32_t)3042742451(uint32_t)3031567470(uint32_t)7647767(uint32_t)(0xf)(uint32_t)41;
  printf("%u", result);
  return 0;
}