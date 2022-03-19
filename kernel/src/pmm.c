#include <common.h>
void* tmp;
static void *kalloc(size_t size) {
  int i=0;
  while((1<<i)<size)i++;
  tmp=tmp+(1<<i);
  if(tmp>=heap.end)return NULL;
  return tmp;
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
