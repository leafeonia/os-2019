#include <stdio.h>
#include <string.h>
#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename){
	printf("open~\n");
	FILE* fp = fopen(filename,"w+");
	if(fp == NULL){
		printf("error: fopen %s fails\n",filename);
		return -1;
	}
	db->opened = 1;
	db->fp = fp;
	db->filename = filename;
	return 0;
}
int kvdb_close(kvdb_t *db){
	if(db->opened != 1){
		printf("error: current kvdb has not successfully opened a db file yet\n");
		return -1;
	}
	fclose(db->fp);
	db->opened = 0;
	printf("close~\n");
	return 0;
}
int kvdb_put(kvdb_t *db, const char *key, const char *value){
    printf("put~\n");
    if(db->opened != 1){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        return -1;
    }
    char temp[] = "temp.txt";
    FILE* fp2 = fopen(temp,"w");
    if(fp2 == NULL){
        printf("error: create temporary file fails\n");
        return -1;
    }
    FILE* fp = db->fp;
    int matched = 0;
    while(!feof(fp)){
        char key_string[50000];
        char value_string[50000];
        fgets(key_string,50000,fp);
        fgets(value_string,50000,fp);
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
        return -1;
    }
    rename(temp,db->filename);
    db->fp = fopen(db->filename,"r+");
    return 0;
}

char *kvdb_get(kvdb_t *db, const char *key){
    printf("get and pet my \033[33mfluffy tail~ \033[0m\n");
    if(db->opened != 1){
        printf("error: current kvdb has not successfully opened a db file yet\n");
        return NULL;
    }
    FILE* fp = db->fp;
    rewind(fp);
    while(!feof(fp)){
        char key_string[50000];
        char value_string[50000];
        fgets(key_string,50000,fp);
        fgets(value_string,50000,fp);
        if(key_string[strlen(key_string)-1] == '\n') key_string[strlen(key_string)-1] = '\0';
        if(value_string[strlen(value_string)-1] == '\n') value_string[strlen(value_string)-1] = '\0';
        if(feof(fp)) break;
        if(strcmp(key, key_string) == 0){
            char* ret = (char*)malloc(sizeof(char)*strlen(value_string)+1);
            if(!ret){
                printf("error: malloc space for return value fails\n");
                return NULL;
            }
            strcpy(ret,value_string);
            return ret;
        }
    }
    printf("error: key [%s] does not exist\n",key);
    return NULL;

}
