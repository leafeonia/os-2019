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
		if(strcmp(key, key_string) == 0){
			matched = 1;
			fprintf(fp2, "%s", key);
			fprintf(fp2, "%s", value);
		}
		else{
			fprintf(fp2, "%s", key_string);
			fprintf(fp2, "%s", value_string);
		}
	}
	if(!matched){
		fprintf(fp2, "%s", key);
		fprintf(fp2, "%s", value);
	}
	fclose(fp);
	fclose(fp2);
	remove(db->filename);
	rename(temp,db->filename);
	db->fp = fp2;
	return 0;
}

char tmp[] = {'f','a'}; 
char *kvdb_get(kvdb_t *db, const char *key){
	printf("get and pet my \033[34mfluffy tail~ \033[0m\n");
	return tmp;
}
