#include<dlfcn.h>
#include<stdio.h>
extern int f();
int main(){
	dlopen("temp-1.so",RTLD_NOW);
	printf("%d\n",f());
	return 0;
}
