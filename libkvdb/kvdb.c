#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/file.h>
#include "kvdb.h"
#include <errno.h>
struct file{
	const char* filename;
	int cnt;
	pthread_mutex_t* lk;
	FILE* fp;
	struct file* next;
};
typedef struct file file_t;
file_t* file_list = NULL;

pthread_mutex_t open_lk = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t close_lk = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t put_lk = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t get_lk = PTHREAD_MUTEX_INITIALIZER;

int kvdb_open(kvdb_t *db, const char *filename){
	pthread_mutex_lock(&open_lk);
	printf("[%d]open~\n",db->id);
	file_t* cur = file_list;
	file_t* prev = file_list;
	while(cur != NULL){
		if(strcmp(cur->filename,filename) == 0){
			db->filename = filename;
			db->lk = cur->lk;
			db->fp = cur->fp;
			cur->cnt++;
			printf("[%d]open finished(same)~ db->fp = %p,fd = %d\n",db->id, db->fp,fileno(db->fp));
			pthread_mutex_unlock(&open_lk);
			return 0;
		}
		prev = cur;
		cur = cur->next;
	}
	
	FILE* fp = fopen(filename,"w+");
	flock(fileno(fp),LOCK_EX);
	if(fp == NULL){
		printf("error: fopen %s fails\n",filename);
		pthread_mutex_unlock(&open_lk);
		return -1;
	}
	pthread_mutex_t *lk = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(lk,NULL);
	file_t *new_file = (file_t*)malloc(sizeof(file_t));
	new_file->filename = filename;
	new_file->fp = fp;
	new_file->lk = lk;
	new_file->cnt = 1;
	new_file->next = NULL;
	if(file_list == NULL) file_list = new_file;
	else prev->next = new_file;
	db->fp = fp;
	db->filename = filename;
	db->lk = lk;
	//db->opened = 1;
	
	printf("[%d]open finished~(new), db->fp = %p,fd = %d\n",db->id, db->fp, fileno(db->fp));
	flock(fileno(fp),LOCK_UN);
	fclose(fp);
	pthread_mutex_unlock(&open_lk);
	return 0;
}

static file_t* found_filename(const char* filename){
	if(!file_list) return NULL;
	file_t* cur = file_list;
	while(cur){
		if(strcmp(cur->filename,filename) == 0) return cur;
		cur = cur->next;
	}
	return NULL;
}
int kvdb_close(kvdb_t *db){
	pthread_mutex_lock(&close_lk);
	printf("[%d]close~\n",db->id);
	//printf("db->fp = %p, size = %d\n",db->fp,(int)sizeof(db->fp));
	if(!db->fp){
		printf("error: current kvdb has not successfully opened a db file yet\n");
		pthread_mutex_unlock(&close_lk);
		return -1;
	}
	//fclose(db->fp);
	db->fp = NULL;
	file_t* cur = found_filename(db->filename);
	if(!cur){
		printf("warning: the db file has been closed by other thread\n");
		pthread_mutex_unlock(&close_lk);
		return -1;
	}
	//printf("close1\n");
	if(cur->cnt > 1){
		cur->cnt--;
		pthread_mutex_unlock(&close_lk);
		printf("[%d]close finished~(file cnt - 1,but not deleted)\n",db->id);
		return 0;
	}
	file_t* bye;
	//printf("close2\n");
	if(strcmp(file_list->filename,db->filename) == 0){
		//printf("close3\n");
		bye = file_list;
		file_list = file_list->next;
	}
	else{
		file_t* cur = file_list;
		file_t* prev = file_list;
		while(cur){
			//printf("close4\n");
			if(strcmp(cur->filename,db->filename) == 0){
				bye = cur;
				prev->next = cur->next;
				break;
			}
			prev = cur;
			cur = cur->next;
		}
		assert(cur);//shouldn't reach end of list, which means no valid filename found
	}
	//printf("close5\n");
	free(bye);
	//printf("close6\n");
	printf("[%d]close finished~\n",db->id);
	pthread_mutex_unlock(&close_lk);
	//printf("close7\n");
	return 0;
}


