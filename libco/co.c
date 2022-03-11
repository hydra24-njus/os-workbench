#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
  #include<stdio.h>
#else
  #define debug(...)
#endif

#define STACK_SIZE 1<<10

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};


struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

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










struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  debug("co_start\n");
  return NULL;
}

void co_wait(struct co *co) {
  debug("co_wait\n");
}

void co_yield() {
  debug("co_yield\n");
}
