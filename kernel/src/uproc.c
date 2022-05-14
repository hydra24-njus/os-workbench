#include <os.h>
#include <syscall.h>

#include "initcode.inc"
extern int ucreate(task_t *task);
extern task_t *cpu_currents[8];
#define current cpu_currents[cpu_current()]
Context *syscall(Event e,Context *c){
  r_panic_on(1,"syscall:%d",c->GPRx);
  switch(c->GPRx){
    case SYS_kputc:break;
    default:assert(0);
  }
  return NULL;
}
void pgmap(task_t *task,void *va,void *pa){
  task->va[task->pgcnt]=va;
  task->pa[task->pgcnt]=pa;
  task->pgcnt++;
  printf("map:%p -> %p\n",va,pa);
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
void uproc_init(){
  os->on_irq(0,EVENT_SYSCALL,syscall);
  os->on_irq(0,EVENT_PAGEFAULT,pagefault);
  vme_init((void * (*)(int))pmm->alloc,pmm->free);
  ucreate(pmm->alloc(sizeof(task_t)));
  return;
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
  return 0;
}
int64_t uptime(task_t *task){
  return 0;
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
