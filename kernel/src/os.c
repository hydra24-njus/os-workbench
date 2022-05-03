#include <os.h>
typedef struct irq_handler{
  int seq;int event;
  handler_t handler;
  struct irq_handler* next;
}irq_handler_t;
spinlock_t kmtlock;
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
  spinlock_t lk1;
  kmt->spin_init(&lk1,"lk1");
  kmt->spin_lock(&lk1);
  for(int k=0;k<10;k++){
    printf("%d",i);
  }
  kmt->spin_unlock(&lk1);
  while(1)yield();
}
static inline task_t *task_alloc() {
  return pmm->alloc(sizeof(task_t));
}

sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal

void producer(void *arg) { while (1) { P(&empty); putch('('); V(&fill);  } }
void consumer(void *arg) { while (1) { P(&fill);  putch(')'); V(&empty); } }

static void os_init() {
  pmm->init();
  kmt->init();
  kmt->spin_init(&kmtlock,"中断处理");
  for(uintptr_t i=0;i<10;i++){
    kmt->create(task_alloc(),"fun",fun,(void *)i);
  }
}
static void os_run() {
  iset(true);
  while (1)yield();
}
Context *os_trap(Event ev, Context *context){
  kmt->spin_lock(&kmtlock);
  Context *next=NULL;
  for(irq_handler_t* handler_now=&irq_guard;handler_now!=NULL;handler_now=handler_now->next){
    if(handler_now->event==EVENT_NULL||handler_now->event==ev.event){
      Context* r=handler_now->handler(ev,context);
      panic_on(r&&next,"returning multiple contexts");
      if(r)next=r;
    }
  }
  panic_on(!next,"returning NULL context");
  kmt->spin_unlock(&kmtlock);
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
