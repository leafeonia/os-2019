#include <common.h>
#include "vfs.h"
#include <klib.h>
#include <devices.h>
#include "my_os.h"
/*
static inodeops_t* proc_inode_ops;
static inode_t* procfs_inode[10];

int boom(){
	LOG("procfs doesn't support this function");
	return -1;
}

static int dummy(){return 0;}
*/
void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
	procfs_ops = pmm->alloc(sizeof(fsops_t));
	procfs->ops = procfs_ops;
	extern task_t* tasks[16][NR_TASK];
	for(int i = 0;i < 16;i++){
		for(int j = 0;j < NR_TASK;j++){
			task_t* cur = tasks[i][j];
			if(cur->fence1 == MAGIC1) printf("%d %d: %s\n",i,j,cur->name);
		}
	}

	
	
	/*
	proc_inode_ops = pmm->alloc(sizeof(inodeops_t));
	proc_inode_ops->open   = proc_inode_open;
	proc_inode_ops->close  = proc_inode_close;
	proc_inode_ops->read   = proc_inode_read;
	proc_inode_ops->write  = proc_inode_write;
	proc_inode_ops->lseek  = dummy;
	proc_inode_ops->mkdir  = boom;
	proc_inode_ops->rmdir  = boom;
	proc_inode_ops->link   = boom;
	proc_inode_ops->unlink = boom;*/
}
