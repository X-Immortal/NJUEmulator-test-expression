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
  unsigned result = vaddr_read((uint32_t)9430783)+(uint32_t)(0xc)+((uint32_t)4294967295+(uint32_t)1)+(divide(((uint32_t)4294967295+(uint32_t)1),(uint32_t)1213012482)==divide(((uint32_t)(0x8)+(uint32_t)(0x9)),(uint32_t)174))+(divide(((uint32_t)4294967295+(uint32_t)1),((uint32_t)0-(uint32_t)1))+(divide(divide((((uint32_t)4294967295+(uint32_t)1)==(((uint32_t)0-(uint32_t)1)*vaddr_read((uint32_t)0))),(uint32_t)9063),(uint32_t)450069)+((uint32_t)0-(uint32_t)1)))+((((uint32_t)4294967295+(uint32_t)1)+minus((uint32_t)0))*(uint32_t)(0xc))+((uint32_t)0-(uint32_t)1)+(minus((uint32_t)0))+(uint32_t)0+((((divide(((uint32_t)0-(uint32_t)1),(uint32_t)93))))!=(uint32_t)(0xa9b4642))+(uint32_t)(0x7)+minus(minus((vaddr_read((uint32_t)77830)!=(uint32_t)(0x675))))+(minus(divide((uint32_t)(0x6995),(uint32_t)42))==divide(divide(((uint32_t)4294967295+(uint32_t)1),(uint32_t)87),(uint32_t)50989))+((uint32_t)4294967295+(uint32_t)1)+(uint32_t)900459300+vaddr_read(((uint32_t)4294967295+(uint32_t)1))+((uint32_t)0-(uint32_t)1)+(divide(((uint32_t)4294967295+(uint32_t)1),(uint32_t)78)==((uint32_t)0-(uint32_t)1))+(uint32_t)0+((uint32_t)0!=minus(((((uint32_t)0-(uint32_t)1)+divide((uint32_t)42810,(uint32_t)7)))))+divide((uint32_t)27,(uint32_t)0)+minus(minus(minus(((uint32_t)0-(uint32_t)1))+((uint32_t)4294967295+(uint32_t)1)+((uint32_t)0-(uint32_t)1)+((uint32_t)4294967295+(uint32_t)1)+divide(divide(minus((uint32_t)0),(uint32_t)9),(uint32_t)169681)));
  printf("%u", result);
  return 0;
}