#include <common.h>
uintptr_t heaptr;
uintptr_t heapend;
spinlock_t biglock;
#define HEAD_SIZE 1024
#define PAGE_SIZE 8192
#define DATA_SIZE (PAGE_SIZE-HEAD_SIZE)



//数据结构
enum{
  p16=0,p32,p64,p128,p256,p512,p1024,p2048,p4096
};
enum{
  FREE=0,FULL
};
struct cpu_t{
  void* type[10][2];
  spinlock_t cpulock;
}buddy[8];//smp<=8
struct page_t{
  union{
    uint8_t size[PAGE_SIZE];
    struct{
      void* next;void* prev;
      size_t type;
      uint8_t map[512];
      int max,now,cur,cpu,bitype;
    };
  };
};

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
void add2full(struct page_t* ptr){
  int cpu=ptr->cpu,btp=ptr->bitype;
  buddy[cpu].type[btp][FREE]=ptr->next;
  ptr->next=buddy[cpu].type[btp][FULL];
  buddy[cpu].type[btp][FULL]=ptr;
}
void add2free(struct page_t* ptr){
  int cpu=ptr->cpu,btp=ptr->bitype;
  struct page_t* tmp=buddy[cpu].type[btp][FREE];
  struct page_t* tmp2=ptr->prev;
  tmp2->next=ptr->next;
  while(tmp->next!=NULL)tmp=tmp->next;
  tmp->next=ptr;ptr->next=NULL;
}

//static_assert(sizeof(bool)==1);
static void *kalloc(size_t size1) {
  uintptr_t addr=0;int cpu=cpu_current();
  size_t size=power2(size1);
  if(size>4096){
  if(size>=(16<<20))return NULL;
    lock(&biglock);
    addr=slowpath_alloc(size);
    debug("addr=%x\t%d\n",addr,addr);
    unlock(&biglock);
    return (void*)addr;
  }
  int bitsize=3;
  while((1<<bitsize)!=size){bitsize++;}
  bitsize-=4;
  struct page_t* ptr=buddy[cpu].type[bitsize][FREE];
  if (ptr == NULL){ //该cpu没有页
    lock(&biglock);
    ptr = sbrk(PAGE_SIZE);
    unlock(&biglock);
    if(ptr==0){
      addr=0;
      goto ret;
    }
    ptr->cpu=cpu;
    ptr->type=size;
    ptr->bitype=bitsize;
    buddy[cpu].type[bitsize][FREE]=ptr;
    ptr->next=NULL;ptr->prev=NULL;
    ptr->now=0;ptr->max=DATA_SIZE/size;ptr->cur=0;
  }
  for(int i=0;i<ptr->max;i++){
    int j=(i+ptr->cur)%ptr->max;
    if((ptr->map[j])==0){//找到页中空闲位置，计算地址
      ptr->map[j]=1;
      ptr->now++;ptr->cur=(j+1)%ptr->max;
      addr=(uintptr_t)ptr+1024+ptr->type*j;
      if(size==2048)addr+=1024;
      else if(size==4096)addr+=3072;
      if(ptr->now==ptr->max)add2full(ptr);
      break;
    }
  }
  ret:
  debug("%x\tsize=%d\taddr=%x\t%d\t%d\n",ptr,size,addr,addr,ptr->now);
  return (void*)addr;
}

static void kfree(void *ptr) {
  uintptr_t addr=(uintptr_t)ptr;
  if(addr>=heapend)return;
  struct page_t* header=(struct page_t*)(addr-addr%PAGE_SIZE);//考虑位操作优化
  addr=(addr%PAGE_SIZE);
  if(header->type==2048){//2048 4096 6144
    int i=addr/2048;
    header->map[i-1]=0;
    header->cur=i-1;
  }
  else if(header->type==4096){
    int i=addr/4096;
    header->map[i-1]=0;
    header->cur=i-1;
  }
  else{
    int i=(addr-1024)/header->type;
    header->map[i]=0;
    header->cur=i;
  }
  if(header->now==header->max)add2free(header);
  header->now--;
  return;
}


#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  //init
  spinlock_init(&biglock);
  heaptr=(uintptr_t)heap.start;heapend=(uintptr_t)heap.end;
  memset(buddy,0,sizeof(buddy));
  //init
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
#define cpu_current() tptr.id
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heap.start = ptr;
  heap.end   = ptr + HEAP_SIZE;
  heaptr=(uintptr_t)heap.start;
  heapend=(uintptr_t)heap.end;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
