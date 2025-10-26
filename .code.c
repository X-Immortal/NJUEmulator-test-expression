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
}
int main() {
  unsigned result = ((uint32_t)0)*(uint32_t)0!=(((uint32_t)4294967295+(uint32_t)1))-(uint32_t)5202!=(uint32_t)8481744*divide(((uint32_t)(0x4b)),(uint32_t)884400)&&(uint32_t)0+(uint32_t)7099&&(uint32_t)(0x2527)+((uint32_t)0-(uint32_t)1)!=(((uint32_t)4294967295+(uint32_t)1)&&(uint32_t)0)!=(uint32_t)(0xcc)!=(uint32_t)5&&(uint32_t)2*(uint32_t)0&&((uint32_t)0-(uint32_t)1)&&minus((uint32_t)2071073)*(uint32_t)46==divide((((uint32_t)4294967295+(uint32_t)1)),(uint32_t)738173809)&&(uint32_t)(0xe8072)-1+((uint32_t)0-(uint32_t)1)-((uint32_t)4294967295+(uint32_t)1)*((uint32_t)0-(uint32_t)1)==(uint32_t)0!=((uint32_t)4294967295+(uint32_t)1)+(uint32_t)383093005*(uint32_t)0&&(uint32_t)0==1&&divide(((uint32_t)(0x3fe52)),(divide(((uint32_t)9695890),(uint32_t)44336361)))!=divide(((uint32_t)668231315),(((uint32_t)4294967295+(uint32_t)1)));
  printf("%u", result);
  return 0;
}