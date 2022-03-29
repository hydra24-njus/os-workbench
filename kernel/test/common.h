#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//摘出来pmm.c需要的am函数
#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug(...)
#endif
#ifdef TEST
static inline int atomic_xchg(int *addr, int newval) {
  int result;
  // printf("test xchg\n");
  asm volatile("lock xchg %0, %1"
               : "+m"(*addr), "=a"(result)
               : "1"(newval)
               : "cc", "memory");
  return result;
}

#endif

//为了能让test成功编译(提取了os.c)
#define MODULE(mod)                           \
  typedef struct mod_##mod##_t mod_##mod##_t; \
  extern mod_##mod##_t *mod;                  \
  struct mod_##mod##_t

#define MODULE_DEF(mod)                \
  extern mod_##mod##_t __##mod##_obj;  \
  mod_##mod##_t *mod = &__##mod##_obj; \
  mod_##mod##_t __##mod##_obj

MODULE(os) {
  void (*init)();
  void (*run)();
};

MODULE(pmm) {
  void (*init)();
  void *(*alloc)(size_t size);
  void (*free)(void *ptr);
};

typedef unsigned long int uintptr_t;
typedef long int intptr_t;
#define HEAP_SIZE (1 << 27)

typedef struct {
  void *start, *end;
} Area;

Area heap;

struct spinlock_t{
  int locked;
};
static inline int xchg(volatile int *addr,int newval){
  int result;
  asm volatile("lock;xchgl %0,%1":
  	       "+m"(*addr),"=a"(result):
  	       "1"(newval):
  	       "cc");
  return result;
}

typedef struct spinlock{
  int locked;
}spinlock_t;
void spinlock_init(spinlock_t *lk);
void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);