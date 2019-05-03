#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>

struct task {
	const char* name;
	_Context context;
	long long fence1;
	uint8_t stack[4096];
	long long fence2;
};
struct spinlock {};
struct semaphore {};

#endif
