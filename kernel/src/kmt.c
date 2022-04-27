#include <os.h>
task_t *cpu_currents[8];
task_t *cpu_idle[8];
task_t *cpu_header;
#define current cpu_currents[cpu_current()]
#define idle cpu_idle[cpu_current()]
/*------------------------------------------------
*cpu_currents[i] = idle -> proc0 -> proc1...
  ------------------------------------------------*/

int holding(struct spinlock *lock){
  int r = 0;
  int i = ienabled();
  iset(false);
  r  = (lock->locked) && (lock->cpu == cpu_current());
  if(i == 1) iset(true);
  return r;
}
static void spin_init(spinlock_t *lk,const char *name){
  strcpy(lk->name,name);
  lk->locked=0;
  lk->intr=0;
  lk->cpu=-1;
}
static void spin_lock(spinlock_t *lk){
  int i=ienabled();
  iset(false);
  r_panic_on(holding(lk), "lock(%s) tried to acquire itself while holding.\n",lk->name);
  while(atomic_xchg(&(lk->locked),1));
  __sync_synchronize();
  lk->intr=i;
  lk->cpu=cpu_current();
  panic_on(ienabled() != 0, "cli() failed in kmt_lock!\n");
  panic_on(lk->locked != 1, "lock failed!\n");
}
static void spin_unlock(spinlock_t *lk){
  r_panic_on(!holding(lk), "lock(%s) tried to release itself without holding.\n",lk->name);
  lk->cpu = -1;
  int i=lk->intr;
  atomic_xchg(&(lk->locked),0);
  if(i)iset(true);
}
static Context *kmt_context_save(Event ev,Context *context){
  //debug("save from CPU(%d)\n",cpu_current());
  //TODO():save context
  r_panic_on(current==NULL,"current==NULL");
  current->context=context;
  if(current->status==RUNNING)current->status=READY;
  return NULL;
}
static Context *kmt_schedule(Event ev,Context *context){
  //TODO():线程调度。
  task_t *p=current;
  while(p!=NULL){
    p=p->next;
    if(p->status==READY)break;
  }
  if(p!=NULL)current=p;
  else{
    p=cpu_header;
    while(p!=NULL){
      p=p->next;
      if(p->status==READY)break;
    }
    if(p!=NULL)current=p;
  }
  if(p==NULL)current=idle;
  panic_on(p->status!=READY&&p->status!=IDLE,"schedule error");
  if(p!=idle)current->status=RUNNING;
  return current->context;
}
const char* name[8]={"idle0","idle1","idle2","idle3","idle4","idle5","idle6","idle7"};
void kmt_init(){
  //debug("smp=%d\n",cpu_count());
  for(int i=0;i<cpu_count();i++){
    task_t *task=pmm->alloc(sizeof(task_t));
    task->status=IDLE;
    task->name=name[i];
    task->entry=NULL;
    task->next=NULL;
    cpu_idle[i]=task;
    cpu_currents[i]=cpu_idle[i];
    Area stack={&task->context+1,task+1};
    task->context=kcontext(stack,NULL,NULL);
  }
  os->on_irq(INT32_MIN+1,EVENT_NULL,kmt_context_save);
  os->on_irq(INT32_MAX,EVENT_NULL,kmt_schedule);
  debug("kmt_init finished.\n");
}
static int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg){
  debug("create,%s\n",name);
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
  return;
}

static void sem_init(sem_t *sem,const char *name,int value){
  debug("sem_init,%s\n",name);
  strcpy(sem->name,name);
  sem->value=value;sem->count=value;
  spin_init(&sem->lock,name);
}
static void sem_wait(sem_t *sem){
  spin_lock(&sem->lock);
  bool flag =false;
  if(sem->count <= 0){
    flag = true;
    current->sem=sem;
    current->status = SLEEPING; 
  }
  sem->count--;
  spin_unlock(&sem->lock);
  if(flag)yield();
}
static void sem_signal(sem_t *sem){
  spin_lock(&sem->lock);
  sem->count++;
  //panic_on(sem->count>sem->value,"sem error");
  task_t *p = cpu_header;
  while(p) {
    if(p->sem == sem){
      p->status = READY;
      p->sem = NULL;
      break;
    }
    p = p->next;
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
