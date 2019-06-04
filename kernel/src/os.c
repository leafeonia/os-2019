#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"
#include "vfs.h"

//#define L2_TEST
#define L3_TEST
#define NR_IRQ 20

struct irq{
	int event, seq;
	handler_t handler;
} irqs[NR_IRQ];

static int irq_id = 0;
static spinlock_t lk_trap;
//extern int holding(spinlock_t* lk);

/*static sem_t empty,full,mutex;
static void producer(){
	for(int i = 0;i < 30;i++){
		kmt->sem_wait(&empty);
		kmt->sem_wait(&mutex);
		printf("(");
		for(volatile int j = 0;j < 200000;j++);
		kmt->sem_signal(&mutex);
		kmt->sem_signal(&full);
	}
	//printf("_intr_read() = %d\n",_intr_read());
	while(1);
}

static void consumer(){
	for(int i = 0;i < 30;i++){
		kmt->sem_wait(&full);
		kmt->sem_wait(&mutex);
		printf(")");
		for(volatile int j = 0;j < 200000;j++);
		kmt->sem_signal(&mutex);
		kmt->sem_signal(&empty);
	}
	//printf("empty: %d %d, full: %d %d, mutex: %d %d\n",empty.lock.locked,empty.value,full.lock.locked,full.value,mutex.lock.locked,mutex.value);
	//printf("consumer: _intr_read() = %d\n",_intr_read());
	while(1);
}*/

void echo_task(void *name){
	device_t *tty = dev_lookup(name);
	//printf("tty: 0x%x, tty->ops = 0x%x\n",tty,tty->ops);
	while(1){
		//printf("echo %s\n",name);
		char line[128], text[128];
		sprintf(text, "(%s) $ ",name);
		//printf("strlen = %d\n",strlen(name));
		tty->ops->write(tty, 0, text, strlen(text));
		int nread = tty->ops->read(tty, 0, line, sizeof(line));
		//printf("checkpoint %s\n",name);
		line[nread - 1] = '\0';
		sprintf(text, "Echo %s.\n", line);
		tty->ops->write(tty, 0, text, strlen(text));
		//printf("finish %s\n",name);
	}
}



void dummy_test(void* arg){
	int i = (intptr_t)arg;
	/*if(i == 2){
		LOG("help me");
		return;
	}*/
	int num = 0;
	while(1) {
		if(i != 2)
			printf("FA%d\n",i);
		else
			printf("%d ",num++);
		for(volatile int j = 0;j < 100000;j++);
	}
	//printf("FA\n");
}

void fs(){
	/*file_t* fs1fildes = (file_t*)(0xa3dc10);
	for(int i = 0;i < NR_FILE;i++){
		printf("%d:?%x %x?\n",i,(char*)fs1fildes+4*i,*((file_t*)((char*)fs1fildes+4*i)));
	}*/
	//vfs->open("/a.txt",1);
	//vfs->open("/proc/fa/fa2/proc",2);
	
	
	/*int fd = vfs->open("/dev/tty1",1);
	int fd2 = vfs->open("/dev/ramdisk0",1);
	char buf[] = "D o  y o u  l i k e  w h a t  y o u  s e e";
	char buf2[] = "\ndeep dark fantasy";
	char buf3[] = "leafeonia~";
	
	vfs->write(fd, buf, strlen(buf));
	vfs->write(fd, buf2, strlen(buf2));
	
	vfs->write(fd2, buf3, strlen(buf3));
	char buf4[100];
	vfs->read(fd2,buf4,strlen(buf3));
	printf("read1: %s\n",buf4);
	vfs->close(fd2);
	vfs->read(fd2,buf4,strlen(buf3));
	printf("read2: %s\n",buf4);
	fd2 = vfs->open("/dev/ramdisk0",1);
	vfs->read(fd2,buf4,strlen(buf3));
	printf("read3: %s\n",buf4);


	vfs->close(fd);
	vfs->write(fd, buf, strlen(buf));*/
	int fd1 = vfs->open("/test.txt",0);
	int fd2 = vfs->open("/test.txt",O_CREAT);
	int fd3 = vfs->open("/test.txt",0);
	int fd4 = vfs->open("/a/b.txt",O_CREAT);
	CYANLOG("%d %d %d %d",fd1,fd2,fd3,fd4);
	vfs->close(fd2);
	vfs->close(fd3);
	while(1);
}

static void ls(char* output, char* pwd){
	char path[128];
	sprintf(path,"%s/.",pwd);
	int fd = vfs->open(path, 0);//TODOFLAG
	dire_t dir[NR_DIRE];
	vfs->read(fd,dir,BLOCK_SIZE);
	for(int i = 0;i < NR_DIRE;i++){
		printf("%d - name: %s, inode_id: %d\n",i,dir[i].name, dir[i].inode_id);
	}
}

