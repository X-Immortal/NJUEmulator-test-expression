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
  unsigned result = divide((((uint32_t)0-(uint32_t)1)*(uint32_t)(0x3eba9)),(uint32_t)15144925)+minus((((uint32_t)4294967295+(uint32_t)1)))+divide((((uint32_t)4294967295+(uint32_t)1)),((uint32_t)713046815==((uint32_t)4294967295+(uint32_t)1)+(uint32_t)(0x5aa)&&divide((((uint32_t)4294967295+(uint32_t)1)),(uint32_t)48)))+(uint32_t)(0xad692d)!=((uint32_t)4294967295+(uint32_t)1)-((uint32_t)4294967295+(uint32_t)1)&&divide(((uint32_t)(0x3)+(uint32_t)(0xfa)),(uint32_t)4)-divide(((uint32_t)0*vaddr_read(divide((divide((((uint32_t)0-(uint32_t)1)),(uint32_t)1035638834)),(1minus((uint32_t)242))))),(uint32_t)429538544)*(uint32_t)(0xad)-(1minus((((divide(((uint32_t)33455),(uint32_t)7997682)*(uint32_t)(0x6faee6e3))))))!=((uint32_t)4294967295+(uint32_t)1)==(uint32_t)0*divide((divide((divide((((uint32_t)4294967295+(uint32_t)1)),(uint32_t)885)&&divide(((uint32_t)(0xf)!=divide((((uint32_t)4294967295+(uint32_t)1)),(uint32_t)2923579968)),(uint32_t)730384)!=divide(((uint32_t)4),(uint32_t)3228826)),(uint32_t)5792099)),(uint32_t)9536176)&&(uint32_t)(0x5e0f93)-((uint32_t)0-(uint32_t)1)-(uint32_t)(0xa)+(uint32_t)61966-((uint32_t)4294967295+(uint32_t)1)*divide(((uint32_t)815405),((uint32_t)(0xe6d)))+(uint32_t)2430035314-(uint32_t)25086064*((uint32_t)0-(uint32_t)1)==minus((uint32_t)22711483)+((uint32_t)4294967295+(uint32_t)1)&&((uint32_t)0-(uint32_t)1)==(uint32_t)6+(uint32_t)(0xb)!=((uint32_t)4294967295+(uint32_t)1)==divide((minus((uint32_t)566874993)),((uint32_t)(0x3)))*1minus((uint32_t)773901)!=((uint32_t)4294967295+(uint32_t)1)==(uint32_t)296765==(((uint32_t)0-(uint32_t)1))&&(uint32_t)78565+((uint32_t)(0x1c))+divide((((uint32_t)0-(uint32_t)1)&&(uint32_t)(0x40)),(divide((((uint32_t)0-(uint32_t)1)),((uint32_t)(0x7)))))-1minus((uint32_t)719775)-1minus(((uint32_t)679735&&vaddr_read(((uint32_t)4294967295+(uint32_t)1))==(uint32_t)(0x1911696a)))*((uint32_t)4294967295+(uint32_t)1)==vaddr_read((uint32_t)215)+((uint32_t)4294967295+(uint32_t)1)!=divide((1minus(((uint32_t)(0x473dd0a)))),(uint32_t)483)-(uint32_t)0-vaddr_read((((uint32_t)4294967295+(uint32_t)1)))!=(uint32_t)0&&(uint32_t)(0x2760d02a)!=divide((((uint32_t)0-(uint32_t)1)),(uint32_t)8679023)+1minus((vaddr_read(((uint32_t)4294967295+(uint32_t)1))))==(uint32_t)5892347!=((uint32_t)4294967295+(uint32_t)1)!=divide((minus(((((uint32_t)0-(uint32_t)1))))),(uint32_t)1221917889)-vaddr_read((uint32_t)(0x63f3628)==((uint32_t)0-(uint32_t)1))!=(uint32_t)(0x1d918)==((uint32_t)0-(uint32_t)1)*((uint32_t)4778*((uint32_t)4294967295+(uint32_t)1))!=((uint32_t)4294967295+(uint32_t)1)&&divide((divide((((uint32_t)0-(uint32_t)1)),(uint32_t)8)),(((uint32_t)0-(uint32_t)1)))+1minus((((uint32_t)0-(uint32_t)1)))-divide(((uint32_t)0),(uint32_t)519177580)&&((uint32_t)0-(uint32_t)1)==divide(((uint32_t)606063686),(uint32_t)17)*1minus((minus((uint32_t)8)))!=divide((((uint32_t)4294967295+(uint32_t)1)),(uint32_t)612213)+((uint32_t)0-(uint32_t)1)&&(uint32_t)(0x6ab)!=((uint32_t)4294967295+(uint32_t)1)+((uint32_t)0-(uint32_t)1)==1minus((uint32_t)782)==1minus((uint32_t)328)&&divide(((uint32_t)72),(((uint32_t)0-(uint32_t)1)))-(uint32_t)979==(uint32_t)7441&&divide((((uint32_t)4294967295+(uint32_t)1)),(minus((1minus((uint32_t)60)))))*divide(((uint32_t)(0xb8)),((uint32_t)(0x7c)!=divide(((uint32_t)0),((uint32_t)0))&&minus((((uint32_t)0-(uint32_t)1)))))!=((uint32_t)4294967295+(uint32_t)1)-((uint32_t)0-(uint32_t)1)-(uint32_t)(0x12de98);
  printf("%u", result);
  return 0;
}