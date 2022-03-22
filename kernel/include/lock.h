
//以下内容参考xv6实现
struct spinlock_t{
  int locked;
};
static inline int xchg(volatile int *addr,int newval){
  int result;
  asm volatile("lock;xchgl %0,%1":
  	       "+m"(*addr),"=a"(result):
  	       "1"(newval):
  	       "cc");
  return result;
}
void lock(struct spinlock_t *lk){
  while(xchg(&lk->locked,1)!=0);
}
void unlock(struct spinlock_t *lk){
  asm volatile("movl $0,%0":"+m"(lk->locked):);
}
