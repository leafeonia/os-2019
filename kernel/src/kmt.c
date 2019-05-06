#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

#define NR_TASK 21

static task_t* tasks[16][NR_TASK];
static int task_id = 0;
//static task_t **current;
task_t **current_task[16];
#define current (current_task[_cpu()])
static int cpu_start[16];
static int cpu_ncli[16];
static int cpu_intena[16];
static spinlock_t lk_kmt_create;
static spinlock_t lk_kmt_save;
static spinlock_t lk_kmt_switch;


static void pushcli(){
	_intr_write(0);  //cli
	if(cpu_ncli[_cpu()] == 0)
    	cpu_intena[_cpu()] = _intr_read();
	cpu_ncli[_cpu()] += 1;
}

static void popcli(){
	if(_intr_read())
		panic("popcli - interruptible");
	if(--cpu_ncli[_cpu()] < 0)
    	panic("popcli");
	if(!cpu_ncli[_cpu()] && cpu_intena[_cpu()]) _intr_write(1);  //sti
}

static int holding(spinlock_t* lk){
	int r;
  	pushcli();
  	r = lk->locked && lk->cpu == _cpu();
  	popcli();
  	return r;
}

void kmt_spin_init(spinlock_t *lk, const char *name){
	lk->name = name;
	lk->cpu = 0;
	lk->locked = 0;
}

void kmt_spin_lock(spinlock_t *lk){
	pushcli();
	if(holding(lk)){
		printf("murderer: %s\n",lk->name);
		panic("acquire");
	}
    	
	while(_atomic_xchg(&lk->locked,1));
	lk->cpu = _cpu();
}

void kmt_spin_unlock(spinlock_t *lk){
	if(!holding(lk))
    	panic("release");
    lk->cpu = 0;
    _atomic_xchg(&lk->locked,0);
	popcli();
}
static void kmt_sem_init(sem_t *sem, const char *name, int value){
	sem->value = value;
	sem->name = name;
	kmt_spin_init(&sem->lock,name);
}
static void kmt_sem_wait(sem_t *sem){
	
	kmt_spin_lock(&sem->lock);
	//printf("enter kmt_sem_wait, sem->name = %s\n",sem->name);
	while(1){
		
		if(sem->value > 0){
			
			sem->value--;
			//printf("wait: %s: value = %d\n",sem->name,sem->value);
			//LOG("leave kmt_sem_wait");
			kmt_spin_unlock(&sem->lock);
			return;
		}
		//printf("wait and yield: %s: value = %d\n",sem->name,sem->value);
		kmt_spin_unlock(&sem->lock);
		_yield();
		kmt_spin_lock(&sem->lock);
	}
}
static void kmt_sem_signal(sem_t *sem){
	
	kmt_spin_lock(&sem->lock);
	//printf("enter kmt_sem_signal, sem->name = %s\n", sem->name);
	sem->value++;
	//printf("signal: %s: value = %d\n",sem->name,sem->value);
	//LOG("leave kmt_sem_signal");
	kmt_spin_unlock(&sem->lock);
}



