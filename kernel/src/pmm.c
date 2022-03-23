#include <common.h>
#include <lock.h>
void* tmp;
struct spinlock_t biglock;
static void *kalloc(size_t size) {
  lock(&biglock);
  uintptr_t t=(uintptr_t)tmp;
  int i=0;
  while((1<<i)<size)i++;
  t=t+((1<<i)-t%(1<<i));
  tmp=(void*)t+size;
  unlock(&biglock);
  return (void*)t;
}

static void kfree(void *ptr) {
}

static void pmm_init() {
  tmp=heap.start;
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
