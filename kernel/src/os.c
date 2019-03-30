#include <common.h>
#include <klib.h>
#include "my_os.h"

#define NR_TEST 100
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

static void* test_array[NR_CPU][NR_TEST];
static void alloc_test(){
	lock_t mylock;
	lock_init(&mylock);
	lock(&mylock);
	printf("test begins~\n");
	unlock(&mylock);
	int i;
	for (i = 0;i < NR_TEST;i++){
		test_array[_cpu()][i] = pmm->alloc(0x100);
		printf("%x\n",test_array[_cpu()][i]);
	}
	for (i = 0;i < NR_TEST;i++){
		pmm->free(test_array[_cpu()][i]);
	}
	printf("success~\n");
} 

static void os_run() {
  hello();
  alloc_test();
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
