#include <common.h>
#include <lock.h>
//以下内容参考xv6实现
void spinlock_init(spinlock_t *lk){
  lk->locked=0;
}
void lock(spinlock_t *lk){
  while(atomic_xchg(&(lk->locked),1)!=0);
}
void unlock(spinlock_t *lk){
  atomic_xchg(&(lk->locked), 0);
}