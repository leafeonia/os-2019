#include <stdio.h>
#include <kvdb.h>
int main() {
  kvdb_t db;
  const char *key = "operating-systems";
  char *value1,*value2,*value3,*value4;
  kvdb_open(&db, "a.db"); // BUG: should check for errors  
  value1 = kvdb_get(&db, key);
  //value2 = kvdb_get(&db, "leafeon");
  value3 = kvdb_get(&db, "glaceon");
  kvdb_close(&db);
  //value4 = kvdb_get(&db, "leafeon");
  printf("[%s]: [%s]\n", key, value1);
  //printf("[%s]: [%s]\n", "leafeon", value2);
  printf("[%s]: [%s]\n", "glaceon", value3);
  //printf("[%s]: [%s]\n", "leafeon", value4);
  free(value1);
  //free(value2);
  free(value3);
  //free(value4);
  return 0;
}
