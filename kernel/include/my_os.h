
#define DEBUG

#ifdef DEBUG
	#define LOG(...) printf(__VA_ARGS__); 
#else
	#define LOG(...) ((void)0)
#endif

#define Assert(cond) \
	do { \
		if (!(cond)) { \
			printf("Assertion fails at %s:%d\n",__FILE__, __LINE__);\
			_halt(1);\
		}\
	} while(0)

#define panic(s) \
  do { \
    printf("\33[1;31msystem panic: %s\33[0m\n",s); \
    _halt(1); \
  } while (0)

#define INT_MAX 2147483647
#define INT_MIN -2147483648
#define MAGIC1 0x123456789012345
#define MAGIC2 0x987654321098765

#define GOLDLOG(s) printf("\33[1;33m%s\n\33[0m",s)
