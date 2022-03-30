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
    void* prev;void* next;
    int type,bitype;
    int max,now,cur,cpu,state;
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
void add2full(page_t* ptr){
  if(ptr->state==FULL)return;
  //debug("add to free(%x):\nfull:",ptr);
  size_t bitype=ptr->bitype,cpu=ptr->cpu;
  page_t* tmp=buddy[cpu].type[bitype][FREE];
  if(tmp==ptr){
    buddy[cpu].type[bitype][FREE]=ptr->next;
    if(ptr->next!=NULL)((page_t*)ptr->next)->prev=NULL;
  }
  else{
    while(tmp->next!=ptr)tmp=tmp->next;
    tmp->next=ptr->next;
    if(ptr->next!=NULL)((page_t*)ptr->next)->prev=tmp;
  }

  tmp=buddy[cpu].type[bitype][FULL];
  if(tmp==NULL){
    buddy[cpu].type[bitype][FULL]=ptr;
    ptr->next=NULL;ptr->prev=NULL;
  }
  else{
    while(tmp->next!=NULL)tmp=tmp->next;
    tmp->next=ptr;ptr->prev=tmp;ptr->next=NULL;
  }
  ptr->state=FULL;/*
  page_t* cont=buddy[cpu].type[bitype][FULL];
  while(cont!=NULL){debug("%x->",cont);cont=cont->next;}
  debug("\nfree:");
  cont=buddy[cpu].type[bitype][FREE];
  while(cont!=NULL){debug("%x->",cont);cont=cont->next;}
  debug("\n");*/
}
void add2free(page_t* ptr){
  if(ptr->state==FREE)return;
  //debug("add to free(%x):\nfull:",ptr);
  size_t bitype=ptr->bitype,cpu=ptr->cpu;
  page_t* tmp=buddy[cpu].type[bitype][FULL];
  if(tmp==ptr){
    buddy[cpu].type[bitype][FULL]=ptr->next;
    if(ptr->next!=NULL)((page_t*)ptr->next)->prev=NULL;
  }
  else{
    while(tmp->next!=ptr)tmp=tmp->next;
    tmp->next=ptr->next;
    if(ptr->next!=NULL)((page_t*)ptr->next)->prev=tmp;
  }

  tmp=buddy[cpu].type[bitype][FREE];
  if(tmp==NULL){
    buddy[cpu].type[bitype][FREE]=ptr;
    ptr->next=NULL;ptr->prev=NULL;
  }
  else{
    while(tmp->next!=NULL)tmp=tmp->next;
    tmp->next=ptr;ptr->prev=tmp;ptr->next=NULL;
  }
  ptr->state=FREE;/*
  page_t* cont=buddy[cpu].type[bitype][FULL];
  while(cont!=NULL){debug("%x->",cont);cont=cont->next;}
  debug("\nfree:");
  cont=buddy[cpu].type[bitype][FREE];
  while(cont!=NULL){debug("%x->",cont);cont=cont->next;}
  debug("\n");*/
}


static void *kalloc(size_t size) {
  uintptr_t addr=0;
  size=power2(size);size_t bitsize=bitpos(size);bitsize-=3;int cpu=cpu_current();
  if(size>4096){
    if(size>(16<<20))return NULL;
      lock(&biglock);
      uintptr_t tmp=heaptr;
      heaptr+=size;
      if(heapstart>=heapend)tmp=0;
      unlock(&biglock);
      return (void*)tmp;
  }
  page_t* ptr=buddy[cpu].type[bitsize][FREE];
  if(ptr==NULL){
    lock(&biglock);
    ptr=sbrk(8192);
    unlock(&biglock);
    if(ptr==NULL)return NULL;
    buddy[cpu].type[bitsize][FREE]=ptr;
    ptr->prev=NULL;ptr->next=NULL;ptr->state=FREE;
    ptr->type=size;ptr->bitype=bitsize;
    ptr->max=DATA_SIZE/size;ptr->now=0;
    ptr->cpu=cpu;ptr->cur=0;
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
  if(ptr->now==ptr->max)add2full(ptr);
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
  add2free(header);
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
  heapend   = (uintptr_t)ptr + HEAP_SIZE;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heapstart, heapend);
}
#endif


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
