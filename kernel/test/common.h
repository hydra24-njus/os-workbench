#include"am.h"
#include "thread.h"

typedef struct spinlock{
  int locked;
}spinlock_t;
void spinlock_init(spinlock_t *lk);
void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);

#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug(...)
#endif
