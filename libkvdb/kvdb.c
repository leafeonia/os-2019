#include <stdio.h>
#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename){
	printf("open~\n");
	return 0;
}
int kvdb_close(kvdb_t *db){
	printf("close~\n");
	return 0;
}
int kvdb_put(kvdb_t *db, const char *key, const char *value){
	printf("put it inside me~\n");
	return 0;
}

char tmp[2]; 
char *kvdb_get(kvdb_t *db, const char *key){
	printf("get and pet my fluffy tail~\n");
	return tmp;
}
