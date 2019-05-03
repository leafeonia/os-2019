#include "lock.h"

#define DEBUG

#ifdef DEBUG
	#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)
#else
	#define LOG(s) ((void)0)
#endif

#define Assert(cond) \
	do { \
		if (!(cond)) { \
			printf("Assertion fails at %s:%d\n",__FILE__, __LINE__);\
			_halt(1);\
		}\
	} while(0)

#define panic(format, ...) \
  do { \
    printf("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)

#define INT_MAX 2147483647
#define INT_MIN -2147483648
#define MAGIC1 0x123456789012345
#define MAGIC2 0x987654321098765
