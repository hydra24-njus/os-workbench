#include <os.h>
#include <syscall.h>

#include "initcode.inc"
void uproc_init(){
  //vme_init(pmm->alloc,pmm->free);
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
