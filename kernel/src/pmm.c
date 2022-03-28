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
    uint8_t size[16384];
    struct{
      void* next;
      size_t type;
      bool map[480];
      int max,now,cur;
    };
  };
};

//辅助函数
void* sbrk(int size){
  uintptr_t tmp=heaptr;
  heaptr+=size;
  if(heaptr>heapend)return NULL;
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
uintptr_t slowpath_alloc(size_t size){
  uintptr_t tmp=heapend;
  tmp-=size;
  tmp-=heapend%size;
  if(tmp<=heaptr)return 0;
  heapend=tmp;
  return heapend;
}

static void *kalloc(size_t size1) {
  uintptr_t addr=0;
  size_t size=power2(size1);
  if(size>4096){
  if(size>=(16<<20))return NULL;
    lock(&biglock);
    addr=slowpath_alloc(size);
    debug("addr=%x\t%d\n",addr,addr);
    unlock(&biglock);
    return (void*)addr;
  }
  struct page_t* ptr=NULL;
  switch(size){
    case 32  :ptr=buddy[cpu_current()].p32  ;break;
    case 64  :ptr=buddy[cpu_current()].p64  ;break;
    case 128 :ptr=buddy[cpu_current()].p128 ;break;
    case 256 :ptr=buddy[cpu_current()].p256 ;break;
    case 512 :ptr=buddy[cpu_current()].p512 ;break;
    case 1024:ptr=buddy[cpu_current()].p1024;break;
    case 2048:ptr=buddy[cpu_current()].p2048;break;
    case 4096:ptr=buddy[cpu_current()].p4096;break;
  }
  if (ptr == NULL){ //该cpu没有页
    lock(&biglock);
    ptr = sbrk(16384);
    if(ptr==0){
      unlock(&biglock);
      addr=0;
      goto ret;
    }
    switch (size){
    case 32  :ptr->type=32  ;buddy[cpu_current()].p32=ptr  ;break;
    case 64  :ptr->type=64  ;buddy[cpu_current()].p64=ptr  ;break;
    case 128 :ptr->type=128 ;buddy[cpu_current()].p128=ptr ;break;
    case 256 :ptr->type=256 ;buddy[cpu_current()].p256=ptr ;break;
    case 512 :ptr->type=512 ;buddy[cpu_current()].p512=ptr ;break;
    case 1024:ptr->type=1024;buddy[cpu_current()].p1024=ptr;break;
    case 2048:ptr->type=2048;buddy[cpu_current()].p2048=ptr;break;
    case 4096:ptr->type=4096;buddy[cpu_current()].p4096=ptr;break;
    }
    ptr->next=NULL;
    unlock(&biglock);
    ptr->now=0;ptr->max=15360/size;ptr->cur=0;
  }
  else{
    while(ptr->next!=NULL){
      if(ptr->now<ptr->max)break;
      ptr=ptr->next;
    }
  }
  if(ptr->now>=ptr->max){//没有空闲页
    lock(&biglock);
    struct page_t* tmp = sbrk(16384);
    if(tmp==NULL){
      unlock(&biglock);
      goto ret;
    }
    switch (size){
      case 32  :tmp->type=32  ;tmp->next=buddy[cpu_current()].p32  ;buddy[cpu_current()].p32=tmp  ;break;
      case 64  :tmp->type=64  ;tmp->next=buddy[cpu_current()].p64  ;buddy[cpu_current()].p64=tmp  ;break;
      case 128 :tmp->type=128 ;tmp->next=buddy[cpu_current()].p128 ;buddy[cpu_current()].p128=tmp ;break;
      case 256 :tmp->type=256 ;tmp->next=buddy[cpu_current()].p256 ;buddy[cpu_current()].p256=tmp ;break;
      case 512 :tmp->type=512 ;tmp->next=buddy[cpu_current()].p512 ;buddy[cpu_current()].p512=tmp ;break;
      case 1024:tmp->type=1024;tmp->next=buddy[cpu_current()].p1024;buddy[cpu_current()].p1024=tmp;break;
      case 2048:tmp->type=2048;tmp->next=buddy[cpu_current()].p2048;buddy[cpu_current()].p2048=tmp;break;
      case 4096:tmp->type=4096;tmp->next=buddy[cpu_current()].p4096;buddy[cpu_current()].p4096=tmp;break;
    }
    unlock(&biglock);
    ptr=tmp;
    ptr->now=0;ptr->max=15360/size;ptr->type=size;ptr->cur=0;
  }
  if(ptr==NULL)return NULL;
  for(int i=0;i<ptr->max;i++){
    int j=(i+ptr->cur)%ptr->max;
    if((ptr->map[j])==false){//找到页中空闲位置，计算地址
      ptr->map[j]=true;
      ptr->now++;ptr->cur=(j+1)%ptr->max;
      addr=(uintptr_t)ptr+1024+ptr->type*j;
      if(size==2048)addr+=1024;
      else if(size==4096)addr+=3072;
      break;
    }
  }
  ret:
  debug("%x\tsize=%d\taddr=%x\t%d\t%d\n",ptr,size,addr,addr,ptr->now);
  return (void*)addr;
}

static void kfree(void *ptr) {
  uintptr_t addr=(uintptr_t)ptr;
  if(addr>heapend)return;
  struct page_t* header=(struct page_t*)(addr-addr%16384);//考虑位操作优化
  addr=(addr%16384);
  if(header->type==2048){//2048 4096 6144
    int i=addr/2048;
    header->now--;
    header->map[i-1]=false;
    return;
  }
  else if(header->type==4096){
  int i=addr/4096;
    header->now--;
    header->map[i-1]=false;
    return;
  }
  int i=(addr-1024)/header->type;
  header->map[i]=false;
  header->now--;
  return;
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
