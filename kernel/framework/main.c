#include <common.h>
#include <klib.h>
void test(){
  for(int i=0;i<100;i++)malloc(i);
}
int main() {
  os->init();
  mpe_init(test);
  return 1;
}
