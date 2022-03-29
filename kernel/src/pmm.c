#include <common.h>
uintptr_t heaptr;
uintptr_t heapend;
spinlock_t biglock;
#define HEAD_SIZE 1024
#define PAGE_SIZE 8192
#define DATA_SIZE (PAGE_SIZE-HEAD_SIZE)

//数据结构
enum{
  p8=0,p16,p32,p64,p128,p256,p512,p1024,p2048,p4096
};
enum{
  FREE=0,FULL
};
struct cpu_t{
  void* type[10][2];
}buddy[8];//smp<=8
typedef union{
  uint8_t size[PAGE_SIZE];
  struct{
    void* prev;void* next;
    int type,bitype;
    int max,now,cur,cpu;
    uint8_t map[896];
  };
}page_t;

//辅助函数
void* sbrk(int size){
  uintptr_t tmp=heaptr;
  heaptr+=size;
  if(heaptr>heapend)return NULL;
  return (void*)tmp;
}
unsigned int power2(unsigned int size){
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
uintptr_t slowpath_alloc(size_t size){
  uintptr_t tmp=heapend;
  tmp-=size;
  tmp-=heapend%size;
  if(tmp<=heaptr)return 0;
  heapend=tmp;
  return heapend;
}


//static_assert(sizeof(bool)==1);
static void *kalloc(size_t size) {
  size=power2(size);
  if(size>4096){
    if(size>(16<<20))return NULL;
    uintptr_t tmp=heapend;
    tmp-=size;
    tmp-=tmp%size;
    lock(&biglock);
    if(tmp<=heaptr){unlock(&biglock);return NULL;}
    heapend=tmp;
    unlock(&biglock);
    return (void*)tmp;
  }
  return NULL;
}

static void kfree(void *ptr) {
  
}

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {

  heaptr=(uintptr_t)heap.start;heapend=(uintptr_t)heap.end;

  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heaptr = (uintptr_t)ptr;
  heapend   = (uintptr_t)ptr + HEAP_SIZE;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heaptr, heapend);
}
#endif


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
