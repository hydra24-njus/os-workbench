#include <common.h>
#include <klib.h>
void easy_test(){
  for(int i=63;i<=8192;i+=64){
    uintptr_t ans=(uintptr_t)pmm->alloc(i);
    printf("%d\n",ans);
  }
}
int main() {
  os->init();
  mpe_init(easy_test);
  return 1;
}
