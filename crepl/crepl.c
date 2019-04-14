#include<stdlib.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<assert.h>
#include<fcntl.h>

#define ERR(s)\
	{printf("error: %s\n",s);\
	exit(1);}\

void upload_so(char* source_name,char* lib_name,int command_len){
	int rc = fork();
	
	if(rc < 0) ERR("fork fails");
	if(rc == 0){
		int devnull = open("/dev/null",O_WRONLY);
 		dup2(devnull,STDERR_FILENO);
		execlp("gcc","gcc","-fPIC","-shared",source_name,"-o",lib_name,NULL);
		assert(0);
	}
	else{
		int status = 0;
		wait(&status);
		if(WEXITSTATUS(status) == 1) {
			printf("\033[31mcompile error\33[0m\n");
			FILE* fp = fopen(source_name,"r+");
			if(fp == NULL) ERR("fopen fails");
			fputc('*',fp);
			//printf("command len = %d\n",command_len);
			fseek(fp,-2,SEEK_END);
			printf("%ld\n",ftell(fp));
			fputs("//",fp);
			fflush(fp);
		}
		/*close(fd[1]); 
  		dup2(fd[0],STDIN_FILENO);
  		char error[512];
  		printf("enter\n");
  		fgets(error,512,stdin);
  		if(strlen(error) > 1){
  			printf("compile error\n");
  			printf("%s\n",error);
  			exit(1);
  		}*/
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
    	upload_so(template_source,template_lib,strlen(command));
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
