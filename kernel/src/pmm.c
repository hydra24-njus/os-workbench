#include <common.h>
uintptr_t heapstart,heaptr;
uintptr_t heapend,heapstop;
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
void* sbrk(size_t size){
  uintptr_t tmp=heapstart;
  heapstart+=size;
  if(heapstart>=heapend)return NULL;
  return (void*)tmp;
}
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
  size=power2(size);size_t bitsize=bitpos(size);bitsize-=3;int cpu=cpu_current();
  if(size>4096){
    if(size>(16<<20))return NULL;
      lock(&biglock);
      uintptr_t tmp=heaptr;
      if(tmp%size!=0)tmp=tmp+size-tmp%size;
      if(tmp+size>=heapstop)tmp=0;
      else heaptr=tmp+size;
      unlock(&biglock);
      debug("%x %d\n",tmp,size);
      return (void*)tmp;
  }
  page_t* ptr=buddy[cpu].type[bitsize][FREE];int flag=0;
  if(ptr==NULL){
    lock(&biglock);
    ptr=sbrk(8192);
    unlock(&biglock);
    if(ptr==NULL)return NULL;
    buddy[cpu].type[bitsize][FREE]=ptr;
    ptr->next=NULL;ptr->state=FREE;
    ptr->type=size;ptr->bitype=bitsize;
    ptr->max=DATA_SIZE/size;ptr->now=0;
    ptr->cpu=cpu;flag=1;
  }
  if(flag==0){
    while(ptr->next!=NULL){
      if(ptr->max>ptr->now){
        flag=1;
        break;
      }
      ptr=ptr->next;
    }
  }
  if(flag==0){
    lock(&biglock);
    ptr=sbrk(8192);
    unlock(&biglock);
    if(ptr==NULL)return NULL;
    ptr->next=buddy[cpu].type[bitsize][FREE];
    buddy[cpu].type[bitsize][FREE]=ptr;
    ptr->state=FREE;
    ptr->type=size;ptr->bitype=bitsize;
    ptr->max=DATA_SIZE/size;ptr->now=0;
    ptr->cpu=cpu;flag=1;
  }
  for(int i=0;i<ptr->max;i++){
    if(ptr->map[i]==0){
      ptr->map[i]=1;ptr->now++;
      addr=(uintptr_t)ptr+1024+size*i;
      if(size==2048)addr+=1024;
      else if(size==4096)addr+=3072;
      break;
    }
  }
  debug("%x %d %d %d\n",addr,ptr->type,ptr->now,ptr->max);
  return (void*)addr;
}

static void kfree(void *ptr) {
  //printf("free,%x\n",ptr);
  if(ptr==NULL)return;
  uintptr_t addr=(uintptr_t)ptr;
  if(addr>=heapend)return;
  page_t* header=(page_t*)(addr&(~(PAGE_SIZE-1)));
  addr=(addr%PAGE_SIZE);addr=(addr-HEAD_SIZE)/header->type;
  //printf("addr=%x\n",addr);
  header->map[addr]=0;
  header->now--;
  return;
}

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  spinlock_init(&biglock);
  heapstart=(uintptr_t)heap.start;if(heapstart%8192!=0)heapstart+=8192-(heapstart%8192);
  heapstop=(uintptr_t)heap.end;if(heapstop%8192!=0)heapstart-=heapstart%8192;
  heapend=heapstop-(16<<20);heaptr=heapend;
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heapstart = ((uintptr_t)ptr&(~(PAGE_SIZE-1)));
  heapstop   = (uintptr_t)ptr + HEAP_SIZE;heapstop-=heapstop%8192;
  heapend=heapstop-(16<<20);heapend=heapend&(~((16<<20)-1));heaptr=heapend;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heapstart, heapend);
}
#endif


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
