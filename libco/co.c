#include <stdio.h>
#include "co.h"
#include <ucontext.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>


scheduler s;

void co_init() {
    srand((unsigned)time(NULL));
    s.current = -1;
    s.max_length = 0;
    s.valid = 0;
    s.threads = (co*)malloc(sizeof(co)*NR_THREAD);
    getcontext(&(s.main));
    int i;
    for (i = 0; i < NR_THREAD; ++i) {
        s.threads[i].state = FREE;
    }
}

void transfer(){
//    printf("transfer\n");
    if(s.current != -1){
        int id = s.current;
        co* thread = &s.threads[id];
        thread->func(thread->arg);
        s.threads[id].state = FREE;
        s.current = -1;
        s.valid--;
    }
}


struct co* co_start(const char *name, func_t func, void *arg) {
    int id = 0;
    for (id = 0; id < s.max_length; ++id) {
        if(s.threads[id].state == FREE) break;
    }
    if (id == s.max_length) s.max_length++;
    if (s.max_length >= NR_THREAD){
        printf("The number of threads is more than the design load. Please modify NR_THREAD(in co.h)\n");
        exit(1);
    }
    co* t = &(s.threads[id]);
    t->state = RUNNING;
    t->func = func;
    t->arg = arg;
    t->id = id;
    getcontext(&(t->context));
    t->context.uc_link = &(s.main);
    t->context.uc_stack.ss_size = STACK_SIZE;
    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_flags = 0;
    s.current = id;
    s.valid++;

    makecontext(&(t->context),(void(*)(void))(transfer),0);
    swapcontext(&(s.main),&(t->context));
    //func(arg); // Test #2 hangs
    return &s.threads[id];
}

void co_yield() {
    int id = rand()%(s.valid+1);
    if(id == s.valid) {
        int before = s.current;
        assert(before != -1);
        s.current = -1;
        swapcontext(&(s.threads[before].context),&(s.main));
    }
    else{
        int i;
        int cnt = 0;
        for (i = 0; i < s.max_length; ++i) {
            if(s.threads[i].state == RUNNING){
                if(cnt == id){
                    int before = s.current;
                    s.current = i;
                    if(before != -1)swapcontext(&(s.threads[before].context),&(s.threads[i].context));
                    else swapcontext(&(s.main),&(s.threads[i].context));
                }
                cnt++;
            }
        }
    }
}

void co_wait(co *thd) {
    while(thd->state != FREE){
//        setcontext(&(thd->context));
        s.current = thd->id;
        if(swapcontext(&(s.main),&(thd->context)) == -1){
            printf("error\n");
            exit(1);
        }
    }
}


