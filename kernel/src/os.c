#include <os.h>
#include <devices.h>
typedef struct irq_handler{
  int seq;int event;
  handler_t handler;
  struct irq_handler* next;
}irq_handler_t;
void irq_guard_fun(){
  debug("should not reach here.\n");
  assert(0);
}
static irq_handler_t irq_guard={
  .seq=INT32_MIN,
  .event=-1,
  .handler=(handler_t)irq_guard_fun,
  .next=NULL
};

task_t *task_alloc() {
  return pmm->alloc(sizeof(task_t));
}

void tty_reader(void *arg) {
  device_t *tty = dev->lookup(arg);
  char cmd[128], resp[128], ps[16];
  snprintf(ps, 16, "(%s) $ ", arg);
  while (1) {
    tty->ops->write(tty, 0, ps, strlen(ps));
    int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
    cmd[nread] = '\0';
    sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
    tty->ops->write(tty, 0, resp, strlen(resp));
  }
}
void ide(void *arg){
  while(1);
}
static void os_init() {
  ioe_init();
  pmm->init();
  kmt->init();
  //dev->init();
  uproc->init();
  //kmt->create(task_alloc(), "tty_reader", tty_reader, "tty1");
  //kmt->create(task_alloc(), "tty_reader", tty_reader, "tty2");
  //for(int i=0;i<20;i++)kmt->create(task_alloc(),"ide_test",ide,NULL);
}
static void os_run() {
  iset(true);
  while (1);
}
Context *os_trap(Event ev, Context *context){
  panic_on(ienabled()==1,"cli");
  Context *next=NULL;
  for(irq_handler_t* handler_now=&irq_guard;handler_now!=NULL;handler_now=handler_now->next){
    if(handler_now->event==EVENT_NULL||handler_now->event==ev.event){
      Context* r=handler_now->handler(ev,context);
      panic_on(r&&next,"returning multiple contexts");
      if(r)next=r;
    }
  }
  panic_on(!next,"returning NULL context");
  panic_on(ienabled()==1,"cli");
  return next;
}
void os_on_irq(int seq, int event, handler_t handler){
  irq_handler_t *h=pmm->alloc(sizeof(irq_handler_t));
  h->event=event;
  h->seq=seq;
  h->handler=handler;
  h->next=NULL;
  irq_handler_t* prev=&irq_guard;irq_handler_t* p=irq_guard.next;
  while(p!=NULL){
    if(p->seq>seq)break;
    prev=p;
    p=p->next;
  }
  prev->next=h;
  h->next=p;
}
MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
