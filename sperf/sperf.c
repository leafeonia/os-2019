#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<assert.h>
#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)

#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\

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
  		int devnull = open("/dev/null",O_WRONLY);
 		dup2(devnull,STDOUT_FILENO);
  		execlp("strace","-T","-e","raw=all","pstree",NULL);
  		//execlp("ls","ls",NULL);
  		assert(0);
  	}
  	else{
  		close(fd[1]); //close stdout, only read in
  		dup2(fd[0],STDIN_FILENO);
  		//LOG("FUCK FROM PARENT");
  		char buf[1024];
  		while(fgets(buf,1024,stdin))
  			printf("%s",buf);	
  	}
  	
    return 0;
}
