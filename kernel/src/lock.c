#include "lock.h"
#include "am.h"

void lock_init(lock_t *lock){
	lock->flag = 0;
}

void lock(lock_t *lock){
	//_intr_write(0);  //cli
	while(_atomic_xchg(&lock->flag,1));
}

void unlock(lock_t *lock){
	_atomic_xchg(&lock->flag,0);
	//_intr_write(1); //sti
}
