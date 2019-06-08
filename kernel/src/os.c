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
	//int fd1 = vfs->open("/test.txt",0);
	int fd2 = vfs->open("/test.txt",O_CREAT);
	int fd3 = vfs->open("/test.txt",0);
	//int fd4 = vfs->open("/a/b.txt",O_CREAT);
	//CYANLOG("%d %d %d %d",fd1,fd2,fd3,fd4);
	vfs->close(fd2);
	vfs->close(fd3);
	while(1);
}

static void ls(char* output, char* pwd){
	char path[128];
	if(strncmp("/dev",pwd,4) == 0){
		sprintf(output,"ramdisk0 ramdisk1 input fb tty1 tty2 tty3 tty4\n");
		return;
	}
	sprintf(path,"%s/.",pwd);
	int fd = vfs->open(path, 0);//TODOFLAG
	dire_t dir[NR_DIRE];
	vfs->read(fd,dir,BLOCK_SIZE);
	vfs->close(fd);
	for(int i = 0;i < NR_DIRE;i++){
		if(dir[i].inode_id){
			if(strcmp(dir[i].name,".") == 0 || strcmp(dir[i].name,"..") == 0) continue;
			char temp[64];
			sprintf(temp,"%s ",dir[i].name);
			strcat(output, temp);
		}
		//printf("%d - name: %s, inode_id: %d\n",i,dir[i].name, dir[i].inode_id);
	}
	strcat(output,"\n");
}

static void touch(char* output, char* pwd, char* filename){
	char newpath[128];
	if(strcmp(pwd,"/") == 0) sprintf(newpath,"/%s",filename);
	else sprintf(newpath,"%s/%s",pwd,filename);
//	sprintf(output,newpath);
	if(vfs->open(newpath, O_CREAT) == -1) sprintf(output, "touch %s fails\n", filename);
}

void echo(char* output, char* pwd,char* filename,char* content){
	char newpath[128];
	if(strcmp(pwd,"/") == 0) sprintf(newpath,"/%s",filename);
	else sprintf(newpath,"%s/%s",pwd,filename);
	int fd = vfs->open(newpath, O_CREAT);
	if(fd == -1) {
		sprintf(output, "echo \"%s\" into %s fails\n",content, filename);
		return;
	}
	vfs->lseek(fd, 0, SEEK_END);
	vfs->write(fd, content, strlen(content));
	vfs->close(fd);
}

static void rm(char* output, char* pwd, char* filename){
	char newpath[128];
	if(strcmp(pwd,"/") == 0) sprintf(newpath,"/%s",filename);
	else sprintf(newpath,"%s/%s",pwd,filename);
	if(vfs->unlink(newpath) == -1){
		sprintf(output, "rm fails\n");
	}
}

static void cat(char* output, char* pwd, char* filename){
	char newpath[128];
	if(strcmp(pwd,"/") == 0) sprintf(newpath,"/%s",filename);
	else sprintf(newpath,"%s/%s",pwd,filename);
	int fd = vfs->open(newpath, 0);
	if(fd == -1) {
		sprintf(output, "cat: open %s fails\n",newpath);
		return;
	}
	vfs->read(fd, output, BLOCK_SIZE);
	vfs->close(fd);
	strcat(output,"\n");
}

static void link(char* output, char* pwd, char* oldpath, char* newpath){
	CYANLOG("link: %s %s",oldpath, newpath);
	char absolute_oldpath[128];
	char absolute_newpath[128];
	if(strcmp(pwd,"/") == 0) {
		sprintf(absolute_newpath,"/%s",newpath);
		sprintf(absolute_oldpath,"/%s",oldpath);
	}
	else{
		sprintf(absolute_newpath,"%s/%s",pwd, newpath);
		sprintf(absolute_oldpath,"%s/%s",pwd, oldpath);
	}
	if(vfs->link(absolute_oldpath, absolute_newpath)) sprintf(output, "link fails\n");
	//else sprintf(output,"\0");
}

static void mkdir(char* output, char* pwd, char* dirname){
	char newpath[128];
	if(strcmp(pwd,"/") == 0) sprintf(newpath,"/%s",dirname);
	else sprintf(newpath,"%s/%s",pwd,dirname);
	if(vfs->mkdir(newpath) != 0) sprintf(output, "mkdir %s fails\n",dirname);
}

static void cd(char* output,char* pwd, char* dir){
	if(strcmp(".",dir) == 0) return;
	else if(strcmp("..",dir) == 0){
		char* cur = pwd + strlen(pwd) - 1;
		while(cur != pwd && *cur != '/') cur--;
		*cur = '\0';
		if(strlen(pwd) == 0) sprintf(pwd,"/");
	}
	else{
		char temp[128];
		strcpy(temp, pwd);
		if(strcmp(temp, "/") == 0) strcat(temp, dir);
		else{
			strcat(temp,"/");
			strcat(temp,dir);
		}
		if(vfs->open(temp,O_DIRE) == -1) sprintf(output,"cd %s fails\n",dir);
		else strcpy(pwd, temp);
	}
	CYANLOG("after cd, new pwd = %s",pwd);
}

