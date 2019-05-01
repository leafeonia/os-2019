#include "lock.h"

#define DEBUG

#ifdef DEBUG
	#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)
#else
	#define LOG(s) ((void)0)
#endif
