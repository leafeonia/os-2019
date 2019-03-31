#include <common.h>
#include <klib.h>
#include <pthread.h>
#include <my_os.h>

#define NAIVE

static uintptr_t pm_start, pm_end;
//static pthread_mutex_t big_lock;

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start; 
  pm_end   = (uintptr_t)_heap.end;
  //assert(pthread_mutex_init(&big_lock,NULL));
}

static void *kalloc(size_t size) {
#ifdef NAIVE
  lock_t mylock;
  lock_init(&mylock);
  lock(&mylock);
  void *ret = (void*)pm_start;
  pm_start += size;  
  printf("%x from cpu#%d\n",ret,_cpu()+1);
  //unlock(&mylock);
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


