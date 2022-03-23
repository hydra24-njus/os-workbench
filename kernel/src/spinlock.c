#include <common.h>
//以下内容参考xv6实现
static inline int xchg(volatile int *addr,int newval){
  int result;
  asm volatile("lock;xchgl %0,%1":
  	       "+m"(*addr),"=a"(result):
  	       "1"(newval):
  	       "cc");
  return result;
}
void spinlock_init(spinlock_t *lk){
  lk->locked=0;
}
void lock(spinlock_t *lk){
  while(xchg(&lk->locked,1)!=0);
}
void unlock(spinlock_t *lk){
  asm volatile("movl $0,%0":"+m"(lk->locked):);
}
