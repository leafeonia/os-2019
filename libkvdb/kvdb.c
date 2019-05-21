#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/file.h>
#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename){
	printf("open~\n");
	if(db->initialized != 1){
		pthread_mutex_init(&db->lk,NULL);
		db->initialized = 1;
	}
	pthread_mutex_lock(&db->lk);
	FILE* fp = fopen(filename,"w+");
	if(fp == NULL){
		printf("error: fopen %s fails\n",filename);
		pthread_mutex_unlock(&db->lk);
		return -1;
	}
	db->opened = 1;
	db->fp = fp;
	db->filename = filename;
	pthread_mutex_unlock(&db->lk);
	return 0;
}
int kvdb_close(kvdb_t *db){
	printf("close~\n");
	pthread_mutex_lock(&db->lk);
	if(db->opened != 1){
		printf("error: current kvdb has not successfully opened a db file yet\n");
		pthread_mutex_unlock(&db->lk);
		return -1;
	}
	db->opened = 0;
	if(!db->fp){
		printf("error: file has been closed. This may be due to the operation of other threads\n");
	} 
	else fclose(db->fp);
	db->fp = NULL;
	pthread_mutex_unlock(&db->lk);
	return 0;
}


int kvdb_put(kvdb_t *db, const char *key, const char *value){
    printf("put~\n");
    pthread_mutex_lock(&db->lk);
    if(db->opened != 1){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        pthread_mutex_unlock(&db->lk);
        return -1;
    }
    int fd = fileno(db->fp);
    flock(fd,LOCK_EX);
    char temp[] = "temp.txt";
    FILE* fp2 = fopen(temp,"w");
    if(fp2 == NULL){
        printf("error: create temporary file fails\n");
        pthread_mutex_unlock(&db->lk);
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
        pthread_mutex_unlock(&db->lk);
        flock(fd,LOCK_UN);
        return -1;
    }
    rename(temp,db->filename);
    db->fp = fopen(db->filename,"r+");
    printf("update: filename = %s,db->fp = %s\n",db->filename,db->fp);
    pthread_mutex_unlock(&db->lk);
    flock(fd,LOCK_UN);
    return 0;
}

char *kvdb_get(kvdb_t *db, const char *key){
    //printf("get and pet my \033[33mfluffy tail~ \033[0m\n");
    if(db->opened != 1){
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
