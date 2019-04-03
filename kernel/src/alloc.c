#include <common.h>
#include <klib.h>
#include <my_os.h>

//#define NAIVE

typedef long long ALIGN;

typedef union header{
	struct{
		union header *next;
		size_t size;
	}s;
	ALIGN x;
} HEADER;

static uintptr_t pm_start, pm_end;
static lock_t mem_lock;
HEADER base;
HEADER* freep = NULL;

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start; 
  pm_end   = (uintptr_t)_heap.end;
  lock_init(&mem_lock);
}

static void* fancy_alloc(size_t nbytes){
	HEADER *p, *prevp;
	size_t nunits = (nbytes + sizeof (HEADER) - 1) / sizeof (HEADER) + 1;
	if(freep == NULL){
		base.s.next = freep = prevp = &base;
		base.s.size = 0;
	}
}




static void *kalloc(size_t size) {
  vpid* ret;
#ifdef NAIVE
  lock(&mem_lock);
  ret = (void*)pm_start;
  pm_start += size;  
  //printf("%x from cpu#%d\n",ret,_cpu()+1);
  unlock(&mem_lock);
  return ret;
#else
  lock(&mem_lock);
  ret = fancy_alloc(size);
  unlock(&mem_lock);
#endif
  return ret;
}

static void kfree(void *ptr) {
#ifdef NAIVE

#else
  lock(&mem_lock);
  free(ptr);
  unlock(&mem_lock);
#endif
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};