/*  (chart for one core)
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
	//LOG("enter kmt_context_save");
	if(*current) {
		//printf("current = 0x%x\n",current);
		assert((*current)->fence1 == MAGIC1 && (*current)->fence2 == MAGIC2);
	}
	//printf("BEFORE:ctx->eip = 0x%x, *current = 0x%x, &tasks[0] = 0x%x, tasks[0]->context.eip = 0x%x, &tasks[1] = 0x%x, tasks[1]->context.eip = 0x%x\n\n",ctx->eip, *current, &tasks[0], tasks[0]->context.eip, &tasks[1], tasks[1]->context.eip);
	
	kmt_spin_lock(&lk_kmt_save);
	if(*current) {
		//printf("In kmt_save: *current = 0x%x, ctx = 0x%x\n",*current, ctx);
		if(!cpu_start[_cpu()]){
			//LOG("Fxxk off!");
			cpu_start[_cpu()] = 1;
			kmt_spin_unlock(&lk_kmt_save);
			return NULL;
		}
		(*current)->context = *ctx;
	}
	//printf("AFTER :ctx->eip = 0x%x, *current = 0x%x, &tasks[0] = 0x%x, tasks[0]->context.eip = 0x%x, &tasks[1] = 0x%x, tasks[1]->context.eip = 0x%x\n\n\n",ctx->eip, *current, &tasks[0], tasks[0]->context.eip, &tasks[1], tasks[1]->context.eip);
	kmt_spin_unlock(&lk_kmt_save);
	return NULL;
}

static _Context* kmt_context_switch(_Event ev, _Context *ctx){
	/*
	//LOG("kmt_context_switch");
	//printf("intr_read = %d\n",_intr_read());
	//printf("intr_read = %d\n",_intr_read());
	//printf("outside: intr_read = %d\n",_intr_read());
	
	if(!(*current)) return NULL;
	else assert((*current)->fence1 == MAGIC1 && (*current)->fence2 == MAGIC2);
	kmt_spin_lock(&lk_kmt_switch);
	//printf("ctx = 0x%x\n",ctx);
	
	
	//task_t* cur_deref = *current;
	//if(!cur_deref) return NULL;
	
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
	kmt_spin_unlock(&lk_kmt_switch);
	return ret;
	*/
	//LOG("kmt_context_switch");
	if(!(*current)) return NULL;
	else assert((*current)->fence1 == MAGIC1 && (*current)->fence2 == MAGIC2);
	kmt_spin_lock(&lk_kmt_switch);
	//printf("\n\n\nenter kmt_context_switch. _cpu() = %d, current = 0x%x, *current = 0x%x, current_task[%d] = 0x%x, intr_read() = %d\n",_cpu(),current,*current,_cpu(),current_task[_cpu()], _intr_read());
	//printf("task_id = %d, _ncpu() = %d, task_id / _ncpu() + 1 = %d, current + 1 = 0x%x, &tasks[_cpu()][task_id / _ncpu() + 1] = 0x%x, (*(current + 1) = 0x%x\n",task_id,_ncpu(),task_id / _ncpu() + 1, current + 1,&tasks[_cpu()][task_id / _ncpu() + 1],*(current + 1));
	//do{
		if(!(*current) || !(*(current + 1))/*  current + 1 == &tasks[_cpu()][task_id / _ncpu() + 1]*/){
			//LOG("go back now");
			current = &tasks[_cpu()][0];
		}
		else
			current++;
	//} while ((current - tasks[_cpu()]) % _ncpu() != _cpu());
	_Context* ret = &(*current)->context;
	
	//printf("current = 0x%x, *current = 0x%x, [cpu-%d] Schedule: %s\n\n\n",current, *current, _cpu(), (*current)->name);
	kmt_spin_unlock(&lk_kmt_switch);
	return ret;
	
}




static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){

	//printf("intr_read = %d\n",_intr_read());
	//LOCKKKKKKKKKKKKKKKKKK
	//kmt_spin_lock(&lk_kmt_create);
	//tasks[task_id] = task; //one core
	tasks[task_id % _ncpu()][task_id / _ncpu()] = task;
	_Area stack = (_Area){task->stack, &(task->fence2)};
	task->context = *_kcontext(stack, entry, arg);
	task->name = name;
	task->fence1 = MAGIC1;
	task->fence2 = MAGIC2;
	
	//printf("kmt_create: A task has been created. address: 0x%x, Name: %s, func_entry: 0x%x,task_id = %d, located at tasks[%d][%d](address: 0x%x)\n",task, name, entry,task_id, task_id % _ncpu(), task_id / _ncpu(), tasks[task_id % _ncpu()][task_id / _ncpu()]);
	//printf("tasks[0] = 0x%x, &tasks[0] = 0x%x, tasks[1] = 0x%x, &tasks[1] = 0x%x\n", tasks[0], &tasks[0], tasks[1], &tasks[1]);
	current_task[task_id % _ncpu()] = &tasks[task_id % _ncpu()][task_id / _ncpu()] ;
	task_id++;
	//printf("*current = 0x%x\n",*current);
	//printf("current->context.eip = 0x%x\n",current->context.eip);
	//printf("func_entry = 0x%x\n",entry);
	
	/*for(int i = 0;i < 4;i++){
		for(int j = 0;j < 2 ;j++){
			printf("tasks[%d][%d] = 0x%x\n",i,j,tasks[i][j]);
			printf("&tasks[%d][%d] = 0x%x\n",i,j,&tasks[i][j]);
		}
	}*/
	//kmt_spin_unlock(&lk_kmt_create);
	return 0;
}

void do_nothing(){}

static void kmt_init(){
	//LOG("kmt_init");
	//printf("tasks[0] = 0x%x, &tasks[0] = 0x%x, tasks[1] = 0x%x, &tasks[1] = 0x%x\n", tasks[0], &tasks[0], tasks[1], &tasks[1]);
	memset(cpu_ncli,0,sizeof(cpu_ncli));
	memset(cpu_start,0,sizeof(cpu_start));
	//current = tasks;
	for(int i = 0;i < _ncpu();i++){
		current_task[i] = &tasks[i][0];
	}
	//for(int i = 0;i < _ncpu();i++)
		//printf("current_task[%d] = 0x%x\n",i,current_task[i]);
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
	kmt_create(pmm->alloc(sizeof(task_t)), "idle1", do_nothing, NULL);
	kmt_create(pmm->alloc(sizeof(task_t)), "idle2", do_nothing, NULL);
	kmt_create(pmm->alloc(sizeof(task_t)), "idle3", do_nothing, NULL);
	kmt_create(pmm->alloc(sizeof(task_t)), "idle4", do_nothing, NULL);
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
