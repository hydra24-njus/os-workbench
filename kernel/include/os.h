#include <common.h>

struct task {
  // TODO
  union{
    struct {
    int status;
    const char *name;
    struct task *next;
    Context   *context;
  };
  uint8_t stack[4096];
  };
};

struct spinlock {
  // TODO
  int locked;
  int intr;//cli/sti
  char* name;
  int cpu;
};

struct semaphore {
  // TODO
};
