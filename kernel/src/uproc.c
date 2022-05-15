#include <os.h>
#include <syscall.h>

#include "initcode.inc"

extern int ucreate(task_t *task);
extern void teardown(task_t *task);
extern task_t *cpu_currents[8];
extern task_t *cpu_last[8];
#define current cpu_currents[cpu_current()]
#define last cpu_last[cpu_current()]

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
  return 0;
}
int wait(task_t *task,int *status){
  return 0;
}
int exit(task_t *task,int status){
  current->status=DEAD;
  for(int i=0;i<task->pgcnt;i++){
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
  int64_t wakeup=io_read(AM_TIMER_UPTIME).us+1000000*seconds;
  while(wakeup>io_read(AM_TIMER_UPTIME).us){
    last=current;
    current->status=ZOMBIE;
  }
  return 0;
}
int64_t uptime(task_t *task){
  return 0;
}
Context *syscall(Event e,Context *c){
  panic_on(ienabled()==1,"cli");
  iset(true);
  //r_panic_on(1,"syscall:%d",c->GPRx);
  switch(c->GPRx){
    case SYS_kputc:kputc(current,c->GPR1);break;
    case SYS_exit:exit(current,c->GPR1);break;
    case SYS_sleep:sleep(current,c->GPR1);break;
    default:assert(0);
  }
  iset(false);
  return NULL;
}
void uproc_init(){
  os->on_irq(0,EVENT_SYSCALL,syscall);
  os->on_irq(0,EVENT_PAGEFAULT,pagefault);
  vme_init((void * (*)(int))pmm->alloc,pmm->free);
  ucreate(pmm->alloc(sizeof(task_t)));
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
