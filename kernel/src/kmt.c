#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

#define NR_TASK 17

static task_t* tasks[NR_TASK];
static int task_id = 0;
static task_t **current;
static int cpu_ncli[16];
static spinlock_t lk_kmt_create;
static spinlock_t lk_kmt_save;
static spinlock_t lk_kmt_switch;


static void pushcli(){
	_intr_write(0);  //cli
	cpu_ncli[_cpu()] += 1;
}

static void popcli(){
	if(--cpu_ncli[_cpu()] < 0)
    	panic("popcli");
	if(!cpu_ncli[_cpu()]) _intr_write(1);  //sti
}

static int holding(spinlock_t* lk){
	int r;
  	pushcli();
  	r = lk->locked && lk->cpu == _cpu();
  	popcli();
  	return r;
}

static void kmt_spin_init(spinlock_t *lk, const char *name){
	lk->name = name;
	lk->cpu = 0;
	lk->locked = 0;
}

static void kmt_spin_lock(spinlock_t *lk){
	pushcli();
	if(holding(lk)){
		printf("murderer: %s\n",lk->name);
		panic("acquire");
	}
    	
	while(_atomic_xchg(&lk->locked,1));
	lk->cpu = _cpu();
}

static void kmt_spin_unlock(spinlock_t *lk){
	if(!holding(lk))
    	panic("release");
    lk->cpu = 0;
    _atomic_xchg(&lk->locked,0);
	popcli();
}
static void kmt_sem_init(sem_t *sem, const char *name, int value){

}
static void kmt_sem_wait(sem_t *sem){

}
static void kmt_sem_signal(sem_t *sem){

}



/*
	current-----
			   |
		  	   v
		  	   (cur_deref)
    0x114740    0x114744           (task_t**)
	============================
	| tasks[0] | tasks[1] | ...    (tasks_t*)
	============================
		|			|			
		v			v	
	0x200008    0x201098           (task_t*)
	(&dummy1)	(&dummy2)
*/

static _Context* kmt_context_save(_Event ev, _Context *ctx){
	kmt_spin_lock(&lk_kmt_save);
	if(*current) assert((*current)->fence1 == MAGIC1 && (*current)->fence2 == MAGIC2);
	//printf("BEFORE:ctx->eip = 0x%x, *current = 0x%x, &tasks[0] = 0x%x, tasks[0]->context.eip = 0x%x, &tasks[1] = 0x%x, tasks[1]->context.eip = 0x%x\n\n",ctx->eip, *current, &tasks[0], tasks[0]->context.eip, &tasks[1], tasks[1]->context.eip);
	//LOG("enter kmt_context_save");
	if(*current) {
		//printf("*current = 0x%x\n",*current);
		//(*current)->context = *ctx;
	}
	//printf("AFTER :ctx->eip = 0x%x, *current = 0x%x, &tasks[0] = 0x%x, tasks[0]->context.eip = 0x%x, &tasks[1] = 0x%x, tasks[1]->context.eip = 0x%x\n\n\n",ctx->eip, *current, &tasks[0], tasks[0]->context.eip, &tasks[1], tasks[1]->context.eip);
	kmt_spin_unlock(&lk_kmt_save);
	return NULL;
}

static _Context* kmt_context_switch(_Event ev, _Context *ctx){
	kmt_spin_lock(&lk_kmt_switch);
	printf("intr_read = %d\n",_intr_read());
	kmt_spin_unlock(&lk_kmt_switch);
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
	//printf("current = 0x%x, current + 1 =0x%x\n",current, current+1);
	if(current + 1 == &tasks[task_id]){
		//LOG("checkpoint 2");
		current = &tasks[0];
	}
	else
		current++;
	//printf("current = 0x%x, task_name: %s\n",current, (*current)->name);
	_Context* ret = &(*current)->context;
	
	return ret;
}

static void kmt_init(){
	//LOG("kmt_init");
	//printf("tasks[0] = 0x%x, &tasks[0] = 0x%x, tasks[1] = 0x%x, &tasks[1] = 0x%x\n", tasks[0], &tasks[0], tasks[1], &tasks[1]);
	memset(cpu_ncli,0,sizeof(cpu_ncli));
	current = tasks;
	os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
	os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
	LOG("kmt_init");
	kmt_spin_init(&lk_kmt_create,"kmt_create");
	kmt_spin_init(&lk_kmt_save,"kmt_save");
	kmt_spin_init(&lk_kmt_switch,"kmt_switch");
	/*for(int i = 0;i < NR_TASK;i++){
		task_t* task = &tasks[i];
		_Area stack = (_Area){task->stack,task->fence2};
		task->context = *_kcontext(stack, ) 
	}*/
}
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){


	//LOCKKKKKKKKKKKKKKKKKK
	kmt_spin_lock(&lk_kmt_create);
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
	kmt_spin_unlock(&lk_kmt_create);
	return 0;
}
static void kmt_teardown(task_t *task){

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
