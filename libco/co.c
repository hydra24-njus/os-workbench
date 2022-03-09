#include "co.h"
#include <stdlib.h>
#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
  #include<stdio.h>
#else
  #define debug()
#endif

struct co {
};

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  debug("co_start");
  return NULL;
}

void co_wait(struct co *co) {
  debug("co_wait");
}

void co_yield() {
  debug("co_yield");
}
