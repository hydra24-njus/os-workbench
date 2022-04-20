#include <os.h>

static Context *kmt_context_save(Event ev,Context *context){
  return NULL;
}
static Context *kmt_schedule(Event ev,Context *context){
    debug("schedule\n");
  return context;
}
void kmt_init(){
  os->on_irq(INT32_MIN+1,EVENT_NULL,kmt_context_save);
  debug("save\n");
  os->on_irq(INT32_MAX,EVENT_NULL,kmt_schedule);
  return;
}
MODULE_DEF(kmt) = {
 // TODO
 .init=kmt_init,
};
