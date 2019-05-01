#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>

struct task {
	int id;
	uint8_t fence1[32];
	uint8_t stack[4096];
	uint8_t fence2[32];
};
struct spinlock {};
struct semaphore {};

#endif
