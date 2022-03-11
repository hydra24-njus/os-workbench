#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
  #include<stdio.h>
#else
  #define debug(...)
#endif

struct co {
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
