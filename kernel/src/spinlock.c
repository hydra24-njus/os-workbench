#include <os.h>
#include <lock.h>
//以下内容参考xv6实现
void spinlock_init(pmm_spinlock_t *lk){
  lk->locked=0;
}
void lock(pmm_spinlock_t *lk){
  while(atomic_xchg(&(lk->locked),1)!=0);
}
void unlock(pmm_spinlock_t *lk){
  atomic_xchg(&(lk->locked), 0);
}