static void rmdir(char* output, char* pwd, char* dirname){
	char newpath[128];
	if(strcmp(pwd,"/") == 0 && (strcmp(dirname,"proc") == 0 || strcmp(dirname,"dev") == 0)){
		sprintf(output, "rmdir: removing /%s is prohibited\n", dirname);
		return;
	}
	if(strcmp(dirname,".") == 0 || strcmp(dirname,"..") == 0){
		sprintf(output, "rmdir: removing %s is prohibited\n", dirname);
		return;
	}
	if(strcmp(pwd,"/") == 0) sprintf(newpath,"/%s",dirname);
	else sprintf(newpath,"%s/%s",pwd,dirname);
	if(vfs->rmdir(newpath) != 0) sprintf(output, "rmdir %s fails\n",dirname);
}

static void shell(void* name){
  //device_t* tty = dev_lookup(name);
  char input[512];
  char output[4096];
  sprintf(input,"/dev/%s",name);
  int stdin = vfs->open(input, 1);//TODOFLAG
  int stdout = vfs->open(input, 4);
  char pwd[128];
  sprintf(pwd,"/");
  while (1) {
    //char input[128], output[512];
    memset(input,0,sizeof(input));
    memset(output, 0, sizeof(output));
    sprintf(output, "(%s) %s $ ", name, pwd); 
    //tty->ops->write(tty, 0, output, strlen(output));
    //LOG("FA1");
    vfs->write(stdout, output, sizeof(output));
    memset(output, 0, sizeof(output));
    int nread = vfs->read(stdin, input, sizeof(input));
    input[nread - 1] = '\0';
    
    //ls
    if(strcmp("ls",input) == 0){
    	ls(output, pwd);
    }
    
    //touch
    else if(strncmp("touch ",input,6) == 0){
    	sprintf(output,"\0");
    	char* newfile = input + 6;
    	while(*newfile == ' ') newfile++; //remove blank
    	if(strlen(newfile) == 0) sprintf(output,"touch: please type in filename\n");
    	else touch(output, pwd, newfile);
    }
    
    //echo
    else if(strncmp("echo ",input,5) == 0){
    	char* content = input + 5;
    	while(*content && *content != '"') content++;
    	content++;
    	int le = 0;
    	for(;*(content+le);le++){
    		if(*(content + le) == '"' && *(content + le - 1) != '\\') break;
    	}
    	*(content + le) = '\0';
    	if(strlen(content) == 0){
    		sprintf(output,"echo: please type in content (content must be wrapped in \" \")\n");
    	}
    	else{
    		char* filename = content + le + 1;
    		le = 0;
    		while(*filename && strncmp(filename, ">>", 2)) filename++;
    		filename += 2;
    		if(strlen(filename) == 0){
    			sprintf(output,"%s\n", content);
    		}
    		else{
    			while(*filename == ' ') filename++;
    			if(strlen(filename) == 0) sprintf(output,"echo: please type in filename\n");
    			else {
    				//sprintf(output,"add \"%s\" into file %s\n",content, filename);
    				echo(output, pwd, filename, content);
    			}
    		}
    	} 
    	
    }
    
    //rm
    else if(strncmp("rm ",input, 3) == 0){
    	char* filename = input + 3;
    	while(*filename == ' ') filename++; //remove blank
    	if(strlen(filename) == 0) sprintf(output,"please type in filename\n");
    	else rm(output, pwd, filename);
    }
    
    //pwd
    else if(strncmp("pwd",input ,3) == 0){
    	sprintf(output,"%s\n", pwd);
    }
    
    else if(strncmp("cat ",input ,4) == 0){
    	char* filename = input + 4;
    	while(*filename == ' ') filename++; //remove blank
    	if(strlen(filename) == 0) sprintf(output,"please type in filename\n");
    	else cat(output,pwd, filename);
    }
    
    else if(strncmp("link ",input ,5) == 0){
    	char* oldpath = input + 5;
    	while(*oldpath == ' ') oldpath++; //remove blank
    	int le = 0;
    	while(*(oldpath + le) && *(oldpath + le) != ' ') le++;
    	if(!*(oldpath + le)) sprintf(output,"please type in newpath\n");
    	else{
    		*(oldpath + le) = '\0';
			char* newpath = oldpath + le + 1;
			while(*newpath == ' ' ) newpath++;
			if(strlen(newpath) == 0) sprintf(output,"please type in newpath\n");
			else link(output, pwd, oldpath, newpath);
    	}
    	
    }
    
    else if(strncmp("mkdir ",input,6) == 0){
    	char* dirname = input + 6;
    	while(*dirname == ' ') dirname++;
    	if(strlen(dirname) == 0) sprintf(output,"please type in name of new directory\n");
    	else{
    		mkdir(output, pwd, dirname);
    	}
    }
    else if(strncmp("cd ",input, 3) == 0){
    	char* dirname = input + 3;
    	while(*dirname == ' ') dirname++;
    	if(strlen(dirname) == 0) sprintf(output,"please type in name of directory\n");
    	else{
    		cd(output, pwd, dirname);
    	}
    }
    
    else if(strncmp("rmdir ",input, 6) == 0){
    	char* dirname = input + 6;
    	while(*dirname == ' ') dirname++;
    	if(strlen(dirname) == 0) sprintf(output,"please type in name of new directory\n");
    	else{
    		rmdir(output, pwd, dirname);
    	}
    }
    
    else {
    	sprintf(output, "Invalid operation. Supported commands: ls pwd echo touch cat link mkdir cd rm rmdir.\n", input);
    }
    vfs->write(stdout, output, sizeof(output));
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
  kmt->create(pmm->alloc(sizeof(task_t)), "shell2", shell, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "shell3", shell, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "shell4", shell, "tty4");
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
