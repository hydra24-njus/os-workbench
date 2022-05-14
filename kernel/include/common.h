#ifndef TEST
#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#else
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>

//摘出来pmm.c需要的am函数
static inline int atomic_xchg(int *addr, int newval) {
  int result;
  // printf("test xchg\n");
  asm volatile("lock xchg %0, %1"
               : "+m"(*addr), "=a"(result)
               : "1"(newval)
               : "cc", "memory");
  return result;
}

typedef struct {
  void *start, *end;
} Area;

typedef unsigned long int uintptr_t;
#define HEAP_SIZE (1 << 27)
#define cpu_current() 1
#endif
#ifdef LOCAL_MACHINE
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define PURPLE 35
#define CYAN 36
#define WHITE 37

  #define debug(...) printf(__VA_ARGS__)
  #define r_panic_on(cond, ...) \
    c_panic_on(RED, cond, __VA_ARGS__);

  #define c_panic_on(color, cond, ...) \
  do{ \
    if(cond) {\
        printf("\033[36m[cpu(%d)]:\033[0m", cpu_current());\
        printf("\033[%dm", color); \
        printf(__VA_ARGS__); \
        printf("\033[0m"); \
        halt(1);\
    }\
  }while(0); \

#else
  #define debug(...)
  #define r_panic_on(...)
#endif
#define MAGIC 0x114514
enum{
  READY=0,RUNNING,WAITING,SLEEPING,IDLE,ZOMBIE,DEAD
};
struct task {
  // TODO
  union{
    struct {
    int status,pid;
    const char *name;//for debug
    AddrSpace as;
    void *va[64],*pa[64];
    int pgcnt;
    struct task *next;
    Context   *context;
  };
  uint8_t stack[4096];
  };
};

struct spinlock {
  // TODO
  int locked;
  char* name;
  int cpu;
};

struct semaphore {
  // TODO
  int value;
  struct spinlock lock;
  char *name;
  int head,tail,qlen;
  struct task *queue[64];
};
