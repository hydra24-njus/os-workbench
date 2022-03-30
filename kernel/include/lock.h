typedef struct{
  int locked;
}spinlock_t;
void spinlock_init(spinlock_t *lk);
void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);