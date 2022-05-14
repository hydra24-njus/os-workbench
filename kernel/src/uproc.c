#include <os.h>
#include <syscall.h>

#include "initcode.inc"
extern int ucreate(task_t *task);
Context *syscall(Event e,Context *c){
  panic("ucreate");
  return NULL;
}
Context *pagefault(Event e,Context *c){
  panic("pagefault");
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
