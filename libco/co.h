#ifndef __CO_H__
#define __CO_H__

#include <ucontext.h>

typedef void (*func_t)(void *arg);
#define STACK_SIZE 1024*128
#define NR_THREAD 16

typedef struct _co {
    ucontext_t context;
    func_t func;
    void *arg;
    int state;
    int id;
    unsigned char stack[STACK_SIZE];
}co;

typedef struct _scheduler{
    ucontext_t main;
    int max_length;
    co* threads;
    int current;
    int valid;
}scheduler;

void co_init();
co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(co *thd);


enum ThreadState{FREE,RUNNING};

#endif
