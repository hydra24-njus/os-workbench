#include <common.h>
#define MAGIC 0x114514
struct task {
  // TODO
  union{
    struct {
    int status;
    const char *name;
    struct task *next;
    void (*entry)(void*);
    Context   *context;
  };
  uint8_t stack[4096-sizeof(uint32_t)];
  uint8_t canary[4];
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
