#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef unsigned long int uintptr_t;
#define HEAP_SIZE (1 << 27)

typedef struct {
  void *start, *end;
} Area;

Area heap;

typedef struct spinlock{
  int locked;
}spinlock_t;
void spinlock_init(spinlock_t *lk);
void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);