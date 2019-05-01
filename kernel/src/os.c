#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

#define L2_TEST


void echo_task(void *name){
	device_t *tty = dev_lookup(name);
	while(1){
		char line[128], text[128];
		sprintf(text, "(%s) $ ",name);
		tty_write(tty, 0, text, strlen(name));
		int nread = tty->ops->read(tty, 0, line, sizeof(line));
		line[nread - 1] = '\0';
		sprintf(text, "Echo %s.\n", line);
		tty_write(tty, 0, text, strlen(text));
	}
}

static void os_init() {
  pmm->init();
  LOG("os_init");
  #ifdef L2_TEST
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
  #endif
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

static _Context *os_trap(_Event ev, _Context *context) {
  return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
