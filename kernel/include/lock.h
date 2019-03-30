typedef struct __lock_t{
	int flag;
} lock_t;

void lock_init(lock_t *lock);
void lock(lock_t *lock);
void unlock(lock_t *lock);
