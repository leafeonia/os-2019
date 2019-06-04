#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"
#include "vfs.h"

static struct mount_point mt_list[5];
static int mt_idx = 0;
static spinlock_t lk_vfs;


file_t* fd2file(int fd){
	extern task_t** current_task[16];
	task_t** cur = current_task[_cpu()];
	file_t* ret = (*cur)->fildes[fd];
	assert(ret);
	return ret;
}

int vfs_mount(const char *path, filesystem_t *fs){
	kmt->spin_lock(&lk_vfs);
	mt_list[mt_idx].path = path;
	mt_list[mt_idx++].fs = fs;
	for(int i = 0;i < mt_idx;i++){
		for(int j = i + 1;j < mt_idx;j++){
			if(strlen(mt_list[i].path) < strlen(mt_list[j].path)){
				struct mount_point temp = mt_list[i];
				mt_list[i] = mt_list[j];
				mt_list[j] = temp;
			}
		}
	}
	kmt->spin_unlock(&lk_vfs);
	return 0;
}

void vfs_init(){
	GOLDLOG("hello L3!");
	//printf("size: %d\n\n",sizeof(task_t));
	
	
	blkfs = pmm->alloc(sizeof(filesystem_t));
	device_t *dev = dev_lookup("ramdisk1");
	blkfs_ops->init = blkfs_init;
	blkfs->ops->init(blkfs,"blkfs",dev);
	
	devfs = pmm->alloc(sizeof(filesystem_t));
	devfs_ops->init = devfs_init;
	devfs->ops->init(devfs,"devfs",NULL);
	
	procfs = pmm->alloc(sizeof(filesystem_t));
	procfs_ops->init = procfs_init;
	procfs->ops->init(procfs,"procfs",NULL);
	
	
	kmt->spin_init(&lk_vfs,"lk_vfs");
	vfs_mount("/",blkfs);
	vfs_mount("/dev",devfs);
	vfs_mount("/proc",procfs);
	
}
int vfs_access(const char *path, int mode){
	return 0;
}

int vfs_unmount(const char *path){
	return 0;
}
int vfs_mkdir(const char *path){
	return 0;
}
int vfs_rmdir(const char *path){
	return 0;
}
int vfs_link(const char *oldpath, const char *newpath){
	return 0;
}
int vfs_unlink(const char *path){
	return 0;
}
int vfs_open(const char *path, int flags){
	kmt->spin_lock(&lk_vfs);
	/*if(strncmp(path,"/proc",5) == 0){
		printf("proc\n");
	}
	else if(strncmp(path,"/dev",4) == 0){
		printf("dev\n");
	}
	else if(strncmp(path,"/",1) == 0){
		printf("blockfs\n");
	}*/
	filesystem_t* fs = NULL;
	int omit = 0; //omit the part of path that is registered in mount point
	for(int i = 0;i <= mt_idx;i++){
		if(i == mt_idx) panic("filesystem not found\n");
		//printf("%s %s %d\n",path,mt_list[i].path,strlen(mt_list[i].path));
		if(strncmp(path,mt_list[i].path,strlen(mt_list[i].path)) == 0){
			omit = strlen(mt_list[i].path);
			//printf("found: %s\n",mt_list[i].path);
			fs = mt_list[i].fs;
			break;
		}
	}
	const char* fs_path = path + omit;
	//GOLDLOG(fs_path);
	inode_t* inode = fs->ops->lookup(fs,fs_path,flags);
	if(!inode){
		LOG("vfs->open(%s, %d) fails", path, flags);
		kmt->spin_unlock(&lk_vfs);
		return -1;
	}
	inode->refcnt++;
	//printf("inode->ptr = 0x%x\n",inode->ptr);
	extern task_t** current_task[16];
	task_t** cur = current_task[_cpu()];
	int fd = -1;
	/*printf("in vfs. %s: %x\n",(*cur)->name,(*cur)->fildes);
	for(int i = 3;i < 20;i++){
		//printf("%d: %x\n",i,(*cur)->fildes[i]);
		file_t* tmp = (*cur)->fildes[i];
		printf("%d:*%x %x*\n",i,&(*cur)->fildes[i],tmp);
	}*/
	for(int i = 3;i < 20;i++){
		//printf("%d: %x\n",i,(*cur)->fildes[i]);
		file_t* tmp = (*cur)->fildes[i];
		//printf("%d:*%x*\n",i,tmp);
		if(tmp == NULL){
			fd = i;
			break;
		}
	}
	if(fd == -1) panic("no available fd(MAXIMUM 20)\n");
	file_t* file = pmm->alloc(sizeof(file_t));
	file->refcnt = 1;
	file->offset = 0;
	file->inode = inode;
	(*cur)->fildes[fd] = file;
	//printf("return fd = %d\n",fd);
	//inode->ops->open(file, flags, inode);
	kmt->spin_unlock(&lk_vfs);
	return fd;
}
ssize_t vfs_read(int fd, void *buf, size_t nbyte){
	file_t* file = fd2file(fd);
	if(!file->inode){
		LOG("error: current file has been closed. Read fails.");
		return 1;
	}
	return file->inode->ops->read(file, buf, nbyte);
}
ssize_t vfs_write(int fd, void *buf, size_t nbyte){
	file_t* file = fd2file(fd);
	if(!file->inode){
		LOG("error: current file has been closed. Write fails");
		return 1;
	}
	//kmt->spin_lock(&lk_vfs);
	
	
	return file->inode->ops->write(file, buf, nbyte);
	//kmt->spin_unlock(&lk_vfs);
}
off_t vfs_lseek(int fd, off_t offset, int whence){
	file_t* file = fd2file(fd);
	file->offset = offset;
	return 0;
}
int vfs_close(int fd){
	file_t* file = fd2file(fd);
	file->offset = 0;
	file->inode->refcnt--;
	//LOG("file->inode->block[0] = %d",file->inode->block[0]);
	file->inode->fs->ops->close(file->inode); //devfs: do nothing   blkfs: free the inode
	file->inode = NULL;
	return 0;
}


MODULE_DEF(vfs) {
  .init   = vfs_init,
  .access = vfs_access,
  .mount = vfs_mount,
  .unmount = vfs_unmount,
  .mkdir = vfs_mkdir,
  .rmdir = vfs_rmdir,
  .link = vfs_link,
  .unlink = vfs_unlink,
  .open = vfs_open,
  .read = vfs_read,
  .write = vfs_write,
  .lseek = vfs_lseek,
  .close = vfs_close,
};
