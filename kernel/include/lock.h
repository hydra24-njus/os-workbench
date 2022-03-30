//以下内容参考xv6实现
typedef struct spinlock{
  int locked;
}spinlock_t;
void spinlock_init(spinlock_t *lk);
void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);