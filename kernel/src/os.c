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
void test_kmt(){
  debug("xxx\n");
}
#define P kmt->sem_wait
#define V kmt->sem_signal
sem_t empty,fill;
void producer() { while (1) { P(&empty); putch('('); V(&fill);  } }
void consumer() { while (1) { P(&fill);  putch(')'); V(&empty); } }
void* task_alloc(){
  return pmm->alloc(sizeof(task_t));
}
void fun(void *i){
    while (1) {
    printf("Thread-%s%d on CPU #%d\n", "func", i, cpu_current());
    for (int volatile i = 0; i < 100000; i++) ;
  }
}

static void os_init() {
  pmm->init();
  kmt->init();
  kmt->spin_init(&kmt_lock,"中断处理");
  //for(uintptr_t i=0;i<32;i++)kmt->create(pmm->alloc(sizeof(task_t)),"func",fun,(void *)i);
  //dev->init();
  kmt->sem_init(&empty, "empty", 2);  // 缓冲区大小为 5
  kmt->sem_init(&fill,  "fill",  0);
  for (int i = 0; i < 4; i++) // 4 个生产者
    kmt->create(task_alloc(), "producer", producer, NULL);
  for (int i = 0; i < 5; i++) // 5 个消费者
    kmt->create(task_alloc(), "consumer", consumer, NULL);
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  iset(true);
  while (1) ;
}
Context *os_trap(Event ev, Context *context){
  kmt->spin_lock(&kmt_lock);
  Context *next=NULL;
  for(irq_handler_t* handler_now=&irq_guard;handler_now!=NULL;handler_now=handler_now->next){
    if(handler_now->event==EVENT_NULL||handler_now->event==ev.event){
      Context* r=handler_now->handler(ev,context);
      panic_on(r&&next,"returning multiple contexts");
      if(r)next=r;
    }
  }
  panic_on(!next,"returning NULL context");
  //panic_on(sane_context(next),"returning to invalid context");
  kmt->spin_unlock(&kmt_lock);
  return next;
}
void debug_handler(){
  irq_handler_t *tmp=&irq_guard;
  while(tmp!=NULL){
    debug("%d->",tmp->seq);
    tmp=tmp->next;
  }
  debug("\n");
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
  //debug_handler();
}
MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
