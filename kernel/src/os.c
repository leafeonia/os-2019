#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

#define L2_TEST
#define NR_IRQ 20

struct irq{
	int event, seq;
	handler_t handler;
} irqs[NR_IRQ];

static int irq_id = 0;
static spinlock_t lk_trap;
extern int holding(spinlock_t* lk);

void echo_task(void *name){
	device_t *tty = dev_lookup(name);
	while(1){
		//printf("echo %s\n",name);
		char line[128], text[128];
		sprintf(text, "(%s) $ ",name);
		//printf("strlen = %d\n",strlen(name));
		tty->ops->write(tty, 0, text, strlen(text));
		int nread = tty->ops->read(tty, 0, line, sizeof(line));
		//printf("checkpoint %s\n",name);
		line[nread - 1] = '\0';
		sprintf(text, "Echo %s.\n", line);
		tty->ops->write(tty, 0, text, strlen(text));
		//printf("finish %s\n",name);
	}
}



void dummy_test(void* arg){
	int i = (intptr_t)arg;
	/*if(i == 2){
		LOG("help me");
		return;
	}*/
	int num = 0;
	while(1) {
		if(i != 2)
			printf("FA%d\n",i);
		else
			printf("%d ",num++);
		for(volatile int j = 0;j < 100000;j++);
	}
	//printf("FA\n");
}

static void os_init() {
  LOG("os_init");
  //printf("begin of os_init. intr_read = %d\n",_intr_read());
  pmm->init();
  kmt->init(); 
  kmt->spin_init(&lk_trap,"lk_trap");
  //_vme_init(pmm->alloc,pmm->free);
  dev->init();
  #ifdef L2_TEST
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
  //printf("checkpoint 1 of os_init. intr_read = %d\n",_intr_read());
  /*kmt->create(pmm->alloc(sizeof(task_t)), "dummy1", dummy_test, (void*)1);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy2", dummy_test, (void*)2);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy3", dummy_test, (void*)3);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy4", dummy_test, (void*)4);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy5", dummy_test, (void*)5);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy6", dummy_test, (void*)6);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy7", dummy_test, (void*)7);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy8", dummy_test, (void*)8);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy9", dummy_test, (void*)9);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy10", dummy_test, (void*)10);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy11", dummy_test, (void*)11);*/
  #endif
  //printf("end of os_init. intr_read = %d\n",_intr_read());
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]); _putc('\n');
}


void alloc_test(){
  int *p = NULL;
  int *p_old = NULL;
  while((p = pmm->alloc(1000*sizeof(int)))){
    //if((uintptr_t)p % 500 == 0)
    //printf("\33[1;35m malloc address:%x from cpu %d\n\33[0m", (uintptr_t)p,_cpu()+1);
    for(int i = 0;i < 1000;i++){
      p[i] = i;
    }
    if(p_old != NULL){
      for(int i = 0;i < 1000;i++){
        assert(p_old[i] == i);
        /*if(p_old[i] != i){
        	LOG("assertion fail");
        	return;
        }*/
      }
    }
    if(p_old != NULL){
      pmm->free(p_old);
      //printf("free %x\n",(uintptr_t)p);
    }
    p_old = p;
  }
}


/*
#define NR_TEST 0x40
#define NR_CPU 4
static void alloc_test(){
	printf("test begins~\n");
	int i;
	for (i = 0;i < NR_TEST;i++){
		test_array[_cpu()][i] = pmm->alloc(0x100);
		
	}
	for (i = 0;i < NR_TEST;i++){
		pmm->free(test_array[_cpu()][i]);
	}
	printf("success~\n");
} */

static void os_run() {
  hello();
  //alloc_test();
  _intr_write(1);
  while (1) {
    _yield();
  }
}


task_t **current_t[16];
handler_t input_notify;
extern int task_id;
extern task_t* taskss[21];

static _Context *os_trap(_Event ev, _Context *context) {
  if (task_id <= _cpu()) {
    return context;
  }
  if (current_t[_cpu()] && *current_t[_cpu()]) {
    (*current_t[_cpu()])->context = *context;
  }

  if (ev.event == _EVENT_IRQ_IODEV) {
    assert(input_notify);
    input_notify(ev, context);
  }

  do {
    if (!current_t[_cpu()] || current_t[_cpu()] + 1 == taskss + task_id) {
      current_t[_cpu()] = &taskss[0];
    } else {
      current_t[_cpu()]++;
    }
  } while (current_t[_cpu()] == NULL || (current_t[_cpu()] - taskss) % _ncpu() != _cpu());

  return &((*current_t[_cpu()])->context);
 
 
 /* //printf("os_trap: event = %d\n",ev.event);
  if(ev.event == _EVENT_IRQ_TIMER) return context;
  if(!holding(&lk_trap)) kmt->spin_lock(&lk_trap);
  else LOG("???????");
  _Context* ret = context;
  //if(ev.event != 5)printf("ev = %d\n",ev.event);
  
  //printf("context->eip = 0x%x\n",context->eip);
  for(int i = 0;i < irq_id;i++){
  	if(irqs[i].event == _EVENT_NULL || irqs[i].event == ev.event){
  		//if(irqs[i].event != _EVENT_NULL) LOG("BOOM");
  		_Context *next = irqs[i].handler(ev,context);
  		if(next) ret = next;
  	}
  }
  kmt->spin_unlock(&lk_trap);
  
  //return context;
  //printf("os_trap returns task with context address: 0x%x\n",ret);
  
  return ret;*/
}


static void os_on_irq(int seq, int event, handler_t handler) {
  if (event == _EVENT_IRQ_IODEV) {
    input_notify = handler;
  }
	/*irqs[irq_id].seq = seq;
	irqs[irq_id].event = event;
	irqs[irq_id++].handler = handler;
	for(int i = 0;i < irq_id;i++){
		for(int j = i + 1;j < irq_id;j++){
			if(irqs[i].seq > irqs[j].seq){
				struct irq temp = irqs[i];
				irqs[i] = irqs[j];
				irqs[j] = temp;
			}
		}
	}*/
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
