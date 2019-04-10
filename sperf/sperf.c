#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>
#define LOG(s) printf("\33[0;31m%s\n\33[0m",s)

#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\

int main(int argc, char *argv[]) {
	pid_t rc;
	int fd[2];
	pipe(fd);
	//if(pipe(fd)) ERR("pipe fails");
	//printf("FUCK\n");
  	rc = fork();
  	if(rc < 0){
  		ERR("fork fails");
  	}
  	else if(rc == 0){ //child
  		LOG("FUCK FROM CHILD");
  		close(fd[0]);
  		dup2(fd[1],STDOUT_FILENO);
  		execlp("strace","strace","-T","ls",NULL);
  	}
  	else{
  		LOG("FUCK FROM PARENT");
  		close(fd[1]); //close stdout, only read in
  		dup2(STDOUT_FILENO,fd[0]);
  	}
  	
    return 0;
}
