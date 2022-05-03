#include <os.h>
task_t *cpu_currents[8];
task_t *cpu_idle[8];
task_t *cpu_header;
#define current cpu_currents[cpu_current()]
#define idle cpu_idle[cpu_current()]
/*------------------------------------------------
*cpu_currents[i] = idle -> proc0 -> proc1...
  ------------------------------------------------*/

static int ncli[8]={0};
static int intena[8]={0};
int holding(struct spinlock *lock){
  int r = 0;
  int i = ienabled();
  iset(false);
  r  = (lock->locked) && (lock->cpu == cpu_current());
  if(i == 1) iset(true);
  return r;
}
void pushcli(){
  int i=ienabled();
  iset(false);
  if(ncli[cpu_current()]==0)intena[cpu_current()]=i;
  ncli[cpu_current()]+=1;
}
void popcli(){
  if(ienabled())panic("error");
  if(--ncli[cpu_current()]<0)panic("error");
  if(ncli[cpu_current()]==0 && intena[cpu_current()]) iset(true);
}
static void spin_init(spinlock_t *lk,const char *name){
  debug("spin_init\n");
  strcpy(lk->name,name);
  lk->locked=0;
  lk->cpu=-1;
}
static void spin_lock(spinlock_t *lk){
  debug("spin_lock\n");
  pushcli();
  r_panic_on(holding(lk), "lock(%s) tried to acquire itself while holding.\n",lk->name);
  while(atomic_xchg(&(lk->locked),1));
  __sync_synchronize();
  lk->cpu=cpu_current();
  panic_on(ienabled() != 0, "cli() failed in kmt_lock!\n");
  r_panic_on(lk->locked != 1, "lock(%s) failed!\n",lk->name);
}
static void spin_unlock(spinlock_t *lk){
  debug("spin_unlock\n");
  r_panic_on(!holding(lk), "lock(%s) tried to release itself without holding.\n",lk->name);
  lk->cpu = -1;
  __sync_synchronize();
  atomic_xchg(&(lk->locked),0);
  popcli();
}
static Context *kmt_context_save(Event ev,Context *context){
  debug("kmt_context_save\n");
  r_panic_on(current==NULL,"current==NULL");
  current->context=context;
  if(current->status==RUNNING)current->status=READY;
  return NULL;
}
static Context *kmt_schedule(Event ev,Context *context){
  debug("kmt_schedule\n");
  //TODO():线程调度。
  task_t *p=cpu_header;
  while(p->status!=READY){
    p=p->next;
  }
  if(p==NULL)p=idle;
  current=p;
  return current->context;
}
const char* name[8]={"idle0","idle1","idle2","idle3","idle4","idle5","idle6","idle7"};
void kmt_init(){
  for(int i=0;i<cpu_count();i++){
    task_t *task=pmm->alloc(sizeof(task_t));
    task->status=IDLE;
    task->name=name[i];
    task->entry=NULL;
    task->next=NULL;
    cpu_idle[i]=task;
    cpu_currents[i]=task;
    Area stack={&task->context+1,task+1};
    task->context=kcontext(stack,NULL,NULL);
  }
  os->on_irq(INT32_MIN+1,EVENT_NULL,kmt_context_save);
  os->on_irq(INT32_MAX,EVENT_NULL,kmt_schedule);
}
static int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg){
  debug("create\n");
  task->status=READY;
  task->name=name;
  task->entry=entry;
  if(cpu_header==NULL)cpu_header=task;
  else{
    task->next=cpu_header->next;
    cpu_header->next=task;
  }
  Area stack={&task->context+1,task+1};
  task->context=kcontext(stack,entry,arg);
  task_t *p=cpu_header;
  while(p!=NULL){
    debug("%s->",p->name);
    p=p->next;
  }
  debug("\n");
  return 0;
}
static void teardown(task_t *task){
  debug("teardown\n");
  assert(0);
  return;
}

void enqueue(sem_t *sem,task_t *task){
  sem->queue[sem->tail++]=task;
  if(sem->tail>=sem->qlen)sem->tail-=sem->qlen;
}
task_t* dequeue(sem_t *sem){
  task_t *ret=sem->queue[sem->head++];
  if(sem->head>=sem->qlen)sem->head-=sem->qlen;
  return ret;
}
static void sem_init(sem_t *sem,const char *name,int value){
  debug("sem_init\n");
  strcpy(sem->name,name);
  sem->value=value;
  spin_init(&sem->lock,name);
  sem->qlen=sizeof(sem->queue)/sizeof(task_t*);
  sem->head=0;sem->tail=0;
}
static void sem_wait(sem_t *sem){
  debug("sem_wait\n");
  spin_lock(&sem->lock);
  sem->value--;
  if(sem->value<0){
    enqueue(sem,current);
    current->status=SLEEPING;
  }
  spin_unlock(&sem->lock);
  if(sem->value<0){
    yield();
  }
}
static void sem_signal(sem_t *sem){
  debug("sem_signal\n");
  spin_lock(&sem->lock);
  sem->value++;
  if(sem->value<=0){
    task_t *task=dequeue(sem);
    task->status=READY;
  }
  spin_unlock(&sem->lock);
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
