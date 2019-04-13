#include<stdlib.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/unistd.h>
#include<string.h>

#define ERR(s)\
	{printf("error: %s\n",s);\
	exit(1);}\
	
int main(int argc, char *argv[]) {
    char template[] = "template-XXXXXX.c";
    int fd = mkstemps(template,2);
    if (fd == -1) ERR("mkstemp fails");
    printf("template=%s,fd = %d\n", template, fd); 
    char command[512];
    while(1){
    	scanf("%[^\n]",command);
    	if(strcmp(command,"q") == 0) break;
    	if(write(fd, command, strlen(command)) == -1) ERR("write fails");
    }
    read(fd,command,sizeof(command));
    printf("%s\n",command);
    unlink(template);
    close(fd);
    return 0;
}  
