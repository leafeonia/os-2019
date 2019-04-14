#include<dlfcn.h>
#include<stdio.h>
int main(){
	dlopen("temp-1.so",RTLD_NOW);
	printf("%d\n",f());
	return 0;
}
