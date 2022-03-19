#include <common.h>
void* tmp;
static void *kalloc(size_t size) {
  uintptr_t t=(uintptr_t)tmp;
  while(t%size!=0)t++;
  printf("t=%d\t",t);
  tmp=(void*)t+size;
  return (void*)t;
}

static void kfree(void *ptr) {
}

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  tmp=heap.start;
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heap.start = ptr;
  heap.end   = ptr + HEAP_SIZE;
  tmp=heap.start;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
