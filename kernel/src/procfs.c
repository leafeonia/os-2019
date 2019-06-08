#include <common.h>
#include "vfs.h"
#include <klib.h>
#include <devices.h>
#include "my_os.h"

static inodeops_t* proc_inode_ops;
static inode_t procfs_inode;

static int boom(){
	LOG("procfs doesn't support this function");
	return -1;
}

static int dummy(){return 0;}
static void dummy2(){}

inode_t* procfsops_lookup(filesystem_t *fs, const char *path, int flags){
	GOLDLOG("procfsops_lookup: path = %s",path);
	const char* left_path = path + 1;
	if(strcmp(left_path,".") == 0) procfs_inode.block[0] = 1;
	else if(strcmp(left_path,"cpuinfo") == 0) procfs_inode.block[0] = 2;
	else if(strcmp(left_path,"meminfo") == 0) procfs_inode.block[0] = 3;
	return &procfs_inode;
}

ssize_t proc_inode_read(file_t *file, char *buf, size_t size){
	int code = file->inode->block[0];
	if(code == 1) sprintf(buf, "ls of procfs");
	else if(code == 2) sprintf(buf, "cpuinfo: %d",_ncpu());
	else if(code == 3) sprintf(buf, "meminfo");
	file->offset += size;
	return size;
}

void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
	procfs_ops = pmm->alloc(sizeof(fsops_t));
	fs->ops = procfs_ops;
	extern task_t* tasks[16][NR_TASK];
	for(int i = 0;i < 16;i++){
		for(int j = 0;j < NR_TASK;j++){
			task_t* cur = tasks[i][j];
			if(cur->fence1 == MAGIC1) printf("%d %d: %s\n",i,j,cur->name);
		}
	}
	
	procfs_ops->init   = dummy2;
	procfs_ops->lookup = procfsops_lookup;
	procfs_ops->close  = dummy;

	procfs_inode.ptr = NULL;
	procfs_inode.refcnt = 0;
	procfs_inode.fs = procfs;
	procfs_inode.ops = proc_inode_ops;
	for(int i = 0;i < 12;i++) procfs_inode.block[i] = 0;
	
	
	proc_inode_ops = pmm->alloc(sizeof(inodeops_t));
	/*proc_inode_ops->open   = proc_inode_open;
	proc_inode_ops->close  = proc_inode_close;
	proc_inode_ops->read   = proc_inode_read;
	proc_inode_ops->write  = proc_inode_write;*/
	proc_inode_ops->lseek  = dummy;
	proc_inode_ops->mkdir  = boom;
	proc_inode_ops->rmdir  = boom;
	proc_inode_ops->link   = boom;
	proc_inode_ops->unlink = boom;
}
