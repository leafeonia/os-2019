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

void upload_so(char* source_name,char* lib_name){
	int rc = fork();
	if(rc < 0) ERR("fork fails");
	if(rc == 0){
		execlp("gcc","gcc","-fPIC","-shared",source_name,"-o",lib_name,NULL);
		assert(0);
	}
	else{
		wait(NULL);
	}
	return;
} 

int main(int argc, char *argv[]) {
    char template_source[] = "temp-XXXXXX.c";
    char template_lib[] = "temp-XXXXXX.so";
    int fd = mkstemps(template_source,2);
    int fd2 = mkstemps(template_lib,3);
    if (fd == -1 || fd2 == -1) ERR("mkstemp fails");
    char command[512];
    printf(">> ");
    while(1){
    	memset(command,0,sizeof(command));
    	fgets(command,sizeof(command),stdin);
    	if(strcmp(command,"\n") == 0) continue;
    	if(strcmp(command,"q\n") == 0) break;
    	if(write(fd, command, strlen(command)) == -1) ERR("write fails");
    	upload_so(template_source,template_lib);
    	printf(">> ");
    }
    //read(fd,command,sizeof(command));
    //printf("%s\n",command);
    unlink(template_lib);
    unlink(template_source);
    close(fd);
    close(fd2);
    return 0;
}  
