#include <common.h>
#include <klib.h>
void easy_test(){
    uintptr_t ans=(uintptr_t)pmm->alloc(2000);
    printf("%d\n",ans);
}
int main() {
  os->init();
  mpe_init(easy_test);
  return 1;
}
