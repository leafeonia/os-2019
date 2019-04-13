#include<stdlib.h>
#include<stdio.h>

#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\
	
int main(int argc, char *argv[]) {
    char template[] = "template-XXXXXX.c";
    int fd = mkstemps(template,2);
    if (fd == -1) ERR("mkstemp fails");
    printf("template=%s,fd = %d\n", template, fd); 
    while(1){
    	
    }
    return 0;
}
