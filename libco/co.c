#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>

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
}coset[CO_MAX];
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
  for(int i=1;i<CO_MAX;i++)coset[i].status=CO_DEAD;
  coset[0].name="main";
  coset[0].status=CO_RUNNING;
  current=&coset[0];
}

void co_wrapper(){
  current->status=CO_RUNNING;
  current->func(current->arg);
  current->status=CO_DEAD;
}
struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  for(int i=1;i<CO_MAX;i++){
    if(coset[i].status==CO_DEAD){
      coset[i].name=(char*)name;
      coset[i].func=func;
      coset[i].arg=arg;
      coset[i].status=CO_NEW;
      return &coset[i];
    }
  }
  debug("no available coroutine.\n");
  return NULL;
}

void co_wait(struct co *co) {
  current->status=CO_WAITING;
  co->waiter=current;
  while(co->status!=CO_DEAD)co_yield();
  if(current->waiter)current->waiter->status=CO_RUNNING;
}

void co_yield() {
  int val=setjmp(current->context);
  if(val==0){
    int random=rand()%CO_MAX;
    do{
      current=&coset[(random++)%CO_MAX];
    }while(current->status>CO_RUNNING);
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
