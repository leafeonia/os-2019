#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<assert.h>
#include<regex.h>
#include<sys/types.h>
#include<string.h>
#include<sys/time.h>

#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)

#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\
	
#define LEN_NAME 64
#define NR_SYS 1024
typedef struct _sys_call{
	char sys_name[LEN_NAME];
	double sys_time;
}sys_call;

sys_call list[NR_SYS];
int list_max = 0;
//volatile int exited = 0;//father process quits after child process quits

void insert(char* name, double timee){
	for(int i = 0;i < list_max;i++){
		if (strcmp(name,list[i].sys_name) == 0){
			list[i].sys_time += timee;
			return;
		}
	}
	strcpy(list[list_max].sys_name,name);
	list[list_max++].sys_time = timee;
}

void update(){
	for(int i = 0;i < list_max;i++){
  		for(int j = i+1;j < list_max;j++){
  			if(list[i].sys_time < list[j].sys_time){
  				sys_call temp = list[i];
  				list[i] = list[j];
  				list[j] = temp;
  			}
  		}
  	}
  	double tot = 0.0;
  	for(int i = 0;i < list_max;i++){
  		tot += list[i].sys_time;
  	}
  	for(int i = 0;i < list_max;i++){
  		printf("%s %.02f%%\n",list[i].sys_name,list[i].sys_time/tot*100);
  	}
}


void sig_handler(int sig){
	/*if(sig == SIGCHLD){
		exited = 1;
	}*/
	if(sig == SIGALRM){
		update();
	}
}


int main(int argc, char *argv[]) {
	pid_t rc;
	int fd[2];
	pipe(fd);
  	rc = fork();
  	if(rc < 0){
  		ERR("fork fails");
  	}
  	else if(rc == 0){ //child
  		//LOG("FUCK FROM CHILD");
  		close(fd[0]);
  		dup2(fd[1],STDERR_FILENO);
  		//int devnull = open("/dev/null",O_WRONLY);
 		//dup2(devnull,STDOUT_FILENO);
  		//execlp("strace","strace","-T","-e","raw=all","python","-c","2**1000",NULL);
  		//printf("argv[0] = %s,argv[1] = %s,argv[2] = %s\n",argv[0],argv[1],argv[2]);
  		char * arg[10];// = {"strace","-T","-e raw=all",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
  		for(int i = 0;i < 10;i++) arg[i] = (char*)malloc(sizeof(char*));
  		arg[0] = "strace";
  		arg[1] = "-T";
  		arg[2] = "-e raw=all";
  		for(int i = 1;i <= argc;i++)
  			strcpy(arg[2+i],argv[i]);
  		for(int i = 0;i < 6;i++) printf("%s ",arg[i]);
  		printf("?\n");
  		//arg[3] = argv[1];
  		//argv[0] = "strace";
  		//char * argv2[] = {"strace", "-T", "pstree", 0};
    	execvp("strace", arg);
  		//execlp("ls","ls",NULL);
  		assert(0);
  	}
  	
  	else{ //parent
  		printf("argc = %d,argv[0] = %s\n",argc,argv[0]);
  		close(fd[1]); //close stdout, only read in
  		dup2(fd[0],STDIN_FILENO);
  		//LOG("FUCK FROM PARENT");
  		//signal(SIGCHLD,sig_handler);
  		
  		//set timer
  		signal(SIGALRM, sig_handler);
    	struct itimerval new_value, old_value;
    	new_value.it_value.tv_sec = 0;
    	new_value.it_value.tv_usec = 100000;
    	new_value.it_interval.tv_sec = 0;
    	new_value.it_interval.tv_usec = 500000;
    	setitimer(ITIMER_REAL, &new_value, &old_value);

  		
  		for(int i = 0;i < NR_SYS;i++)
  			list[i].sys_time = 0.0;
  		char buf[1024];
  		regex_t preg_one, preg_two;//match syscall name, time, perspectively
  		regmatch_t matches_one[1],matches_two[1];
  		regcomp(&preg_one,"^[a-zA-Z0-9_]+",REG_EXTENDED);
  		regcomp(&preg_two,"<.*>",REG_EXTENDED);
  		
  		while(fgets(buf,1024,stdin)){
  			int is_matched_one = regexec(&preg_one,buf,1,matches_one,0);
  			int is_matched_two = regexec(&preg_two,buf,1,matches_two,0);
  			if(is_matched_one == REG_NOMATCH || is_matched_two == REG_NOMATCH){
  				//printf("NO MATCH\n");
  			}	
  			else{
  				//printf("enter\n");
  				char sys_name[LEN_NAME];
  				char sys_time[12];
  				memset(sys_name,0,LEN_NAME);
  				memset(sys_time,0,12);
  				memcpy(sys_name,buf+matches_one[0].rm_so,matches_one[0].rm_eo-matches_one[0].rm_so);
  				memcpy(sys_time,buf+matches_two[0].rm_so+1,matches_two[0].rm_eo-matches_two[0].rm_so-2);
  				//memset has set '\0', no need to set again
  				//sys_name[matches_one[0].rm_eo-matches_one[0].rm_so] = '\0';
  				//sys_name[matches_one[0].rm_eo-matches_one[0].rm_so] = '\0';
  				insert(sys_name,atof(sys_time));
  				//printf("%s %f\n",sys_name,atof(sys_time));
  			}
  			//printf("%s",buf);	
  		}
  		update();
  		
  		
  	}
  	//while(!exited);
  	
    return 0;
}


//1.get input
//2.dynamic update
//3.regex
//4.exec
