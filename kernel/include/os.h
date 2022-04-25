#include <common.h>
#define MAGIC 0x114514
enum{
  READY=0,RUNNING,SLEEPING,IDLE
};
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
  int value;
  struct spinlock lock;
  char *name;
  int cpu;
};
