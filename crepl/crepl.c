#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[]) {
    char template[] = "template-XXXXXX";
    mktemp(template);
    printf("template=%s\n", template); 
    while(1);
    return 0;
}
