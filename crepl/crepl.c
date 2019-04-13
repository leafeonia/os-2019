#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[]) {
    char template[] = "template-XXXXXX";
    int fd = mkstemp(template);
    printf("template=%s,fd = %d\n", template, fd); 
    while(1);
    return 0;
}
