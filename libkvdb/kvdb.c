#include <stdio.h>
#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename){
	printf("open~\n");
	FILE* fp = fopen(filename,"a+");
	if(fp == NULL){
		printf("error: fopen %s fails\n",filename);
		db->fp = NULL;
		return 1;
	}
	db->fp = fp;
	return 0;
}
int kvdb_close(kvdb_t *db){
	if(db->fp == NULL){
		printf("error: current kvdb has not successfully opened a db file yet\n");
		return 1;
	}
	fclose(db->fp);
	printf("close~\n");
	return 0;
}
int kvdb_put(kvdb_t *db, const char *key, const char *value){
	printf("put it inside me~\n");
	return 0;
}

char tmp[] = {'f','a'}; 
char *kvdb_get(kvdb_t *db, const char *key){
	printf("get and pet my \033[35mfluffy tail~ \033[0m\n");
	return tmp;
}
