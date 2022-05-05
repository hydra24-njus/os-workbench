typedef struct pmm_spinlock{
    int locked;
}pmm_spinlock_t;
void pmm_spinlock_init(pmm_spinlock_t *lk);
void pmm_lock(pmm_spinlock_t *lk);
void pmm_unlock(pmm_spinlock_t *lk);
