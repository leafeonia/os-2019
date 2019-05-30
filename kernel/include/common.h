#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#define NR_FILE 20

struct file;
struct task {
	const char* name;
	_Context context;
	long long fence1;
	uint8_t stack[4096];
	struct file fildes[NR_FILE];
	long long fence2;
};
struct spinlock {
	const char* name;
	int locked;
	int cpu;
};
struct semaphore {
	const char* name;
	struct spinlock lock;
	int value;
};

struct inodeops;
struct inode;

typedef struct file {
  int refcnt; // 引用计数
  struct inode *inode;
  uint64_t offset;
}file_t;

typedef struct inodeops {
  int (*open)(file_t *file, int flags);
  int (*close)(file_t *file);
  ssize_t (*read)(file_t *file, char *buf, size_t size);
  ssize_t (*write)(file_t *file, const char *buf, size_t size);
  off_t (*lseek)(file_t *file, off_t offset, int whence);
  int (*mkdir)(const char *name);
  int (*rmdir)(const char *name);
  int (*link)(const char *name, struct inode *inode);
  int (*unlink)(const char *name);
  // 你可以自己设计readdir的功能
}inodeops_t;

typedef unsigned int dev_t;

typedef struct inode {
  int refcnt;
  void *ptr;       // private data
  filesystem_t *fs;
  inodeops_t *ops; // 在inode被创建时，由文件系统的实现赋值
                   // inode ops也是文件系统的一部分

}inode_t;



typedef struct fsops {
  void (*init)(filesystem_t *fs, const char *name, dev_t *dev);
  inode_t *(*lookup)(filesystem_t *fs, const char *path, int flags);
  int (*close)(inode_t *inode);
}fsops_t;

typedef struct filesystem {
  fsops_t *ops;
  dev_t *dev;
}filesystem_t;
#endif
