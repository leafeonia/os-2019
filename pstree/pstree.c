#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define nr_proc 10000
#define root 0
#define bool char
#define true 1
#define false 0

bool _p = false;
bool _n = false;

typedef struct _node{
	int index; 
	struct _node* next;
}node;

typedef struct _proc_info{
	char name[100];
	int pid,ppid;
	node* childlist;
}proc_info;

proc_info* proc = NULL;
int map[nr_proc];  //map(the index in array 'proc',pid)
int max_index = 1;

void init(){
    proc = (proc_info*)malloc(nr_proc*sizeof(proc_info));
    map[0] = root;
    strcpy(proc[root].name,"(root)");
    int i;
    for(i = 0;i < nr_proc;i++){
    	proc[i].childlist = NULL;
    }
}

int searchIndex(int pid){  //search the index in array 'proc' by pid
	int i;
	for(i = 0;i < max_index;i++){
		if(map[i] == pid) return i;
	}
	return -1;
}

void addchild(int father,int child){
	node* temp = (node*)malloc(sizeof(node*));
	if(proc[father].childlist == NULL){  
	    temp->index = child;
	    temp->next = NULL;
		proc[father].childlist = temp;
	}
	else{
		node* cur = proc[father].childlist;
		while(cur->next) cur = cur->next;
		temp->index = child;
		temp->next = NULL;
		cur->next = temp;
	}
}

void sort(){
	int i,j;
	for(i = 1;i < max_index;i++){
		for(j = i+1;j < max_index;j++){
			if(proc[i].pid > proc[j].pid){
				proc_info temp = proc[i];
				proc[i] = proc[j];
				proc[j] = temp;
				int temp2 = map[i];
				map[i] = map[j];
				map[j] = temp2;
			}
		}
	}
}

void buildTree(){
	if(_n == true) sort();
	int i;
	for(i = 1;i < max_index;i++){
		proc_info cur = proc[i];
		int father = searchIndex(cur.ppid);
		if(father == -1){
			printf("error: cannot find father process\n");
			exit(1);
		}
		addchild(father,i);
	}
}

void print(int cur,int depth){
	int i;
	for(i = 0;i < depth-1;i++) printf("|    ");
	if(cur) printf("|---");
	if(_p == true)printf("%s(%d)\n",proc[cur].name,proc[cur].pid);
	else printf("%s\n",proc[cur].name);
	while(proc[cur].childlist){
		print(proc[cur].childlist->index,depth+1);
		proc[cur].childlist = proc[cur].childlist->next;
	}
}

void input(int argc, char *argv[]){
	while(1){
		int option_index = 0;
		static struct option long_options[] = {
			{"show-pids",no_argument,0,0},
			{"numeric-sort",no_argument,0,0},
			{"version",no_argument,0,0},
			{0,0,0,0}
		};
		int ch = getopt_long(argc,argv,"pnV",long_options,&option_index);
		if(ch == -1) break;
		
		switch(ch){
			case 'p':
				_p = true;
				break;
			case 'V':
				printf("PStree (OS Minilab 171860575) 1.0\n");
						exit(0);
				break;
			case 'n':
				_n = true;
				break;
			case 0:
				switch(option_index){
					case 0:
						_p = true;
						break;
					case 1:
						_n = true;
						break;
					case 2:
						printf("PStree (OS Minilab 171860575) 1.0\n");
						exit(0);
						break;
				}
				break;
			default:
				printf("error: invalid option\n");
				exit(1);
		}
	}
	
}

int main(int argc, char *argv[]) {

  /*printf("Hello, World!\n");
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // always true
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // always true*/
  
  int i;
  
  input(argc,argv);
  
  
  init();

  char buf[100];
  char process_name[100];	
  char filepath[100];
  int pid,ppid;
  
  DIR* dir;
  struct dirent *ptr;
  if((dir = opendir("/proc")) == NULL){
	  printf("error: cannot open /proc\n");
	  exit(2);
  }
  while((ptr = readdir(dir) ) != NULL){
	  //printf("%s\n",ptr->d_name);
	  if('0' <= ptr->d_name[0] && ptr->d_name[0] <= '9' && ptr->d_type == 4){
		  sprintf(filepath,"/proc/%s/status",ptr->d_name);
		  FILE *fp = fopen(filepath,"r");
		  if(fp){
		      fgets(buf,100,fp);
			  sscanf(buf,"%*s %s",process_name);
			  //printf("%s ",process_name);
			  for(i = 0;i < 5;i++){
				fgets(buf,100,fp);
			  }
			  sscanf(buf,"%*s %d",&pid);
			  fgets(buf,100,fp);
			  sscanf(buf,"%*s %d",&ppid);
			  //printf("%d %d\n",pid,ppid);
			  map[max_index] = pid;
			  proc_info temp;
			  temp.pid = pid;
			  temp.ppid = ppid;
			  strcpy(temp.name,process_name);
			  temp.childlist = NULL;
			  proc[max_index] = temp;
			  max_index++;
			  if(max_index > nr_proc){
			  	printf("error: too many processes to handle. Please modify the value of macro nr_proc\n");
			  	exit(1);
			  }
		  }
		  else{
			  printf("error: cannot open the file: %s\n",filepath);
			  exit(2);
		  }
	  }
  }
  
  buildTree();
  /*while(proc[0].childlist){
    proc_info cur = proc[proc[0].childlist->index];
  	printf("%d %s\n",cur.pid,cur.name);
  	proc[0].childlist = proc[0].childlist->next;
  }*/
  
  print(root,0);
  
  return 0;
}
