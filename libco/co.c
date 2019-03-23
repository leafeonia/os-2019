#include <stdio.h>
#include "co.h"
#include <ucontext.h>
#include <string.h>
#include <stdlib.h>


struct co {
	ucontext_t context;
	func_t func;
	void *arg;
	unsigned char stack[STACK_SIZE];
};

struct co threads[MAX_THREAD];
int thread_num = 0;

void co_init() {
	//memset(thread_state,0,sizeof(thread_state));
}

struct co* co_start(const char *name, func_t func, void *arg) {
  ucontext_t new,cur;
  //unsigned char* stack = (unsigned char*)malloc(sizeof(unsigned char)*STACK_SIZE);
  int index = thread_num;
  
  getcontext(&new);
  new.uc_stack.ss_sp = threads[index].stack;
  new.uc_stack.ss_size = sizeof(threads[index].stack);
  new.uc_stack.ss_flags = 0;
  new.uc_link = &cur;
  
  threads[index].context = new;
  threads[index].func = func;
  threads[index].arg = arg;

  thread_num++;
  
  makecontext(&new,(void(*)(void))func,1);
  
  func(arg); // Test #2 hangs
  return &threads[index];
}

void co_yield() {
	setcontext(&(threads[rand()%thread_num].context));
}

void co_wait(struct co *thd) {
}

