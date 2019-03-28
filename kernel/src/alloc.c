#include <common.h>
#include <klib.h>
#include <pthread.h>

#define NAIVE

static uintptr_t pm_start, pm_end;
static pthread_mutex_t big_lock;

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  assert(pthread_mutex_init(&bigLock,NULL));
}

static void *kalloc(size_t size) {
#ifdef NAIVE
  pthread_mutex_lock(&big_lock);
  void *ret = pm_start;
  pm_start += size;  
  pthread_mutex_unlock(&big_lock);
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


