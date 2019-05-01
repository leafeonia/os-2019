#include "lock.h"

#define DEBUG

#ifdef DEBUG
	#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)
#else
	#define LOG(s) ((void)0)
#endif

#define Assert(cond) \
	do { \
		if (!cond) { \
			printf("Assertion fails at %s:%d\n",__FILE__, __LINE__);\
			_halt(1);\
		}\
		while(0)\
	}
