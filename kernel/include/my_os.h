
//#define DEBUG
#define ERR_LOG

#ifdef ERR_LOG
	#define LOG(...) printf("\033[1;35m");printf(__VA_ARGS__);printf("\033[0m\n"); 
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

#ifdef DEBUG
	#define GOLDLOG(...) printf("\033[1;33m");printf(__VA_ARGS__);printf("\033[0m\n");
#else
	#define GOLDLOG(...) ((void)0) 
#endif

#ifdef DEBUG
	#define CYANLOG(...) printf("\033[36m");printf(__VA_ARGS__);printf("\033[0m\n");
#else
	#define CYANLOG(...) ((void)0) 
#endif

