#include <stdio.h>
#include "co.h"
#include <ucontext.h>
#include <string.h>
#include <stdlib.h>


struct co {
	ucontext_t context;
	func_t func;
	void *arg;
	char stack[STACK_SIZE];
};

struct co* thread_state[MAX_THREAD];
int thread_num = 0;

void co_init() {
	//memset(thread_state,0,sizeof(thread_state));
}

struct co* co_start(const char *name, func_t func, void *arg) {
  ucontext_t new,cur;
  char* stack[STACK_SIZE] = malloc(sizeof(char)*STACK_SIZE);
  
  getcontext(&new);
  new.uc_stack.ss_sp = stack;
  new.uc_stack.ss_size = sizeof(stack);
  new.uc_stack.ss_flags = 0;
  new.uc_link = &cur;
  
  struct co* ret;
  ret->context = new;
  ret->func = func;
  ret->arg = arg;
  ret->stack = stack;
  
  thread_state[thread_num++] = ret;
  
  makecontext(&new,(void(*)(void))func,arg);
  
  func(arg); // Test #2 hangs
  return &ret;
}

void co_yield() {
	setcontext(thread_state[rand()%thread_num]);
}

void co_wait(struct co *thd) {
}

