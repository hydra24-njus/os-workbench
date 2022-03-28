#include <common.h>

static void os_init() {
  pmm->init();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  pmm->alloc(4096);
  void* tmp1=pmm->alloc(4096);
  void* tmp2=pmm->alloc(4096);
  pmm->free(tmp1);pmm->free(tmp2);
  pmm->alloc(4096);pmm->alloc(4096);pmm->alloc(4096);pmm->alloc(4096);
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
