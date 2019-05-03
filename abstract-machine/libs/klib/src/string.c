#include "klib.h"
#include "assert.h"
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t ans = 0;
	while(*s){
		s++;
		ans++;
	}
    return ans;
}

char *strcpy(char* dst,const char* src) {
	char* temp = dst;
	while(*src) *dst++ = *src++; 
	*dst = '\0';
	return temp;
}

char* strncpy(char* dst, const char* src, size_t n) {
  return NULL;
}

char* strcat(char* dst, const char* src) {
    char* temp = dst;
	while(*dst) dst++;
	while(*src) *dst++ = *src++;
	*dst = '\0';
	return temp;
}

int strcmp(const char* s1, const char* s2) {  
  while(*s1 && *s2 && (*s1 == *s2)){
    s1++;
	s2++;
  }
  if(*s1 > *s2) return 1;
  else if(*s1 < *s2) return -1;
  else return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  return 0;
}

void* memset(void* v,int c,size_t n) {
	char* temp = (char*) v;
	for(int i = 0;i < n;++i){
		*temp++ = c;
	}
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  void* origin = out;
  char* outc = (char*) out;
  char* inc = (char*) in;
  while(n--) *outc++ = *inc++;
  return origin;
}

void* memmove(void* dest, void* src, size_t n){
	return memcpy(dest,src,n);
}

int memcmp(const void* s1, const void* s2, size_t n){
	char* temps1 = (char*) s1;
	char* temps2 = (char*) s2;
	n-=1;
	while(*temps1 && *temps2 && (n--) && (*temps1 == *temps2)){
		temps1++;
		temps2++;
	}
	if(*temps1 > *temps2) return 1;
	else if (*temps1 < *temps2)return -1;
	else return 0;
}

#endif
