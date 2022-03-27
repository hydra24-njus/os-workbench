#include <common.h>
void* tmp[7];
void* tmp0[7];
spinlock_t biglock;
static void *kalloc(size_t size) {
  
  uintptr_t t=(uintptr_t)tmp[cpu_current()];
  int i=0;
  while((1<<i)<size)i++;
  if(t%(1<<i)!=0)t=t+((1<<i)-t%(1<<i));
  tmp[cpu_current()]=(void*)t+size;
  if(tmp[cpu_current()]>tmp[cpu_current()+1])return 0;
  //assert(tmp<heap.end);
  return (void*)t;
}

static void kfree(void *ptr) {
}

static void pmm_init() {
  spinlock_init(&biglock);
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  uintptr_t ttt=pmsize/6;
  for(int i=0;i<5;i++){
    tmp[i]=(void*)((uintptr_t)heap.start+ttt*i);
    tmp0[i]=tmp[i];
  }
  tmp0[6]=heap.end;
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
