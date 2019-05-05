#include<stdlib.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<assert.h>
#include<fcntl.h>
#include<dlfcn.h>

#define ERR(s)\
	{printf("error: %s\n",s);\
	exit(1);}\

static int m64 = 1;

int upload_so(char* source_name,char* lib_name,int command_len){
	int rc = fork();
	
	if(rc < 0) ERR("fork fails");
	if(rc == 0){
		int devnull = open("/dev/null",O_WRONLY);
 		dup2(devnull,STDERR_FILENO);
 		if(m64 == 1) execlp("gcc","gcc","-shared","-fPIC",source_name,"-o",lib_name,"-ldl",NULL);
 		else execlp("gcc","gcc","-shared","-fPIC","-m32",source_name,"-o",lib_name,"-ldl",NULL);
		assert(0);
	}
	else{
		int status = 0;
		wait(&status);
		if(WEXITSTATUS(status) == 1) {
			printf("\033[1;31mcompile error\33[0m\n");
			FILE* fp = fopen(source_name,"r+");
			if(fp == NULL) ERR("fopen fails");
			fseek(fp,-command_len,SEEK_END);
			fputs("//",fp); //comment the uncompliable command
			fflush(fp);
			fclose(fp);
			return 1;
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
	return 0;
} 

int main(int argc, char *argv[]) {
	if(sizeof(long) == 4) m64 = 0;
    char template_source[] = "temp-XXXXXX.c";
    char template_lib[] = "temp-XXXXXX.so";
    int fd = mkstemps(template_source,2);
    int fd2 = mkstemps(template_lib,3);
    if (fd == -1 || fd2 == -1) ERR("mkstemp fails");
    /*char template_source[] = "temp-1.c";
    char template_lib[] = "temp-1.so";
    char template_func[] = "temp_func.c";
    FILE* fp = fopen(template_source,"a");
    FILE* fp2 = fopen(template_func,"w");*/
    
    char command[512];
    char expr[512];
    int expr_id = 0;
    
    printf("type in 'q' to quit.\n>> ");
    while(fgets(command,sizeof(command),stdin)!=NULL){
    	//memset(command,0,sizeof(command));
    	
    	
    	char temp_buf[100];
    	sscanf(command,"%s",temp_buf);
    	if(strncmp(temp_buf,"\n",1) == 0) continue;
    	if(strncmp(temp_buf,"q\n",2) == 0) break;
    	//function
    	if(strncmp(temp_buf, "int",3) == 0){
    		//fputs(command,fp);
    		//fflush(fp);
    		if(write(fd, command, strlen(command)) == -1) ERR("write fails");
    		if(!upload_so(template_source,template_lib,strlen(command))) printf("\033[1;32madded: %s\33[0m",command);
    	}
    	
    	//expression
    	else{
    		//fputs(command,fp2);
    		//fflush(fp2);
    		expr_id++;
    		command[strlen(command)-1] = '\0'; //replace '\n' to '\0'
    		sprintf(expr,"int __expr_wrap_%d() {return %s;}\n",expr_id,command);
    		if(write(fd, expr, strlen(expr)) == -1) ERR("write fails");
    		if(upload_so(template_source,template_lib,strlen(expr))){
    			printf(">> ");
    			continue;
    		}
    		
    		void *handle;
    		char lib_name_local[25];
    		sprintf(lib_name_local,"./%s",template_lib);
    		if((handle = dlopen(lib_name_local,RTLD_LAZY)) == NULL) {
    			printf("%s\n",dlerror());
    			ERR("dlopen fails");
    		}
    		char func_name[20];
    		//printf("expr_id = %d\n",expr_id);
    		sprintf(func_name,"__expr_wrap_%d",expr_id);
    		int (*func)() = dlsym(handle,func_name);
    		if(dlerror() != NULL) printf("%s\n",dlerror());
    		printf(">> %s = %d\n",command,(*func)());
    		
    		dlclose(handle);// !!! bug producer 
    	}
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
