#include<dlfcn.h>
#include<stdio.h>
extern int f();
int main(){
	void* handle = dlopen("temp-1.so",RTLD_LAZY);
	int (*f)(void);
	f = (int (*)(void)) dlsym(handle,"f");
	printf("%d\n",f());
	return 0;
}
