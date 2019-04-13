#include<stdlib.h>
#include<stdio.h>
#include<stdlib.h>

#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\
	
int main(int argc, char *argv[]) {
    char template[] = "template-XXXXXX.c";
    int fd = mkstemps(template,2);
    if (fd == -1) printf("FUCK");//ERR("mkstemp fails");
    printf("?\n");
    printf("template=%s,fd = %d\n", template, fd); 
    while(1){
    	
    }
    unlink(template);
    close(fd);
    return 0;
}
