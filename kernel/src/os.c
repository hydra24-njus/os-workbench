#include <common.h>
static void os_init() {
  pmm->init();
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  void* loc;
  for (int i = 1; i <= 100; i++) {
    size_t a = rand() % 128 ;
    loc = pmm->alloc(a);
    printf("a=%d,loc=%x\n",a,loc);
    if (i % 5 == 0) pmm->free(loc);
  }
  printf("end\n");
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
