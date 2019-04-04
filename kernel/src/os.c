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

#define KALLOC_BLOCK 1024
void test_big_small(){
  int *p = NULL;
  int *p_old = NULL;
  size_t size[] = {
    KALLOC_BLOCK/sizeof(int)/10*3,
    KALLOC_BLOCK/sizeof(int)/10*1
  };
  int num = sizeof(size)/sizeof(size[0]);
  for(int i=0;;i++){
    if((p = pmm->alloc(size[i%num]*sizeof(int))) == NULL)
      break;
    //printf("test_big_small: start: %d\n", (uintptr_t)p);
    printf("\33[1;35mtest_big_small: I'm at %d, %d, %d\n\33[0m", (uintptr_t)p, i, _cpu()+1);
    for(int j=0;j < size[i%num];j++){
      p[j] = j;
    }
    
    //printf("test_big_small: %d %d\n", (uintptr_t)p, (uintptr_t)p_old);

    if(p_old != NULL){
      for(int j=0;j < size[(i-1)%num];j++){
        //assert(p_old[j] == j);
        if(p_old[j] != j){
        	LOG("assertion fail");
        	return;
        }
      }
    }
    
    if(p_old != NULL)
      pmm->free(p_old);
    p_old = p;
    /*
    if(i >= 10)
      break;
    */
  }
}

void test_full(){
  //init_lock(&test_lock, 'b');
  //printf("test_full: lock %p\n", &test_lock);
  int *p = NULL;
  int *p_old = NULL;
  int term = 0;
  while((p = pmm->alloc(1000*sizeof(int)))){
    //spin_lock(&test_lock);
    //if((uintptr_t)p % 500 == 0)
    printf("\33[1;35m malloc address:%x from cpu %d\n\33[0m", (uintptr_t)p,_cpu()+1);
    //Assert(test_lock.slock == 1, "test_full: test_lock.slock值为0");
    //spin_unlock(&test_lock);
    for(int i=0;i < 1000;i++){
      //printf("test_full: I'm at %p, %d\n", p, i);
      p[i] = i;
    }
    if(p_old != NULL){
      for(int i=0;i < 1000;i++){
        assert(p_old[i] == i);
        /*if(p_old[i] != i){
        	LOG("assertion fail");
        	return;
        }*/
      }
    }
    if(p_old != NULL){
      pmm->free(p_old);
      printf("free %x\n",(uintptr_t)p);
    }
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
  //test_big_small();
  test_full();
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
