#include <os.h>
#include <syscall.h>

#include "initcode.inc"

extern int ucreate(task_t *task);
extern void teardown(task_t *task);
extern task_t *cpu_currents[8];
extern task_t *cpu_last[8];
extern spinlock_t tasklock;
extern task_t *cpu_header;
#define current cpu_currents[cpu_current()]
#define last cpu_last[cpu_current()]
uint32_t (*pidset)[2048];
spinlock_t pidlock;
void pid_init(){
  pidset=pmm->alloc(4096);
  memset(pidset,0,4096);
  kmt->spin_init(&pidlock,"pidlock");
  *pidset[0]=1;
}
int pid_alloc(){
  int pid=0;
  kmt->spin_lock(&pidlock);
  for(int i=0;i<2048;i++){
    if(*pidset[i]!=0xffffffff){
      int x=*pidset[i];int j=0;
      for(;j<32;j++){
        if(((x>>j)&1)==0)break;
      }
      *pidset[i]|=1<<j;
      pid=i*32+j;
      break;
    }
  }
  kmt->spin_unlock(&pidlock);
  return pid;
}
void pid_free(int pid){
  kmt->spin_lock(&pidlock);
  int i=pid/32;
  int j=pid%32;
  *pidset[i]-=1<<j;
  kmt->spin_unlock(&pidlock);
}
void pgmap(task_t *task,void *va,void *pa){
  task->va[task->pgcnt]=va;
  task->pa[task->pgcnt]=pa;
  task->pgcnt++;
  debug("map:%p -> %p\n",va,pa);
  map(&task->as,va,pa,MMAP_READ|MMAP_WRITE);
}
Context *pagefault(Event e,Context *c){
  AddrSpace *as=&(current->as);
  void *pa=pmm->alloc(as->pgsize);
  void *va=(void *)(e.ref & ~(as->pgsize-1L));
  if(va==as->area.start){
    memcpy(pa,_init,_init_len);
  }
  pgmap(current,va,pa);
  return NULL;
}
int kputc(task_t *task,char ch){
  putch(ch);
  return 0;
}
int fork(task_t *task){
  task_t *t=pmm->alloc(sizeof(task_t));
  t->pid=pid_alloc();
  t->ppid=task->pid;
  ucreate(t);
  int pid=0;
  uintptr_t rsp0=t->context[0]->rsp0;
  void *cr3=t->context[0]->cr3;
  memcpy(t->context[0],task->context[0],sizeof(Context));
  t->context[0]->rsp0=rsp0;
  t->context[0]->cr3=cr3;
  t->context[0]->GPRx=0;
  for(int i=0;i<task->pgcnt;i++){
    int sz=task->as.pgsize;
    void *va=task->va[i];
    void *pa=task->pa[i];
    void *npa=pmm->alloc(sz);
    memcpy(npa,pa,sz);
    pgmap(t,va,npa);
  }
  pid=t->pid;
  return pid;
}
int wait(task_t *task,int *status){
  for(task_t *t=cpu_header;t!=NULL;t=t->next){
    if(t->ppid==task->pid){
      sem_t *wait_sem=pmm->alloc(sizeof(sem_t));
      kmt->sem_init(wait_sem,"wait_sem",0);
      t->wait_sem=wait_sem;
        kmt->spin_lock(&tasklock);
        last=current;
        current->status=SLEEPING+ZOMBIE;
        kmt->spin_unlock(&tasklock);
      kmt->sem_wait(wait_sem);
        kmt->spin_lock(&tasklock);
        if(last->status>=ZOMBIE&&last->status!=DEAD)last->status-=ZOMBIE;
        current->status=ZOMBIE;
        last=NULL;
        kmt->spin_unlock(&tasklock);
      *status=task->retstatus;
      pmm->free(wait_sem);
      return 0;
    }
  }
  return -1;
}
int exit(task_t *task,int status){
  current->status=DEAD;
  for(int i=0;i<task->pgcnt;i++){
    pmm->free(task->pa[i]);
    task->va[i]=NULL;
    task->pa[i]=NULL;
  }
  task->pgcnt=0;
  if(task->ppid!=0&&task->wait_sem!=NULL){
    for(task_t *t=cpu_header;t!=NULL;t=t->next){
      if(t->pid==task->ppid){
        t->retstatus=status;
        break;
      }
    }
    kmt->sem_signal(task->wait_sem);
  }
  pid_free(task->pid);
  kmt->teardown(task);
  return status;
}
int kill(task_t *task,int pid){
  return 0;
}
void *mmap(task_t *task,void *addr,int length,int prot,int flags){
  return NULL;
}
int getpid(task_t *task){
  return task->pid;
}
void debug_smp(){
  return;
}
int sleep(task_t *task,int seconds){
  kmt->spin_lock(&tasklock);
  uint64_t iotime=io_read(AM_TIMER_UPTIME).us;
  current->wakeuptime=iotime+1000000*seconds;
  debug("[%d]uptime:%d\twakeuptime:%d\n",cpu_current(),iotime/1000000,current->wakeuptime/1000000);
  last=current;
  current->status=SLEEPING+ZOMBIE;
  kmt->spin_unlock(&tasklock);
  yield();
  kmt->spin_lock(&tasklock);
  //printf("%s\t%s\n",last->name,current->name);
  if(last->status>=ZOMBIE&&last->status!=DEAD)last->status-=ZOMBIE;
  current->status=ZOMBIE;
  last=NULL;
  kmt->spin_unlock(&tasklock);
  return 0;
}
int64_t uptime(task_t *task){
  int64_t time=io_read(AM_TIMER_UPTIME).us/1000;
  debug("[%d]uptime:%d\n",cpu_current(),time);
  return time;
}
Context *syscall(Event e,Context *c){
  assert(current->cn==1);
  panic_on(ienabled()==1,"cli");
  iset(true);
  switch(c->GPRx){
    case SYS_kputc:c->GPRx=kputc(current,c->GPR1);break;
    case SYS_exit:c->GPRx=exit(current,c->GPR1);break;
    case SYS_sleep:c->GPRx=sleep(current,c->GPR1);break;
    case SYS_uptime:c->GPRx=uptime(current);break;
    case SYS_fork:c->GPRx=fork(current);break;
    case SYS_getpid:c->GPRx=getpid(current);break;
    case SYS_wait:c->GPRx=wait(current,(int*)c->GPR1);break;
    default:assert(0);
  }
  panic_on(ienabled()==0,"cli");
  iset(false);
  return NULL;
}
void uproc_init(){
  os->on_irq(0,EVENT_SYSCALL,syscall);
  os->on_irq(0,EVENT_PAGEFAULT,pagefault);
  pid_init();
  vme_init((void * (*)(int))pmm->alloc,pmm->free);
  task_t *t=pmm->alloc(sizeof(task_t));
  ucreate(t);
  t->pid=pid_alloc();
  return;
}
MODULE_DEF(uproc) = {
  .init=uproc_init,
  .kputc=kputc,
  .fork=fork,
  .wait=wait,
  .exit=exit,
  .kill=kill,
  .mmap=mmap,
  .getpid=getpid,
  .sleep=sleep,
  .uptime=uptime
};
