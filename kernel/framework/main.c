#include <common.h>
#include <klib.h>
void easy_test(){
  for(int i=1024;i<=8192;i+=128){
    uintptr_t ans=(uintptr_t)pmm->alloc(i);
    printf("%d\n",ans);
  }
}
int main() {
  os->init();
  mpe_init(easy_test);
  return 1;
}
