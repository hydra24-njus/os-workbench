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
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug(...)
#endif
