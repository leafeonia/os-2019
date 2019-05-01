#include <common.h>
#include <klib.h>
#include <devices.h>
#include <common.h>
#include "my_os.h"

#define NR_TASK 17
static task_t* tasks[NR_TASK];
static int task_id = 0;

static void kmt_init(){
	/*for(int i = 0;i < NR_TASK;i++){
		task_t* task = &tasks[i];
		_Area stack = (_Area){task->stack,task->fence2};
		task->context = *_kcontext(stack, ) 
	}*/
}
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){


	//LOCKKKKKKKKKKKKKKKKKK
	LOG("enter kmt_create");
	tasks[task_id++] = task;
	_Area stack = (_Area){task->stack, task->fence2};
	task->context = *_kcontext(stack, entry, arg);
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
