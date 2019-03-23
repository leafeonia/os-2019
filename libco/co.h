#ifndef __CO_H__
#define __CO_H__

typedef void (*func_t)(void *arg);
struct co;

void co_init();
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);

#define STACK_SIZE 1024*128
#define MAX_THREAD 16

#endif