static void shell(void* name){
  device_t* tty = dev_lookup(name);
  
  char pwd[128];
  sprintf(pwd,"/");
  while (1) {
    char line[128], text[128];
    sprintf(text, "(%s) $ ", name); 
    tty->ops->write(tty, 0, text, strlen(text));
    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    if(strcmp("ls",line) == 0){
    	sprintf(text, "catch ls.\n");
    	ls(text, pwd);
    	tty->ops->write(tty, 0, text, strlen(text));
    }
    else {
    	sprintf(text, "Echo: %s.\n", line);
    	tty->ops->write(tty, 0, text, strlen(text));
    }
    // supported commands:
    //   ls
    //   cd /proc
    //   cat filename
    //   mkdir /bin
    //   rm /bin/abc
    //   ...
  }
}

static void os_init() {
  //LOG("os_init");
  //printf("begin of os_init. intr_read = %d\n",_intr_read());
  pmm->init();
  kmt->init(); 
  kmt->spin_init(&lk_trap,"lk_trap");
  dev->init();
  vfs->init();
  //_vme_init(pmm->alloc,pmm->free);
  
  #ifdef L2_TEST
  kmt->sem_init(&empty,"empty",5);
  kmt->sem_init(&full,"full", 0);
  kmt->sem_init(&mutex,"mutex",1);
  kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, NULL);
  kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, NULL);
  kmt->create(pmm->alloc(sizeof(task_t)), "print1", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print2", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print3", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print4", echo_task, "tty4");
  //printf("checkpoint 1 of os_init. intr_read = %d\n",_intr_read());
  /*kmt->create(pmm->alloc(sizeof(task_t)), "dummy1", dummy_test, (void*)1);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy2", dummy_test, (void*)2);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy3", dummy_test, (void*)3);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy4", dummy_test, (void*)4);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy5", dummy_test, (void*)5);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy6", dummy_test, (void*)6);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy7", dummy_test, (void*)7);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy8", dummy_test, (void*)8);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy9", dummy_test, (void*)9);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy10", dummy_test, (void*)10);
  kmt->create(pmm->alloc(sizeof(task_t)), "dummy11", dummy_test, (void*)11);*/
  #endif
  #ifdef L3_TEST
  //kmt->create(pmm->alloc(sizeof(task_t)), "fs1", fs, NULL);
  //kmt->create(pmm->alloc(sizeof(task_t)), "fs2", fs, NULL);
  kmt->create(pmm->alloc(sizeof(task_t)), "shell1", shell, "tty1");
  #endif
  //printf("end of os_init. intr_read = %d\n",_intr_read());
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
  //printf("os_trap: event = %d\n",ev.event);
  //printf("cpu() = %d\n",_cpu());
  //if(ev.event == _EVENT_IRQ_TIMER) return context;
  //if(!holding(&lk_trap)) kmt->spin_lock(&lk_trap);
  //else LOG("???????");
  kmt->spin_lock(&lk_trap);
  _Context* ret = context;
  //if(ev.event != 5)printf("ev = %d\n",ev.event);
  /*if(ev.event == 2) {
  	kmt->spin_unlock(&lk_trap);
  	return context;
  }*/
  //printf("context->eip = 0x%x\n",context->eip);
  for(int i = 0;i < irq_id;i++){
  	if(irqs[i].event == _EVENT_NULL || irqs[i].event == ev.event){
  		//if(irqs[i].event != _EVENT_NULL) LOG("BOOM");
  		_Context *next = irqs[i].handler(ev,context);
  		if(next) ret = next;
  	}
  }
  /*
  extern task_t** current_task[16];
	task_t** cur = current_task[_cpu()];
	printf("in os_trap. %s: %x\n",(*cur)->name,(*cur)->fildes);
    file_t* fs1fildes = (file_t*)(0xa3dc10);
	for(int i = 0;i < NR_FILE;i++){
		printf("%d:?%x %x?\n",i,(char*)fs1fildes+4*i,*((file_t*)((char*)fs1fildes+4*i)));
		char tmp[20];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%x",*((file_t*)((char*)fs1fildes+4*i)));
		if(strcmp(tmp,"0") != 0){
			//printf("killer: %s, strcmp(%s, \"0\") = %d\n",tmp,tmp,strcmp(tmp,"0"));
			assert(0);
		}
			
	}
LOG("I love your mom");*/
  kmt->spin_unlock(&lk_trap);
  

  
  //return context;
  //printf("os_trap returns task with context address: 0x%x\n",ret);
  
  return ret;
}


static void os_on_irq(int seq, int event, handler_t handler) {
	irqs[irq_id].seq = seq;
	irqs[irq_id].event = event;
	irqs[irq_id++].handler = handler;
	for(int i = 0;i < irq_id;i++){
		for(int j = i + 1;j < irq_id;j++){
			if(irqs[i].seq > irqs[j].seq){
				struct irq temp = irqs[i];
				irqs[i] = irqs[j];
				irqs[j] = temp;
			}
		}
	}
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
