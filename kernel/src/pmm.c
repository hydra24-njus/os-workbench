#include <common.h>
#include <lock.h>
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
    void* next;
    int type,bitype;
    int max,now,cpu,state;
    uint8_t map[896];
  }__attribute__((packed));
}page_t;
//辅助函数
/*void* sbrk(size_t size){
  uintptr_t tmp=heapstart;
  heapstart+=size;
  if(heapstart>=heapend)return NULL;
  return (void*)tmp;
}*/
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
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heap.start = ptr;
  heap.end   = ptr + HEAP_SIZE;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
