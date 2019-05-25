#include "kvdb.h"
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <unistd.h>
#include <string.h>
void* test1(void* arg){
	printf("FA1\n");
	kvdb_t db,db2;
	db.id = db2.id = *(int*)arg;
	kvdb_open(&db, "a.db"); 
    kvdb_put(&db, "operating systems", "three-easy-pieces");
    kvdb_put(&db, "glaceon", "471");
    kvdb_put(&db, "flareon", "136");
    printf("\033[36m[%d]%s - %s(should return 471)\033[0m\n",db.id,"glaceon",kvdb_get(&db, "glaceon"));
    printf("\033[36m[%d]%s - %s(should error no key)\033[0m\n",db.id,"leafeon",kvdb_get(&db, "leafeon"));
    printf("\033[36m[%d]%s - %s(should return three-easy-pieces)\033[0m\n",db.id,"operating systems",kvdb_get(&db, "operating systems"));
    kvdb_close(&db);
    printf("\033[36m[%d]%s - %s(should error not open)\033[0m\n",db.id,"glaceon",kvdb_get(&db, "glaceon"));
    kvdb_open(&db, "b.db");
    kvdb_put(&db, "eevee", "132");
    kvdb_put(&db, "vaporeon", "134");
    kvdb_open(&db2, "c.db");
    printf("\033[36m[%d]%s - %s(should error no key)\033[0m\n",db.id,"FA",kvdb_get(&db2, "FA"));
    kvdb_put(&db2, "FA", "Van");
    printf("\033[36m[%d]%s - %s(should return Van)\033[0m\n",db.id,"FA",kvdb_get(&db2, "FA"));
    kvdb_close(&db2);
    kvdb_put(&db, "eevee", "133");
    printf("\033[36m[%d]%s - %s(should error no key)\033[0m\n",db.id,"glaceon",kvdb_get(&db, "glaceon"));
    printf("\033[36m[%d]%s - %s(should return 133)\033[0m\n",db.id,"eevee",kvdb_get(&db, "eevee"));
    kvdb_close(&db);
    kvdb_close(&db2);
	return NULL;
}

void* test2(void* arg){
	printf("FA2\n");
	kvdb_t db;
	db.id = *(int*)arg;
	kvdb_open(&db, "a.db"); 
    if(db.id == 2)kvdb_put(&db, "math", "161");
    else kvdb_put(&db, "math", "175");
    printf("\033[36m[%d]%s - %s([2]:should return 161,[4]should return 175)\033[0m\n",db.id,"math",kvdb_get(&db, "math"));
    kvdb_close(&db);
	return NULL;
}

int main(int argc, char** argv) {
	if(argc == 1) printf("bingo");
  		
	int rc = fork();
	if(rc < 0) printf("error");
	else if(rc == 0){
		pthread_t p1,p2;
		int one = 1;
		int two = 2;
  		pthread_create(&p1,NULL,test1,(void*)&one);
  		pthread_create(&p2,NULL,test2,(void*)&two);
  		pthread_join(p1,NULL);
  		pthread_join(p2,NULL);
	}
	else{
		pthread_t p1,p2;
		int three = 3;
		int four = 4;
  		pthread_create(&p1,NULL,test1,(void*)&three);
  		pthread_create(&p2,NULL,test2,(void*)&four);
  		pthread_join(p1,NULL);
  		pthread_join(p2,NULL);
	}
  
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
