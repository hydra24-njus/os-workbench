#include <common.h>
#include <lock.h>
#include <buddysystem.h>

#define MAGIC_NUM 0x7355608
#define PAGE_SIZE (64<<10)
#define HEAD_SIZE 4096
#define DATA_SIZE 61440
spinlock_t biglock;

enum{
  p16=0,p32,p64,p128,p256,p512,p1024,p2048,p4096,MAX_SIZE
};

typedef union{
  struct{
    int magic;
    void* next;
    int type;
    int max,now,cpu;
    spinlock_t page_lock;
    uint8_t map[2048];
  };
  struct{
    uint8_t header[4096];
    uint8_t data[(64<<10)-4096];
  };
  uint8_t page[64<<10];
}apage_t;
struct cpu_t{
  void* link_head[MAX_SIZE];
  spinlock_t cpu_lock[MAX_SIZE];
}percpu[8];



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
  int i=3;
  while((1<<i)<size)i++;
  return i;
}


static void *kalloc(size_t size) {
  uintptr_t addr=0;size_t bitsize=bitpos(size);bitsize-=4;int cpu=cpu_current();
  size=power2(size);
  if(size>(16<<20))return NULL;
  //big memory
  if(size>(4<<10)){
    if(size<(64<<10))size=64<<10;
    lock(&biglock);
    addr=(uintptr_t)buddy_alloc(size);
    unlock(&biglock);
    return (void*)addr;
  }
  //fast path
  apage_t* ptr=percpu[cpu].link_head[bitsize];
  if(ptr==NULL){
    lock(&biglock);
    ptr=buddy_alloc(PAGE_SIZE);
    unlock(&biglock);
    if(ptr==NULL)return NULL;//没有空闲空间
    lock(&(percpu[cpu].cpu_lock[bitsize]));
    percpu[cpu].link_head[bitsize]=ptr;
    ptr->magic=MAGIC_NUM;ptr->next=NULL;ptr->type=size;
    ptr->max=DATA_SIZE/size;ptr->now=0;ptr->cpu=cpu;
    spinlock_init(&(ptr->page_lock));
    unlock(&(percpu[cpu].cpu_lock[bitsize]));
  }
  else{
    while(ptr!=NULL){
      if(ptr->max>ptr->now)break;
      ptr=ptr->next;
    }
    if(ptr==NULL){
      lock(&biglock);
      ptr=buddy_alloc(PAGE_SIZE);
      unlock(&biglock);
      if(ptr==NULL)return NULL;//没有空闲空间
      lock(&(percpu[cpu].cpu_lock[bitsize]));
      ptr->next=percpu[cpu].link_head[bitsize];
      percpu[cpu].link_head[bitsize]=ptr;
      ptr->magic=MAGIC_NUM;ptr->type=size;
      ptr->max=DATA_SIZE/size;ptr->now=0;ptr->cpu=cpu;
      spinlock_init(&(ptr->page_lock));
      unlock(&(percpu[cpu].cpu_lock[bitsize]));
    }
  }
  for(int i=0;i<ptr->max;i++){
    if(ptr->map[i]==0){
      lock(&(ptr->page_lock));
      ptr->map[i]=1;ptr->now++;
      unlock(&(ptr->page_lock));
      addr=(uintptr_t)ptr+4096+size*i;
      break;
    }
  }
  return (void*)addr;
}

static void kfree(void *ptr) {
  apage_t* tmp= (apage_t*)((uintptr_t)ptr&(~(PAGE_SIZE-1)));
  if(tmp->magic!=MAGIC_NUM){
    lock(&biglock);
    buddy_free(ptr);
    unlock(&biglock);
  }
  else{
    uintptr_t addr=(uintptr_t)ptr;
    apage_t* header=(apage_t*)(addr&(~(PAGE_SIZE-1)));
    addr=addr%PAGE_SIZE;addr=(addr-HEAD_SIZE)/header->type;
    lock(&(header->page_lock));
    header->map[addr]=0;
    header->now--;
    unlock(&(header->page_lock));
  }
  print_mem_tree();
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
