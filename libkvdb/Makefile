NAME := $(shell basename $(PWD))
export TASK := M6
export LD_LIBRARY_PATH := $(PWD)
all: $(NAME)-64.so $(NAME)-32.so test-64 test-32
CFLAGS += -U_FORTIFY_SOURCE

include ../Makefile
include ../Makefile.lab

test-64: kvdb.c kvdb.h test.c $(NAME)-64.so
	gcc -Wl,-rpath=. -m64 -ggdb test.c -L. -lkvdb-64 -o $@ -lpthread 
test-32: kvdb.c kvdb.h test.c $(NAME)-32.so
	gcc -Wl,-rpath=. -m32 -ggdb test.c -L. -lkvdb-32 -o $@ -lpthread

# export LD_LIBRARY_PATH is invalid since the envp is effective to only child process of makefile. Add "-Wl,-rpath=." to solve this or execute by "LD_LIBRARY_PATH=$(pwd) ./test-32" in command line.
