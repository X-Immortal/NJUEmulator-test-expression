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
  unsigned result = (divide((((uint32_t)662775)-divide((uint32_t)(0x2431db3),(uint32_t)(0x7339)+(uint32_t)35944570+((uint32_t)724324021-(uint32_t)0-(uint32_t)0))),(uint32_t)280075==(uint32_t)(0x6)));
  printf("%u", result);
  return 0;
}