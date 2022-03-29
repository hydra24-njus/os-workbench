#include <common.h>
static void os_init() {
  pmm->init();
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  void* loc;
  for (int i = 1; i <= 10000; i++) {
    size_t a = (rand() % 2) ? rand() % 2000 + 1000 : rand() % 100 + 50;
    loc = pmm->alloc(a);
    if (i % 5 == 0) pmm->free(loc);
  }
  printf("end\n");
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
