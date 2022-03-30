#include <common.h>
#include <lock.h>
#include <buddysystem.h>

#define MAGIC_NUM 0x7355608
#define PAGE_SIZE (64<<10)
spinlock_t biglock;

typedef union{
  struct{
    int magic;
    void* next;
    int type;
    int max,now,cpu,state;
    spinlock_t page_lock;
    uint8_t map[2048];
  };
  struct{
    uint8_t header[4096];
    uint8_t data[(64<<10)-4096];
  };
  uint8_t page[64<<10];
}apage_t;

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
  uintptr_t addr=0;
  size=power2(size);
  if(size>(16<<20))return NULL;
  if(size>(4<<10)){
    if(size<(64<<10))size=64<<10;
    lock(&biglock);
    addr=(uintptr_t)buddy_alloc(size);
    unlock(&biglock);
    return (void*)addr;
  }
  return (void*)addr;
}

static void kfree(void *ptr) {
  buddy_free(ptr);
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
