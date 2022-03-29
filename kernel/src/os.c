#include <common.h>
static void os_init() {
  pmm->init();
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  for (int i = 1; i <= 27; i++)pmm->alloc(1024);
  pmm->free((void*)0x300800);
  pmm->alloc(1024);
  pmm->alloc(1024);
  pmm->alloc(1024);
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
