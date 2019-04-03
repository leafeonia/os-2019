#include <common.h>
#include <klib.h>
#include "my_os.h"

#define NR_TEST 0x40
#define NR_CPU 4
static void os_init() {
  pmm->init();
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]); _putc('\n');
}



void test_full(){
  //init_lock(&test_lock, 'b');
  //printf("test_full: lock %p\n", &test_lock);
  int *p = NULL;
  int *p_old = NULL;
  int term = 0;
  while((p = pmm->alloc(1000*sizeof(int)))){
    //spin_lock(&test_lock);
    printf("\33[1;35mtest_full: I'm at %#x, %d\n\33[0m", (uintptr_t)p,_cpu());
    //Assert(test_lock.slock == 1, "test_full: test_lock.slock值为0");
    //spin_unlock(&test_lock);
    for(int i=0;i < 1000;i++){
      //printf("test_full: I'm at %p, %d\n", p, i);
      p[i] = i;
    }
    if(p_old != NULL){
      for(int i=0;i < 1000;i++){
        assert(p_old[i] == i);
      }
    }
    if(p_old != NULL)
      pmm->free(p_old);
    p_old = p;
    term++;
    /*
    if(term >= 5)
      break;
      */
  }
}


/*

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
  //test_full();
  LOG("FA");
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
