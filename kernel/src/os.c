#include <common.h>

static void os_init() {
  pmm->init();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  void* tmp1;void* tmp2;
  tmp1=pmm->alloc(2048);
  tmp2=pmm->alloc(2048);
  pmm->free(tmp2);
  tmp2=pmm->alloc(2048);
  pmm->free(tmp1);
  for(int i=0;i<4;i++)pmm->alloc(2048);
  printf("end\n");
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
