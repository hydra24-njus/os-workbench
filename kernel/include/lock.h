typedef struct pmm_spinlock{
  int locked;
}pmm_spinlock_t;
void spinlock_init(pmm_spinlock_t *lk);
void lock(pmm_spinlock_t *lk);
void unlock(pmm_spinlock_t *lk);