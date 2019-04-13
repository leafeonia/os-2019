#include<stdlib.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<assert.h>

#define ERR(s)\
	{printf("error: %s\n",s);\
	exit(1);}\

void upload_so(char* source_name){
	int rc = fork();
	if(rc < 0) ERR("fork fails");
	else if(rc == 0){
		execlp("gcc","gcc","-fPIC","-shared",source_name,"-o"," libfunc.so");
		assert(0);
	}
	else{
		int wc = wait(NULL);
	}
	return 0;
} 

int main(int argc, char *argv[]) {
    char template[] = "temp-XXXXXX.c";
    int fd = mkstemps(template,2);
    if (fd == -1) ERR("mkstemp fails");
    printf("template=%s,fd = %d\n", template, fd); 
    char command[512];
    printf(">> ");
    while(1){
    	memset(command,0,sizeof(command));
    	fgets(command,sizeof(command),stdin);
    	if(strcmp(command,"\n") == 0) continue;
    	if(strcmp(command,"q\n") == 0) break;
    	if(write(fd, command, strlen(command)) == -1) ERR("write fails");
    	upload_so(template);
    	printf(">> ");
    }
    //read(fd,command,sizeof(command));
    //printf("%s\n",command);
    unlink(template);
    close(fd);
    return 0;
}  
