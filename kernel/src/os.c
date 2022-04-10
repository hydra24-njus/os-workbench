#include <common.h>

static void os_init() {
  pmm->init();
  kmt->init();
  dev->init();
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    //putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  while (1) ;
}
Context *os_trap(Event ev, Context *context){
  return NULL;
}
void os_on_irq(int seq, int event, handler_t handler){
  return;
}
MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
