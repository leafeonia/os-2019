#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/file.h>
#include "kvdb.h"
struct file{
	const char* filename;
	pthread_mutex_t* lk;
	FILE* fp;
	struct file* next;
};
typedef struct file file_t;
file_t* file_list = NULL;

pthread_mutex_t open_lk = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t close_lk = PTHREAD_MUTEX_INITIALIZER;

int kvdb_open(kvdb_t *db, const char *filename){
	printf("open~\n");
	pthread_mutex_lock(&open_lk);
	file_t* cur = file_list;
	file_t* prev = file_list;
	while(cur != NULL){
		if(strcmp(cur->filename,filename) == 0){
			db->filename = filename;
			db->lk = cur->lk;
			db->fp = cur->fp;
			return 0;
		}
		prev = cur;
		cur = cur->next;
	}
	
	FILE* fp = fopen(filename,"w+");
	if(fp == NULL){
		printf("error: fopen %s fails\n",filename);
		return -1;
	}
	pthread_mutex_t *lk = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(lk,NULL);
	file_t *new_file = (file_t*)malloc(sizeof(file_t));
	new_file->filename = filename;
	new_file->fp = fp;
	new_file->lk = lk;
	new_file->next = NULL;
	if(file_list == NULL) file_list = new_file;
	else prev->next = new_file;
	db->fp = fp;
	db->filename = filename;
	db->lk = lk;
	//db->opened = 1;
	pthread_mutex_unlock(&open_lk);
	return 0;
}
int kvdb_close(kvdb_t *db){
	printf("close~\n");
	pthread_mutex_lock(&close_lk);
	if(!db->fp){
		printf("error: current kvdb has not successfully opened a db file yet\n");
		return -1;
	}
	fclose(db->fp);
	
	db->fp = NULL;
	printf("close1\n");
	file_t* bye;
	if(strcmp(file_list->filename,db->filename) == 0){
		bye = file_list;
		file_list = file_list->next;
	}
	else{
		file_t* cur = file_list;
		file_t* prev = file_list;
		while(cur){
			printf("close2~\n");
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
	free(bye);
	pthread_mutex_unlock(&close_lk);
	return 0;
}


int kvdb_put(kvdb_t *db, const char *key, const char *value){

    printf("put~\n");
    if(!db->fp){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        return -1;
    }
    pthread_mutex_lock(db->lk);
    int fd = fileno(db->fp);
    flock(fd,LOCK_EX);
    char temp[] = "temp.txt";
    FILE* fp2 = fopen(temp,"w");
    if(fp2 == NULL){
        printf("error: create temporary file fails\n");
        pthread_mutex_unlock(db->lk);
        flock(fd,LOCK_UN);
        return -1;
    }
    FILE* fp = db->fp;
    int matched = 0;
    while(!feof(fp)){
        char key_string[130];
        char *value_string = (char*)malloc(16000002*sizeof(char));
        fgets(key_string,130,fp);
        fgets(value_string,16000002,fp);
        if(key_string[strlen(key_string)-1] == '\n') key_string[strlen(key_string)-1] = '\0';
        if(value_string[strlen(value_string)-1] == '\n') value_string[strlen(value_string)-1] = '\0';
        if(feof(fp)) break;
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
    fclose(fp);
    fclose(fp2);
    if(remove(db->filename) == -1){
        printf("remove origin file fails\n");
        fopen(db->filename,"w+");
        pthread_mutex_unlock(db->lk);
        flock(fd,LOCK_UN);
        return -1;
    }
    rename(temp,db->filename);
    db->fp = fopen(db->filename,"r+");
    printf("update: filename = %s,db->fp = %p\n",db->filename,db->fp);
    pthread_mutex_unlock(db->lk);
    flock(fd,LOCK_UN);
    return 0;
}

char *kvdb_get(kvdb_t *db, const char *key){
    //printf("get and pet my \033[33mfluffy tail~ \033[0m\n");
    if(!db->fp){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        return NULL;
    }
    FILE* fp = db->fp;
    rewind(fp);
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
                return NULL;
            }
            strcpy(ret,value_string);
            free(value_string);
            return ret;
        }
        free(value_string);
    }
    printf("error: key [%s] does not exist\n",key);
    return NULL;

}
