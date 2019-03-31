#include "lock.h"
#include "am.h"

void lock_init(lock_t *lock){
	lock->flag = 0;
}

void lock(lock_t *lock){
	intr_write(0);
	while(_atomic_xchg(&lock->flag,1));
}

void unlock(lock_t *lock){
	lock->flag = 0;
	intr_write(1);
}
