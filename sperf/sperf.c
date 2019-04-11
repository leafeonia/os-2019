#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<assert.h>
#include<regex.h>
#include<sys/types.h>
#include<string.h>

#define LEN_NAME 64

#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)

#define ERR(s)\
	printf("error: %s\n",s);\
	exit(1);\

int main(int argc, char *argv[]) {
	pid_t rc;
	int fd[2];
	pipe(fd);
  	rc = fork();
  	if(rc < 0){
  		ERR("fork fails");
  	}
  	else if(rc == 0){ //child
  		//LOG("FUCK FROM CHILD");
  		close(fd[0]);
  		dup2(fd[1],STDERR_FILENO);
  		int devnull = open("/dev/null",O_WRONLY);
 		dup2(devnull,STDOUT_FILENO);
  		execlp("strace","strace","-T","-e","raw=all","ls",NULL);
  		//execlp("ls","ls",NULL);
  		assert(0);
  	}
  	else{
  		close(fd[1]); //close stdout, only read in
  		dup2(fd[0],STDIN_FILENO);
  		//LOG("FUCK FROM PARENT");
  		char buf[1024];
  		regex_t preg_one, preg_two;//match syscall name, time, perspectively
  		regmatch_t matches_one[1],matches_two[1];
  		regcomp(&preg_one,"^[a-zA-Z0-9_]+",REG_EXTENDED);
  		regcomp(&preg_two,"<.*>",REG_EXTENDED);
  		while(fgets(buf,1024,stdin)){
  			int is_matched_one = regexec(&preg_one,buf,1,matches_one,0);
  			int is_matched_two = regexec(&preg_two,buf,1,matches_two,0);
  			if(is_matched_one == REG_NOMATCH || is_matched_two == REG_NOMATCH){
  				printf("NO MATCH\n");
  			}	
  			else{
  				//printf("enter\n");
  				char sys_name[LEN_NAME];
  				char sys_time[10];
  				memcpy(sys_name,buf+matches_one[0].rm_so,matches_one[0].rm_eo-matches_one[0].rm_so);
  				memcpy(sys_time,buf+matches_two[0].rm_so,matches_two[0].rm_eo-matches_two[0].rm_so);
  				sys_name[matches_one[0].rm_eo-matches_one[0].rm_so] = '\0';
  				sys_time[matches_two[0].rm_eo-matches_two[0].rm_so] = '\0';
  				printf("%s %s\n",sys_name,sys_time);
  			}
  			//printf("%s",buf);	
  		}
  	}
  	
    return 0;
}
