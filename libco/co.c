#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdbool.h>
#ifdef LOCAL_MACHINE
  #include<stdio.h>
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug(...)
#endif
#define STACK_SIZE (1<<16)
#define CO_MAX 128
enum co_status {
  CO_NEW = 1,
  CO_RUNNING,
  CO_WAITING,
  CO_DEAD,
};

struct co {
  char *name;
  void (*func)(void *);
  void *arg;
  enum co_status status;
  struct co *    waiter;
  jmp_buf        context;
  __attribute((aligned(16)))uint8_t        stack[STACK_SIZE];
};
struct ptr{
  uintptr_t this;
  bool flag;
}coset[CO_MAX];
int coroutine_num=0;
struct co *current;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg) : "memory"
#endif
  );
}

void __attribute__((constructor)) co_init(){
  for(int i=0;i<CO_MAX;i++)coset[i].flag=false;
  coset[0].this=(uintptr_t)malloc(sizeof(struct co));
  ((struct co*)coset[0].this)->name="main";
  ((struct co*)coset[0].this)->status=CO_RUNNING;
  current=(struct co*)coset[0].this;
  coset[0].flag=true;
}

void co_wrapper(){
  current->status=CO_RUNNING;
  current->func(current->arg);
  current->status=CO_DEAD;
  if(current->waiter)current->waiter->status=CO_RUNNING;
  co_yield();
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  for(int i=1;i<CO_MAX;i++){
    if(coset[i].flag==false){
      coset[i].flag=true;
      coset[i].this=malloc(sizeof(struct co));
      ((struct co*)coset[i].this)->name=(char*)name;
      ((struct co*)coset[i].this)->func=func;
      ((struct co*)coset[i].this)->arg=arg;
      ((struct co*)coset[i].this)->status=CO_NEW;
      return (struct co*)coset[i].this;
    }
  }
  return NULL;
}

void co_wait(struct co *co) {
  current->status=CO_WAITING;
  co->waiter=current;
  while(co->status!=CO_DEAD)co_yield();
  free(co);
}

void co_yield() {
  int val=setjmp(current->context);
  if(val==0){
    do{
      current=(struct co*)coset[rand()%CO_MAX].this;
    }while(current->status!=CO_DEAD||current->status!=CO_WAITING);
    switch(current->status){
      case CO_NEW:
        stack_switch_call(current->stack+STACK_SIZE-sizeof(uintptr_t),co_wrapper,(uintptr_t)NULL);
        break;
      case CO_RUNNING:
        longjmp(current->context,1);
        break;
      default:
      	debug("error status.\n");
      	break;
     }
  }
  else{
    //do nothing;
  }
}
