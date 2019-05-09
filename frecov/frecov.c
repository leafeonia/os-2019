#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#define ERR(s)\
	{printf("error: %s\n",s);\
	exit(1);}\

int main(int argc, char *argv[]) {
	void* p;
	int fd = open(argv[1],O_RDONLY);
	if(fd == -1){
		ERR("open file failed");
	}
	struct stat st;
	if(fstat(fd,&st) == -1){
		close(fd);
		ERR("obtain file size failed");
	}    
	int length = st.st_size;
	p = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
	if(!p || p == (void*)-1){
		close(fd);
		ERR("mmap failed");
	}
	for(int i =0;i < 100;i++){
		printf("%x ",(unsigned int)(*((int*)p+i)));
	}
	printf("yeah\n");
    return 0;
}
