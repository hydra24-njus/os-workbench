#include <common.h>
#include <lock.h>
#include <buddysystem.h>
spinlock_t biglock;



size_t power2(size_t size){
  size--;
  size|=size>>1;
  size|=size>>2;
  size|=size>>4;
  size|=size>>8;
  size|=size>>16;
  size++;
  if(size<16)size=16;
  return size;
}
unsigned int bitpos(size_t size){
  int i=2;
  while((1<<i)<size)i++;
  return i;
}




static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {

}

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  spinlock_init(&biglock);
  buddy_init((uintptr_t)heap.start,(uintptr_t)heap.end);
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
Area heap;
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heap.start = ptr;
  heap.end   = ptr + HEAP_SIZE;
  buddy_init((uintptr_t)heap.start,(uintptr_t)heap.end);
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
