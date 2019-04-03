#include <common.h>
#include <klib.h>
#include <my_os.h>
#include <assert.h>

#define NAIVE

static uintptr_t pm_start, pm_end;
static lock_t mylock;

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start; 
  pm_end   = (uintptr_t)_heap.end;
  lock_init(&mylock);
  //printf("start,end = %x %x\n",pm_start,pm_end);
}

static void *kalloc(size_t size) {
#ifdef NAIVE
  lock(&mylock);
  void *ret = (void*)pm_start;
  pm_start += size;  
  //printf("%x from cpu#%d\n",ret,_cpu()+1);
  unlock(&mylock);
  return ret;
#else

#endif
  return NULL;
}

static void kfree(void *ptr) {
#ifdef NAIVE

#else

#endif
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};


