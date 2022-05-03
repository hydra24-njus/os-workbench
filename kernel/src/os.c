#include <os.h>
typedef struct irq_handler{
  int seq;int event;
  handler_t handler;
  struct irq_handler* next;
}irq_handler_t;
spinlock_t kmt_lock;
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
void fun(void *i){
  spinlock_t lk1,lk2;
  kmt->spin_init(&lk1,"lk1");
  kmt->spin_init(&lk2,"lk2");
  kmt->spin_lock(&lk1);
  while(1){
    printf("test lk1\n");
  }
  kmt->spin_unlock(&lk1);
}
static void os_init() {
  pmm->init();
  kmt->init();
  kmt->spin_init(&kmt_lock,"中断处理");
  //dev->init();
  //for(uintptr_t i=0;i<32;i++)kmt->create(pmm->alloc(sizeof(task_t)),"func",fun,(void *)i);
  kmt->create(pmm->alloc(sizeof(task_t)),"func",fun,(void *)1);
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  iset(true);
  while (1)yield();
}
Context *os_trap(Event ev, Context *context){
  Context *next=NULL;
  for(irq_handler_t* handler_now=&irq_guard;handler_now!=NULL;handler_now=handler_now->next){
    if(handler_now->event==EVENT_NULL||handler_now->event==ev.event){
      Context* r=handler_now->handler(ev,context);
      panic_on(r&&next,"returning multiple contexts");
      if(r)next=r;
    }
  }
  panic_on(!next,"returning NULL context");
  return next;
}
void os_on_irq(int seq, int event, handler_t handler){
  debug("os_on_irq\n");
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