int kvdb_put(kvdb_t *db, const char *key, const char *value){
    pthread_mutex_lock(&put_lk);
    if(!db->fp){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        pthread_mutex_unlock(&put_lk);
        return -1;
    }
    if(!found_filename(db->filename)){
		printf("\033[33mwarning: the db file to put data has been closed by other thread. Fail to put data: [%.20s] - [%.20s] into %s.\033[0m\n",key, value, db->filename);
		pthread_mutex_unlock(&put_lk);
		return -1;
	}
    printf("[%d]put~\n",db->id);
    //printf("db->lk = %p\n",db->lk);
    //int fd = fileno(db->fp);
    //flock(fd,LOCK_EX);
    char temp[] = "temp.txt";
    FILE* fptemp = fopen(db->filename,"r+");
    //printf("in put, opens %p, fd = %d\n",fptemp, fileno(fptemp));
    FILE* fp2 = fopen(temp,"w+");
    if(fp2 == NULL){
        printf("error: create temporary file fails\n");
        pthread_mutex_unlock(&put_lk);
        //flock(fd,LOCK_UN);
        return -1;
    }
    FILE* fp = fptemp;//db->fp;
    flock(fileno(fp),LOCK_EX);
    flock(fileno(fp2),LOCK_EX);
    //printf("in put, db->fp = %p, fd = %d\n",db->fp, fileno(db->fp));
    int matched = 0;
    while(!feof(fp)){
    	//printf("oho1\n");
        char key_string[130];
        char *value_string = (char*)malloc(16000002*sizeof(char));
        fgets(key_string,130,fp);
        fgets(value_string,16000002,fp);
        if(feof(fp)) {
        	free(value_string);
        	break;
        }
        if(key_string[strlen(key_string)-1] == '\n') key_string[strlen(key_string)-1] = '\0';
        if(value_string[strlen(value_string)-1] == '\n') value_string[strlen(value_string)-1] = '\0';
        
        if(strcmp(key, key_string) == 0){
            matched = 1;
            fprintf(fp2, "%s\n", key);
            fprintf(fp2, "%s\n", value);
        }
        else{
            fprintf(fp2, "%s\n", key_string);
            fprintf(fp2, "%s\n", value_string);
        }
        free(value_string);
    }
    if(!matched){
        fprintf(fp2, "%s\n", key);
        fprintf(fp2, "%s\n", value);
    }
    /*flock(fileno(fp),LOCK_UN);
    fclose(fp);
    fflush(fp);
    rewind(fp2);
    fp = fopen(db->filename, "w");
    flock(fileno(fp),LOCK_EX);*/
    
    //printf("checkpoint\n");
    flock(fileno(fp),LOCK_UN);
    flock(fileno(fp2),LOCK_UN);
    fclose(fp);
    fclose(fp2);
    fp = fopen(db->filename, "w");
    fp2 = fopen(temp,"r");
    if(!fp2){
    	printf("\033[35m[%d]errorrrrrrr: %s %s,put [%s]-[%s] fails\033[0m\n\n",db->id,strerror(errno),temp,key,value);
    	pthread_mutex_unlock(&put_lk);
    	return -1;
    }
    flock(fileno(fp),LOCK_EX);
    flock(fileno(fp2),LOCK_EX);
    
    while(!feof(fp2)){
    	//printf("meet again\n");
    	char key_string[130];
        char *value_string = (char*)malloc(16000002*sizeof(char));
        fgets(key_string,130,fp2);
        fgets(value_string,16000002,fp2);
        if(feof(fp2)) {
        	free(value_string);
        	break;
        }
        fprintf(fp, "%s", key_string);
        fprintf(fp, "%s", value_string);
        free(value_string);
    }
    flock(fileno(fp),LOCK_UN);
    flock(fileno(fp2),LOCK_UN);
    fclose(fp);
    fclose(fp2);
    //fopen(db->filename,"r+");
    remove("temp.txt");
    /*fclose(fp2);
    if(remove(db->filename) == -1){
        printf("remove origin file fails\n");
        fopen(db->filename,"w+");
        pthread_mutex_unlock(db->lk);
        //flock(fd,LOCK_UN);
        return -1;
    }
    rename(temp,db->filename);
    db->fp = fopen(db->filename,"r+");
    file_t* cur = file_list;
    while(cur){
        	printf("oho2\n");
    	if(strcmp(file_list->filename,db->filename) == 0){
			cur->fp = db->fp;
			break;
		}
		cur = cur->next;
    }
    assert(cur);

    printf("update: filename = %s,db->fp = %p\n",db->filename,db->fp);*/
   	printf("[%d]put [%s]-[%s] finished~\n",db->id, key, value);
   	
    pthread_mutex_unlock(&put_lk);
    //flock(fd,LOCK_UN);
    return 0;
}

char *kvdb_get(kvdb_t *db, const char *key){
	pthread_mutex_lock(&get_lk);
    //printf("[%d]get and pet my \033[33mfluffy tail~ \033[0m\n",db->id);
    if(!db->fp){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        pthread_mutex_unlock(&get_lk);
        return NULL;
    }
    FILE* fp = fopen(db->filename,"r");
    flock(fileno(fp),LOCK_EX);
    while(!feof(fp)){
        char key_string[130];
        char *value_string = (char*)malloc(16000002*sizeof(char));
        fgets(key_string,130,fp);
        fgets(value_string,16000002,fp);
        if(key_string[strlen(key_string)-1] == '\n') key_string[strlen(key_string)-1] = '\0';
        if(value_string[strlen(value_string)-1] == '\n') value_string[strlen(value_string)-1] = '\0';
        if(feof(fp)) break;
        if(strcmp(key, key_string) == 0){
            char* ret = (char*)malloc(sizeof(char)*strlen(value_string)+1);
            if(!ret){
                printf("error: malloc space for return value fails\n");
                free(value_string);
                flock(fileno(fp),LOCK_UN);
                fclose(fp);
                pthread_mutex_unlock(&get_lk);
                return NULL;
            }
            strcpy(ret,value_string);
            free(value_string);
            printf("[%d], get and pet finished!~\n",db->id);
            flock(fileno(fp),LOCK_UN);
            fclose(fp);
            pthread_mutex_unlock(&get_lk);
            return ret;
        }
        free(value_string);
    }
    printf("error: key [%s] does not exist\n",key);
    flock(fileno(fp),LOCK_UN);
    fclose(fp);
    pthread_mutex_unlock(&get_lk);
    return NULL;

}
