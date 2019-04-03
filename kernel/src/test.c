#include <common.h>
#include <klib.h>
#include <time.h>
#include <debug.h>
#include <spinlock.h>

#define SSIZE (1 << 12)
#define MSIZE (1 << 30)
#define MAXN 110
#define TOFREE 0
#define TOALLOC 1

// spinlock loglk;
// spinlock testlk;

extern void print_freelist();
// void* mmp[MAXN] = {};
// bool allocated[MAXN] = {};

// void test_init() {
//   init_lock(&loglk, "loglk");
//   init_lock(&testlk, "testlk");
// }

// void test1(int n) {
//   /*小规模小内存顺序分配和回收*/
//   srand(uptime());
//   for(int i = 0; i < n; ++i) {
//     size_t size = rand() % SSIZE;
//     void* ptr = pmm->alloc(size);
//     // PLog("Test #%02d: Get pmm of size=0x%x at %p\n", i, size, ptr);
//     pmm->free(ptr);
//     // PLog("Test #%02d: Free pmm at %p\n", i, ptr);
//   }
//   acquire(&loglk);
//   // Assert(0, "Test1 PASS!");
//   // REDLog("CPU #%d", _cpu());
//   REDLog("Test1 PASS!");
//   // print_freelist();
//   release(&loglk);
//   // panic("Test1 PASS!");
// }

// void test2(int n) {
//   /*大规模内存随机分配和回收*/
//   // PLog("START Test2:");
//   // Assert(n < MAXN, "[Test2 error] too much memory allocated!");
//   srand(uptime());
//   int cnt = 0;
//   void* mmp[MAXN] = {};

//   for(int i = 0; i < n; ++i) {
//     int flag = rand() % 2;
//     acquire(&testlk);
//     if((flag == TOFREE && cnt > 0) || cnt == MAXN-10) { 
//       // PLog("Test #%d: Free pmm at %p\n", i, mmp[cnt]);
//       pmm->free(mmp[cnt]);
//       cnt --;
//     }
//     else {
//       size_t size = rand() %  MSIZE;
//       // PLog("Alloc Test size=0x%x", size);
//       void *ptr = pmm->alloc(size);
//       // PLog("Test #%d: Alloc pmm at %p, size=0x%x", i, ptr, size);
//       cnt ++;
//       mmp[cnt] = ptr;
//     }
//     release(&testlk);
//   }
//   while(cnt > 0){
//     // PLog("[Test2] Free pmm at %p\n", mmp[cnt]);
//     acquire(&testlk);
//     pmm->free(mmp[cnt]);
//     cnt--;
//     release(&testlk);
//   }
//   // acquire(&loglk);
//   // REDLog("CPU #%d", _cpu());
//   // print_freelist();
//   // Assert(0, "Test2 PASS!");
//   // REDLog("Test2 PASS!");
//   // release(&loglk);
// }

void test3(int n) {
  /*更加随机的内存分配和回收*/
  void* mmp[MAXN] = {};
  bool allocated[MAXN] = {};
  srand(uptime());
  memset(allocated, 0, sizeof(allocated));
  int cnt = 0;
  for(int i = 0; i < n; ++i) {
    int flag = rand() % 2;
    // acquire(&testlk);
    if(flag == TOFREE && cnt > 0) {
      // REDLog("#%d free, cnt=%d", i, cnt);
      int id = 0;
      while(allocated[id] == False) {
        id = rand() % n;
        // REDLog("id=%d", id);
      }
      // REDLog("find id=%d", id);
      pmm->free(mmp[id]);
      allocated[id] = False;
      cnt--;
    }
    else {
      size_t size = rand() %  MSIZE;
      // PLog("Alloc Test size=0x%x", size);
      void *ptr = pmm->alloc(size);
      // PLog("Test #%d: Alloc pmm at %p, size=0x%x", i, ptr, size);
      cnt ++;
      // mmp[cnt] = ptr;
      // allocated[cnt] = True;
      for(int i = 0; i < n; ++i) {
        if(!allocated[i]) {
          mmp[i] = ptr;
          allocated[i] = True;
          break;
        }
      }
    }
  }
  // REDLog("Finish alloc");
  for(int i = 0; i < n; ++i) {
    // acquire(&testlk);
    if(allocated[i]) {
      pmm->free(mmp[i]);
      allocated[i] = False;
    }
  };
  REDLog("Test3 PASS!");
}

void hyc_test() {
//  spin_lock(&lk);
  // TestLog("alloc_test begin...");
  // TestLog("(1) Alloc memory for an array with 100 int...");
  int *arr = (int*)pmm->alloc(100 * sizeof(int));
  for (int i = 0; i < 100; i++){
    arr[i] = i;
  }
  // TestLog("(1) Alloc done.");
  for (int i = 0; i < 100; i++){
    // TestLog("(1) arr[%d]: %d", i, arr[i]);
  }
  // TestLog("(2) Free memory for the array...");
  pmm->free(arr);
  // TestLog("(2) Free done.");
  // TestLog("(3) Allocate lots of small memory...");
  for (int i = 0; i < 100; i++){
    pmm->alloc(1);
    pmm->alloc(8);
  }
  // TestLog("(3) Small memory alloc done.");
  // TestLog("(4) Allocate several large memory...");
  pmm->alloc(1 << 12);
  pmm->alloc(1 << 16);
//  pmm->alloc(1 << 24);
  // TestLog("(4) Large memory alloc done.");
  // TestLog("(5) Frequent alloc and free...");
  for (int i = 0; i < 100; i++){
	void *addr = pmm->alloc(i);
	pmm->free(addr);
  }
  // TestLog("(5) Frequent alloc and free done.");
  // TestLog("alloc_test end.\n============================");
//  spin_unlock(&lk);
  REDLog("hycTest PASS!");
}