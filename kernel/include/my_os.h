
#define DEBUG

/*#ifdef DEBUG
	#define LOG(s) printf("\33[1;35m%s\n\33[0m",s)
#else
	#define LOG(s) ((void)0)
#endif*/
#	define Log_write(format, ...) \
  do { \
    if (log_fp != NULL) { \
      fprintf(log_fp, format, ## __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0)
#else
#	define Log_write(format, ...)
#endif

#define printflog(format, ...) \
  do { \
    printf(format, ## __VA_ARGS__); \
    fflush(stdout); \
    Log_write(format, ## __VA_ARGS__); \
  } while (0)

#define LOG(format, ...) \
    printflog("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)


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
