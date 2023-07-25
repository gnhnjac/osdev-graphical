
typedef unsigned int lock_t;

#define ATOMIC_LOCK_INIT 0

extern void acquireLock(lock_t *lock);
extern void releaseLock(lock_t *lock);