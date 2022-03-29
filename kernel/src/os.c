#include <common.h>
static void os_init() {
  pmm->init();
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  for (int i = 1; i <= 224; i++) {
    pmm->alloc(31);
  }
  uintptr_t loco=0x300400;
  for(int i=0;i<224;i++){
    pmm->free((void*)(loco+i*32));
  }
  for (int i = 1; i <= 224; i++) {
    pmm->alloc(31);
  }
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
