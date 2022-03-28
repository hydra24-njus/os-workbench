#include <common.h>
uintptr_t heaptr;
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
void _init(){
  spinlock_init(&biglock);
  heaptr=(uintptr_t)heap.start;
  for(int i=0;i<8;i++){
    buddy[i].p32=buddy[i].p64=buddy[i].p128=buddy[i].p256=buddy[i].p512=buddy[i].p1024=buddy[i].p2048=buddy[i].p4096=NULL;
  }
}
void* sbrk(int size){
  uintptr_t tmp=heaptr;
  heaptr+=size;
  if(heaptr>(uintptr_t)heap.end)return NULL;
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
  lock(&biglock);
  tmp=sbrk(8192);
  unlock(&biglock);
  return tmp;
}

static void *kalloc(size_t size) {
  uintptr_t addr=0;
  size=power2(size);
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
  if (ptr->next == NULL){ //该cpu没有页
    ptr = new_page();
    ptr->prev=NULL;ptr->next=NULL;
    ptr->now=0;ptr->max=7168/size;
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
  }
  else{
    while(ptr!=NULL){
      if(ptr->now<ptr->max)break;
      ptr=ptr->next;
    }
  }
  if(ptr->now>=ptr->max){//没有空闲页
    struct page_t* tmp=ptr;
    ptr = new_page();
    ptr->prev=tmp;tmp->next=ptr;ptr->next=NULL;
    ptr->now=0;ptr->max=7168/size;
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
  }
  for(int i=0;i<ptr->max;i++){
    int x=i/64;
    uint64_t y=1<<(i%64);
    if(((ptr->map[x])&y)==0){//找到页中空闲位置，计算地址
      ptr->map[x]|=y;
      ptr->now++;
      addr=(uintptr_t)ptr+1024+ptr->type*i;
    }
  }
  printf("addr=%x\n",addr);
  return (void*)addr;
}

static void kfree(void *ptr) {

}

static void pmm_init() {
  _init();
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
