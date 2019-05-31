#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"

static filesystem_t* blkfs;
static fsops_t* blkfs_ops;

void vfs_init_wrapped(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
}

void vfs_init(){
	GOLDLOG("hello L3!");
	blkfs = pmm->alloc(sizeof(filesystem));
	blkfs->ops = blkfs_ops;
	printf("??");
	device_t *dev = dev_lookup("ramdisk0");
	printf("?");
	blkfs_ops->init = vfs_init_wrapped;
	blkfs->ops->init(blkfs,"blkfs",dev);
	
}
int vfs_access(const char *path, int mode){
	return 0;
}
int vfs_mount(const char *path, filesystem_t *fs){
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
