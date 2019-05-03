#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

#define NR_TASK 17

static task_t* tasks[NR_TASK];
static int task_id = 0;
static task_t **current;

/*
	current-----
			   |
		  	   v
		  	   (cur_deref)
	============================
	| tasks[0] | tasks[1] | ...  
	============================
		|			|			
		v			v	
	0x200008    0x201098
	(&dummy1)	(&dummy2)
*/

static _Context* kmt_context_save(_Event ev, _Context *ctx){
	if(*current) assert((*current)->fence1 == MAGIC1 && (*current)->fence2 == MAGIC2);
	printf("BEFORE:ctx = 0x%x, *current = 0x%x, &tasks[0] = 0x%x, &tasks[0]->context = 0x%x, &tasks[1] = 0x%x, &tasks[1]->context = 0x%x\n",ctx, *current, &tasks[0], &tasks[0]->context, &tasks[1], &tasks[1]->context);
	//LOG("enter kmt_context_save");
	if(*current) {
		//printf("*current = 0x%x\n",*current);
		(*current)->context = *ctx;
	}
	printf("AFTER: *current = 0x%x, &tasks[0] = 0x%x, &tasks[0]->context = 0x%x, &tasks[1] = 0x%x, &tasks[1]->context = 0x%x\n\n",*current, &tasks[0], &tasks[0]->context, &tasks[1], &tasks[1]->context);
	return NULL;
}

static _Context* kmt_context_switch(_Event ev, _Context *ctx){
	if(*current) assert((*current)->fence1 == MAGIC1 && (*current)->fence2 == MAGIC2);
	//LOG("kmt_context_switch");
	//printf("ctx = 0x%x\n",ctx);
	
	task_t* cur_deref = *current;
	if(!cur_deref) return NULL;
	
	//printf("tasks[0] = 0x%x, &tasks[0] = 0x%x, tasks[1] = 0x%x, &tasks[1] = 0x%x\n", tasks[0], &tasks[0], tasks[1], &tasks[1]);
	
	//LOG("checkpoint0");
	//printf("task_id = %d,cur_deref = 0x%x\n",task_id,cur_deref);
	//cur_deref->context = *ctx;
	//LOG("checkpoint 1");
	//printf("cur_deref + 1 = 0x%x, ")
	if(current + 1 == &tasks[task_id]){
		//LOG("checkpoint 2");
		current = &tasks[0];
	}
	else
		current++;
	//printf("current = 0x%x, task_name: %s\n",current, (*current)->name);
	return &(*current)->context;
}

static void kmt_init(){
	//LOG("kmt_init");
	//printf("tasks[0] = 0x%x, &tasks[0] = 0x%x, tasks[1] = 0x%x, &tasks[1] = 0x%x\n", tasks[0], &tasks[0], tasks[1], &tasks[1]);
	current = tasks;
	os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
	os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
	/*for(int i = 0;i < NR_TASK;i++){
		task_t* task = &tasks[i];
		_Area stack = (_Area){task->stack,task->fence2};
		task->context = *_kcontext(stack, ) 
	}*/
}
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){


	//LOCKKKKKKKKKKKKKKKKKK
	
	tasks[task_id] = task;
	_Area stack = (_Area){task->stack, &(task->fence2)};
	task->context = *_kcontext(stack, entry, arg);
	task->name = name;
	task->fence1 = MAGIC1;
	task->fence2 = MAGIC2;
	
	printf("kmt_create: A task has been created. Position: 0x%x, Name: %s, func_entry: 0x%x\n",task, name, entry);
	printf("tasks[0] = 0x%x, &tasks[0] = 0x%x, tasks[1] = 0x%x, &tasks[1] = 0x%x\n", tasks[0], &tasks[0], tasks[1], &tasks[1]);
	current = &tasks[task_id++];
	//printf("*current = 0x%x\n",*current);
	//printf("current->context.eip = 0x%x\n",current->context.eip);
	//printf("func_entry = 0x%x\n",entry);
	return 0;
}
static void kmt_teardown(task_t *task){

}
static void kmt_spin_init(spinlock_t *lk, const char *name){

}
static void kmt_spin_lock(spinlock_t *lk){

}
static void kmt_spin_unlock(spinlock_t *lk){

}
static void kmt_sem_init(sem_t *sem, const char *name, int value){

}
static void kmt_sem_wait(sem_t *sem){

}
static void kmt_sem_signal(sem_t *sem){

}

MODULE_DEF(kmt) {
  .init   = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
  .spin_init = kmt_spin_init,
  .spin_lock = kmt_spin_lock,
  .spin_unlock = kmt_spin_unlock,
  .sem_init = kmt_sem_init,
  .sem_wait = kmt_sem_wait,
  .sem_signal = kmt_sem_signal,
};
