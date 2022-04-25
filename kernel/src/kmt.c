#include <os.h>
task_t *cpu_currents[8];
task_t *cpu_header[8];
spinlock_t kmt_lock;
#define current cpu_currents[cpu_current()]
#define header cpu_header[cpu_current()]
/*------------------------------------------------
*cpu_currents[i] = idle -> proc0 -> proc1...
  ------------------------------------------------*/

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
  debug("save from CPU(%d)\n",cpu_current());
  //TODO():save context
  current->context=context;
  return NULL;
}
static Context *kmt_schedule(Event ev,Context *context){
  //TODO():线程调度。
  spin_lock(&kmt_lock);
  debug("schedule from CPU(%d)\n",cpu_current());
  task_t *now=current;
  task_t *next=current->next;
  int cpu_next=(cpu_current()+1)%cpu_count();
  if(next==NULL){
    if(header->next==NULL)current=header;
    else current=header->next;
  }
  else current=next;

  if(now==header){
    //do nothing.
  }
  else{
    task_t *prev=header;
    while(prev->next!=now&&prev!=NULL)prev=prev->next;
    panic_on(prev==NULL,"prev==NULL");
    prev->next=now->next;//delete from current list.
    //add to next cpu list.
    now->next=NULL;
    task_t *p=cpu_header[cpu_next];
    while(p->next!=NULL)p=p->next;
    p->next=now;
  }

  for(int i=0;i<cpu_count();i++){
    task_t *p=cpu_header[i];
    while(p!=NULL){printf("%s->",p->name);p=p->next;}
    printf("\n");
  }
  spin_unlock(&kmt_lock);
  return current->context;
}
const char* name[8]={"idle0","idle1","idle2","idle3","idle4","idle5","idle6","idle7"};
void kmt_init(){
  spin_init(&kmt_lock,"kmt_lock");
  //debug("smp=%d\n",cpu_count());
  for(int i=0;i<cpu_count();i++){
    task_t *task=pmm->alloc(sizeof(task_t));
    task->status=IDLE;
    task->name=name[i];
    task->entry=NULL;
    task->next=NULL;
    cpu_header[i]=task;
    cpu_currents[i]=cpu_header[i];
    Area stack={&task->context+1,task+1};
    task->context=kcontext(stack,NULL,NULL);
  }
  os->on_irq(INT32_MIN+1,EVENT_NULL,kmt_context_save);
  os->on_irq(INT32_MAX,EVENT_NULL,kmt_schedule);
  debug("kmt_init finished.\n");
}
static int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg){
  debug("create,%s\n",name);
  int x=rand()%cpu_count();
  task->status=READY;
  task->name=name;
  task->entry=entry;
  task->next=cpu_header[x]->next;
  cpu_header[x]->next=task;
  Area stack={&task->context+1,task+1};
  task->context=kcontext(stack,entry,arg);

  return 0;
}
static void teardown(task_t *task){
  debug("teardown\n");
  return;
}

static void sem_init(sem_t *sem,const char *name,int value){
  debug("sem_init,%s\n",name);
  strcpy(sem->name,name);
  sem->value=value;
  spin_init(&sem->lock,name);
}
static void sem_wait(sem_t *sem){
  spin_lock(&sem->lock);
  debug("sem_wait\n");
  spin_unlock(&sem->lock);
}
static void sem_signal(sem_t *sem){
  spin_lock(&sem->lock);
  debug("sem_signal\n");
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
