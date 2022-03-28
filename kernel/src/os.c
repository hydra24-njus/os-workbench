#include <common.h>

static void os_init() {
  pmm->init();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  void* tmp=pmm->alloc(16);
  void* tmp1=pmm->alloc(16);
  void* tmp2=pmm->alloc(16);
  for(int i=0;i<256;i++)pmm->alloc(18);
  printf("\n\n");
  pmm->free(tmp1);pmm->free(tmp2);
  pmm->alloc(16);pmm->alloc(16);pmm->alloc(17);pmm->alloc(18);
  pmm->free(tmp);
  pmm->alloc(16);pmm->alloc(16);pmm->alloc(17);pmm->alloc(18);
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
