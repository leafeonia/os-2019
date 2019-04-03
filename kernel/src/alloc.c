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
	for (p = prevp->s.next;;prevp = p,p = p->s.next){
		if(p->s.size >= nunits){
			if(p->s.size == nunits){
				prevp->s.next = p->next;
			}
			//suppose size of p is 70,nunits = 50,p = a(an address)
			//after executing the following codes, prevp->s.next = a, a->s.size = 20
			//p = a+20(an address), p->s.size = 50, now it has nothing to do with the free linked list
			//return p+1 = a+21, as the available space.
			else{
				p->s.size -= nunits;
				p += nunits;
				p->s.size = nunits;
			}
			freep = prevp;
			
			//sizeof(void*) != sizeof(HEADER),so return (void*)p+1 is wrong
			return (void*)(p+1);
		}
		if (p == freep){
			return NULL;
		}
	}
}




static void *kalloc(size_t size) {
  void* ret;
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
  //free(ptr);
  unlock(&mem_lock);
#endif
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};


