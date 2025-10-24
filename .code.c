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
  unsigned result = (divide((uint32_t)81&&((uint32_t)43761-divide(((uint32_t)9),(uint32_t)1875171849)&&(divide((uint32_t)0,vaddr_read((uint32_t)0))==(uint32_t)(0x3dfb49)&&(uint32_t)0)),divide((uint32_t)1127460706,vaddr_read(divide(((uint32_t)(0x4db8)),vaddr_read((((uint32_t)(0x1ab9)*(uint32_t)18034901)))))))!=(uint32_t)0-(uint32_t)(0x6f6e784a)!=(uint32_t)189);
  printf("%u", result);
  return 0;
}