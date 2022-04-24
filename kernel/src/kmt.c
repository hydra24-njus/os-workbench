#include <os.h>
task_t *cpu_currents[8];
task_t header;
spinlock_t kmt_lock;
#define current cpu_currents[cpu_current()]

static void spin_init(spinlock_t *lk,const char *name){
  strcpy(lk->name,name);
  lk->locked=0;
  lk->intr=0;
  lk->cpu=-1;
}
static void spin_lock(spinlock_t *lk){
  int i=ienabled();
  iset(false);
  while(atomic_xchg(&(lk->locked),1));
  lk->intr=i;
  lk->cpu=cpu_current();
}
static void spin_unlock(spinlock_t *lk){
  int i=lk->intr;
  atomic_xchg(&(lk->locked),0);
  if(i)iset(true);
}
static Context *kmt_context_save(Event ev,Context *context){
  debug("save\n");
  //TODO():save context
  if(!current)current=&header;
  else current->context=context;
  panic_on(current==NULL,"current==NULL");
  if(current->next!=NULL)current=current->next;
  debug("save finished\n");
  return NULL;
}
static Context *kmt_schedule(Event ev,Context *context){
  //TODO():线程调度。
  //debug("schedule\n");
  debug("schedule.current=%x\n",current);
  return current->context;
}

void kmt_init(){
  spin_init(&kmt_lock,"kmt_lock");
  task_t* task=&header;
  Area stack={&task->context,&task+sizeof(task_t)-sizeof(uint32_t)};
  task->context=kcontext(stack,NULL,NULL);
  os->on_irq(INT32_MIN+1,EVENT_NULL,kmt_context_save);
  os->on_irq(INT32_MAX,EVENT_NULL,kmt_schedule);
  debug("kmt_init finished.\n");
}
static int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg){
  debug("create\n");
  task->status=0;
  task->name=name;
  task->entry=entry;
  task->next=header.next;
  header.next=task;
  Area stack={&task->context,&task+sizeof(task_t)-sizeof(uint32_t)};
  task->context=kcontext(stack,entry,arg);
  debug("1\n");
  return 0;
}
static void teardown(task_t *task){
  return;
}

static void sem_init(sem_t *sem,const char *name,int value){

}
static void sem_wait(sem_t *sem){

}
static void sem_signal(sem_t *sem){

}
MODULE_DEF(kmt) = {
 // TODO
 .init=kmt_init,
 .create=create,
 .teardown=teardown,
 .spin_init=spin_init,
 .spin_lock=spin_lock,
 .spin_unlock=spin_unlock,
 .sem_init=sem_init,
 .sem_wait=sem_wait,
 .sem_signal=sem_signal
};
