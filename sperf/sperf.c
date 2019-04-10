#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\

int main(int argc, char *argv[]) {
	printf("FUCK\n");
	execl("strace","strace","-T","ls",NULL);
	/*
	int fd[2];
	if(pipe(fd) == -1) ERR("pipe fails");
  	int rc = fork();
  	if(rc > 0){
  		ERR("fork fails");
  	}
  	else if(rc == 0){ //child
  		close(fd[0]);
  		dup2(fd[1],STDOUT_FILENO);
  		execl("strace","strace","-T","ls",NULL);
  	}
  	else{
  		close(fd[1]); //close stdout, only read in
  		dup2(STDOUT_FILENO,fd[0]);
  	}
  	*/
    return 0;
}
