#include <os.h>
#include <syscall.h>

#include "initcode.inc"

extern int ucreate(task_t *task);
extern void teardown(task_t *task);
extern task_t *cpu_currents[8];
extern spinlock_t tasklock;
#define current cpu_currents[cpu_current()]
int pid_cnt=1;
int alloc_pid(){
  return pid_cnt++;
}
void free_pid(int pid){

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
  t->pid=alloc_pid();
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
  return -1;
}
int exit(task_t *task,int status){
  current->status=DEAD;
  for(int i=0;i<task->pgcnt;i++){
    unprotect(&task->as);
    map(&task->as,task->va[i],task->pa[i],MMAP_NONE);
    pmm->free(task->pa[i]);
    task->va[i]=NULL;
    task->pa[i]=NULL;
  }
  task->pgcnt=0;
  kmt->teardown(task);
  return 0;
}
int kill(task_t *task,int pid){
  return 0;
}
void *mmap(task_t *task,void *addr,int length,int prot,int flags){
  return NULL;
}
int getpid(task_t *task){
  return 0;
}
int sleep(task_t *task,int seconds){
  kmt->spin_lock(&tasklock);
  uint64_t iotime=io_read(AM_TIMER_UPTIME).us;
  current->wakeuptime=iotime+1000000*seconds;
  debug("[%d]uptime:%d\twakeuptime:%d\n",cpu_current(),iotime/1000000,current->wakeuptime/1000000);
  current->status=SLEEPING;
  kmt->spin_unlock(&tasklock);
  yield();
  return 0;
}
int64_t uptime(task_t *task){
  int64_t time=io_read(AM_TIMER_UPTIME).us/1000;
  debug("[%d]uptime:%d\n",cpu_current(),time);
  return time;
}
Context *syscall(Event e,Context *c){
  assert(current->cn==1);
  //panic_on(ienabled()==1,"cli");
  //iset(true);
  switch(c->GPRx){
    case SYS_kputc:c->GPRx=kputc(current,c->GPR1);break;
    case SYS_exit:c->GPRx=exit(current,c->GPR1);break;
    case SYS_sleep:c->GPRx=sleep(current,c->GPR1);break;
    case SYS_uptime:c->GPRx=uptime(current);break;
    case SYS_fork:c->GPRx=fork(current);break;
    case SYS_wait:c->GPRx=wait(current,(int *)(c->GPR1));break;
    default:assert(0);
  }
  //panic_on(ienabled()==0,"cli");
  //iset(false);
  return NULL;
}
void uproc_init(){
  os->on_irq(0,EVENT_SYSCALL,syscall);
  os->on_irq(0,EVENT_PAGEFAULT,pagefault);
  vme_init((void * (*)(int))pmm->alloc,pmm->free);
  task_t *task=pmm->alloc(sizeof(task_t));
  task->pid=alloc_pid();
  ucreate(task);
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
