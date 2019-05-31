#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

static filesystem_t* blkfs;
static fsops_t* blkfs_ops;
struct mount_point{
	char* path;
	filesystem_t* fs;
};
static struct mount_point mt_list[5];
static int mt_idx = 0;
static spinlock_t lk_vfs;

void vfs_init_wrapped(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
}

void vfs_init(){
	GOLDLOG("hello L3!");
	blkfs = pmm->alloc(sizeof(filesystem_t));
	blkfs_ops = pmm->alloc(sizeof(fsops_t));
	blkfs->ops = blkfs_ops;
	device_t *dev = dev_lookup("ramdisk0");
	blkfs_ops->init = vfs_init_wrapped;
	blkfs->ops->init(blkfs,"blkfs",dev);
	kmt->spin_init(lk_vfs);
	vfs_mount("/",blkfs);
	
}
int vfs_access(const char *path, int mode){
	return 0;
}
int vfs_mount(const char *path, filesystem_t *fs){
	kmt->spin_lock(&lk_vfs);
	mt_list[mt_idx].path = path;
	mt_list[mt_idx++].fs = fs;
	for(int i = 0;i < mt_idx;i++){
		for(int j = i + 1;j < mt_idx){
			if(strlen(mt_list[i].path) > strlen(mt_list[j].path)){
				mount_point temp = mt_list[i];
				mt_list[i] = mt_list[j];
				mt_list[j] = temp;
			}
		}
	}
	kmt->spin_unlock(&lk_vfs);
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
	if(strncmp(path,"/proc",5) == 0){
		printf("proc\n");
	}
	else if(strncmp(path,"/dev",4) == 0){
		printf("dev\n");
	}
	else if(strncmp(path,"/",1) == 0){
		printf("blockfs\n");
	}
	for(int i = 0;i <= mt_idx;i++){
		if(i == mt_idx) panic("filesystem not found\n");
		if(strncmp(path,mt_list[i].path,strlen(mt_list[i].path)) == 0){
			printf("%s\n",mt_list[i].path);
		}
	}
	return 0;
}
ssize_t vfs_read(int fd, void *buf, size_t nbyte){
	return 0;
}
ssize_t vfs_write(int fd, void *buf, size_t nbyte){
	return 0;
}
off_t vfs_lseek(int fd, off_t offset, int whence){
	return 0;
}
int vfs_close(int fd){
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
