#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#define NR_FILE 20
#define NR_TASK 21
#define O_CREAT 0x10
#define O_DIRE 0x20
#define O_RDONLY 0x40

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


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

typedef struct inodeops inodeops_t;
typedef struct inode inode_t;

typedef struct file {
  int refcnt; // 引用计数
  struct inode *inode;
  uint64_t offset;
}file_t;

struct task {
	const char* name;
	_Context context;
	long long fence1;
	uint8_t stack[200000];
	long long fence2;
	file_t *fildes[NR_FILE];
};

struct inodeops{
  int (*open)(file_t *file, int flags, inode_t* inode);
  int (*close)(file_t *file);
  ssize_t (*read)(file_t *file, char *buf, size_t size);
  ssize_t (*write)(file_t *file, const char *buf, size_t size);
  off_t (*lseek)(file_t *file, off_t offset, int whence);
  int (*mkdir)(const char *name, inode_t* inode);
  int (*rmdir)(const char *name, inode_t* inode);
  int (*link)(const char *name, inode_t *inode);
  int (*unlink)(const char *name);
  // 你可以自己设计readdir的功能
};


struct inode{
  int refcnt;
  void *ptr;       // private data
  filesystem_t *fs;
  inodeops_t *ops; // 在inode被创建时，由文件系统的实现赋值
                   // inode ops也是文件系统的一部分
  int mode;
  int block[11];       //block number in blkfs
};



typedef struct fsops {
  void (*init)(filesystem_t *fs, const char *name, device_t *dev);
  inode_t *(*lookup)(filesystem_t *fs, const char *path, int flags);
  int (*close)(inode_t *inode);
}fsops_t;

typedef struct filesystem {
  const char* name;
  fsops_t *ops;
  device_t *dev;
}filesystem_t;
#endif
