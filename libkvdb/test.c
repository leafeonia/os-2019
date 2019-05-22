#include "kvdb.h"
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

void* test1(){
	printf("FA1\n");
	kvdb_t db;
	kvdb_open(&db, "a.db"); 
    kvdb_put(&db, "operating systems", "three-easy-pieces");
    kvdb_put(&db, "glaceon", "471");
    kvdb_put(&db, "flareon", "136");
    kvdb_close(&db);
    kvdb_open(&db, "b.db");
    kvdb_put(&db, "eevee", "132");
    kvdb_put(&db, "vaporeon", "134");
    kvdb_put(&db, "eevee", "133");
    kvdb_close(&db);
	return NULL;
}

void* test2(){
	/*printf("FA2\n");
	kvdb_t db;
	kvdb_open(&db, "a.db"); 
    kvdb_put(&db, "leafeon", "470");
    kvdb_close(&db);*/
	return NULL;
}

int main() {
  pthread_t p1,p2;
  pthread_create(&p1,NULL,test1,NULL);
  pthread_create(&p2,NULL,test2,NULL);
  pthread_join(p1,NULL);
  pthread_join(p2,NULL);
  /*kvdb_t db;
  const char *key = "operating-systems";
  char *value1,*value2,*value3,*value4;

  kvdb_open(&db, "a.db"); // BUG: should check for errors  
  kvdb_put(&db, key, "three-easy-pieces");
  kvdb_put(&db, "leafeon", "470");
  kvdb_put(&db, "leafeon", "471");
  value1 = kvdb_get(&db, key);
  value2 = kvdb_get(&db, "leafeon");
  value3 = kvdb_get(&db, "glaceon");
  kvdb_close(&db);
  value4 = kvdb_get(&db, "leafeon");
  printf("[%s]: [%s]\n", key, value1);
  printf("[%s]: [%s]\n", "leafeon", value2);
  printf("[%s]: [%s]\n", "glaceon", value3);
  printf("[%s]: [%s]\n", "leafeon", value4);
  free(value1);
  */return 0;
}
