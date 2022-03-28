#include <common.h>
uintptr_t heaptr;
uintptr_t heapend;
spinlock_t biglock;

//数据结构
struct buddy_table{
  void* p32;
  void* p64;
  void* p128;
  void* p256;
  void* p512;
  void* p1024;
  void* p2048;
  void* p4096;
}buddy[8];//smp<=8
struct page_t{
  union{
    uint8_t size[8192];
    struct{
      void* prev;
      void* next;
      size_t type;
      uint64_t map[4];
      int max,now;
    };
  };
};

//辅助函数
void* sbrk(int size){
  uintptr_t tmp=heaptr;
  heaptr+=size;
  if(heaptr>(uintptr_t)heapend)return NULL;
  else return (void*)tmp;
}
unsigned int power2(unsigned int size){
  size--;
  size|=size>>1;
  size|=size>>2;
  size|=size>>4;
  size|=size>>8;
  size|=size>>16;
  size++;
  if(size<32)size=32;
  return size;
}
void* new_page(){//TODO();
  void* tmp;
  tmp=sbrk(8192);
  debug("new page.tmp=%x.\n",tmp);
  return tmp;
}
void* slowpath_alloc(size_t size){
  lock(&biglock);
  heapend-=size;
  heapend-=heapend%size;
  unlock(&biglock);
  debug("large_addr=%x\t%d\n",heapend,heapend);
  return (void*)heapend;
}

static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {

}

static void pmm_init() {
  //init
  spinlock_init(&biglock);
  heaptr=(uintptr_t)heap.start;heapend=(uintptr_t)heap.end;
  for(int i=0;i<8;i++){
    buddy[i].p32=buddy[i].p64=buddy[i].p128=buddy[i].p256=buddy[i].p512=buddy[i].p1024=buddy[i].p2048=buddy[i].p4096=NULL;
  }
  //init
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
