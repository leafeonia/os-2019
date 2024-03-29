#include <common.h>
#include "vfs.h"
#include <klib.h>
#include <devices.h>
#include "my_os.h"


#define PID(i,j) (j*_ncpu() + i - 3)
static inodeops_t* proc_inode_ops;
static inode_t procfs_inode;
extern task_t* tasks[16][NR_TASK];

static int boom(){
	LOG("procfs doesn't support this function");
	return -1;
}

static int dummy(){return 0;}
static void dummy2(){}

inode_t* procfsops_lookup(filesystem_t *fs, const char *path, int flags){
	GOLDLOG("procfsops_lookup: path = %s",path);
	if(flags & O_DIRE) {
		LOG("no directory in procfs");
		return NULL;
	}
	if(flags & O_CREAT){
		LOG("procfs is read-only");
		return NULL;
	}
	const char* left_path = path + 1;
	if(strcmp(left_path,".") == 0) procfs_inode.block[0] = 1;
	else if(strcmp(left_path,"cpuinfo") == 0) procfs_inode.block[0] = 2;
	else if(strcmp(left_path,"meminfo") == 0) procfs_inode.block[0] = 3;
	else {
		procfs_inode.block[0] = 0;
		int found = 0;
		for(int i = 0;i < 16;i++){
			for(int j = 0;j < NR_TASK;j++){
				char temp[4];
				sprintf(temp,"%d",PID(i,j));
				if(strcmp(left_path, temp) == 0){
					procfs_inode.block[1] = i;
					procfs_inode.block[2] = j;
					found = 1;
					break;
				}
			}
			if(found) break;
		}
		if(!found){
			LOG("procfs lookup fails: %s not found",path);
			return NULL;
		}
		
		
	}
	return &procfs_inode;
}

ssize_t proc_inode_read(file_t *file, char *buf, size_t size){
	int code = file->inode->block[0];
	//GOLDLOG("code = %d",code);
	if(code == 1) {
		int cnt = 0;
		dire_t* dir = (dire_t*)buf;
		for(int i = 0;i < 16;i++){
			for(int j = 0;j < NR_TASK;j++){
				task_t* cur = tasks[i][j];
				if(cur->fence1 == MAGIC1) {
					if(strncmp(cur->name,"idle",4) == 0) continue;
					sprintf(dir[cnt].name,"%d",PID(i,j));
					dir[cnt++].inode_id = 1;
				}
			}
		}
		sprintf(dir[cnt].name,"cpuinfo");
		dir[cnt++].inode_id = 1;
		sprintf(dir[cnt].name,"meminfo");
		dir[cnt++].inode_id = 1;
		/*for(int i = 0;i < 10;i++){
			printf("%d - name: %s\n",i,dir[i].name );
		}*/
	}
	else if(code == 2) sprintf(buf, "cpuinfo: %d cpu(s) working",_ncpu());
	else if(code == 3) {
		extern uintptr_t pm_start;
		sprintf(buf, "meminfo: heap bound is now at 0x%x", pm_start);
	}
	else if(code == 0){
		int j = file->inode->block[2];
		int i = file->inode->block[1];
		sprintf(buf,"pid: %d\ntask_name: %s",PID(i,j), tasks[i][j]->name);
	}
	file->offset += size;
	return size;
}

ssize_t proc_inode_write(file_t *file, const char *buf, size_t size){
	return 0;
}

void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
	procfs_ops = pmm->alloc(sizeof(fsops_t));
	proc_inode_ops = pmm->alloc(sizeof(inodeops_t));
	fs->ops = procfs_ops;
	
	
	procfs_ops->init   = dummy2;
	procfs_ops->lookup = procfsops_lookup;
	procfs_ops->close  = dummy;

	procfs_inode.ptr = NULL;
	procfs_inode.refcnt = 0;
	procfs_inode.fs = procfs;
	procfs_inode.ops = proc_inode_ops;
	procfs_inode.mode = O_RDONLY;
	for(int i = 0;i < 11;i++) procfs_inode.block[i] = 0;
	
	
	
	proc_inode_ops->open   = dummy;
	proc_inode_ops->close  = dummy;
	proc_inode_ops->read   = proc_inode_read;
	proc_inode_ops->write  = proc_inode_write;
	proc_inode_ops->lseek  = dummy;
	proc_inode_ops->mkdir  = boom;
	proc_inode_ops->rmdir  = boom;
	proc_inode_ops->link   = boom;
	proc_inode_ops->unlink = boom;
}
