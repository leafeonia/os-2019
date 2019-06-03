#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"
#include "vfs.h"



inode_t* blkfsops_lookup(filesystem_t *fs, const char *path, int flags){
	
	return NULL;
}

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;
	//rd_t* rd = dev->ptr;
	//dev->ops->write()	
}

void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
}






int boom(){
	LOG("The file system doesn't support this function");
	return -1;
}


int dev_inode_open(file_t *file, int flags, inode_t* inode){
	file->inode = inode;
	return 0;
}

ssize_t dev_inode_write(file_t *file, const char *buf, size_t size){
	LOG("YEAH");
	device_t* dev = file->inode->ptr;
	//printf("inode = 0x%x, dev = 0x%x, dev->name = %s, \n",file->inode, dev, dev->name);
	/*extern device_t* devices[8];
	for(int i = 0;i < 8;i++){
		printf("devices. id = %d, ptr = 0x%x, name = %s\n",devices[i]->id,devices[i]->ptr,devices[i]->name);
	}*/
	char text[] = "FA";
	dev->ops->write(dev, 0, text, strlen(text));
	//device_t* dev2 = dev_lookup("tty2");
	//dev2->ops->write(dev2, 0, text, strlen(text));
	return 0;
}


inode_t* devfsops_lookup(filesystem_t *fs, const char *path, int flags){
	if(path[0] == '/') path = path + 1;
	//printf("Welcome to devfs_lookup. dev_name = %s\n",path);
	//device_t* dev = dev_lookup(path);
	char* devices_tmp[] = {"ramdisk0","ramdisk1","input","fb","tty1","tty2","tty3","tty4"};
	int id = 0;
	for(;id < 8;id++){
		if(strcmp(devices_tmp[id],path) == 0) break;
	}
	if(id == 8){
		LOG("devfs: lookup device fails.");
		return NULL;
	}
	return devfs_inode[id];
}

void devfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
	
	
	//initialize inodeops of inode of devfs.
	dev_inode_ops = pmm->alloc(sizeof(inodeops_t));
	dev_inode_ops->open = dev_inode_open;
	dev_inode_ops->write = dev_inode_write;
	
	
	
	
	
	//initialize fsops of devfs.
	devfs_ops = pmm->alloc(sizeof(fsops_t));
	devfs->ops = devfs_ops;
	
	//init.
	extern device_t* devices[8];
	/*for(int i = 0;i < 8;i++){
		printf("devices. id = %d, ptr = 0x%x, name = %s\n",devices[i]->id,devices[i]->ptr,devices[i]->name);
	}*/
	for(int i = 0;i < 8;i++){
		devfs_inode[i] = pmm->alloc(sizeof(fsops_t));
		devfs_inode[i]->ptr = devices[i];
		devfs_inode[i]->ops = dev_inode_ops;
		devfs_inode[i]->fs = devfs;
		//printf("%d: ptr = 0x%x, ops = 0x%x,devfs_inode[%d] = 0x%x\n",i,devfs_inode[i]->ptr,devfs_inode[i]->ops,i ,devfs_inode[i]);
	}
	
	
	
	//lookup
	devfs_ops->lookup = devfsops_lookup;
	
	//close (omit)
	devfs_ops->close = boom;
	
	
	
	
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
	blkfs_ops = pmm->alloc(sizeof(fsops_t));
	blkfs->ops = blkfs_ops;
	device_t *dev = dev_lookup("ramdisk0");
	blkfs_ops->init = blkfs_init;
	blkfs->ops->init(blkfs,"blkfs",dev);
	
	devfs = pmm->alloc(sizeof(filesystem_t));
	devfs_ops->init = devfs_init;
	devfs->ops->init(devfs,"devfs",NULL);
	
	procfs = pmm->alloc(sizeof(filesystem_t));
	procfs_ops = pmm->alloc(sizeof(fsops_t));
	procfs->ops = procfs_ops;
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
	(*cur)->fildes[fd] = file;
	//printf("return fd = %d\n",fd);
	inode->ops->open(file, flags, inode);
	//printf("fa");
	kmt->spin_unlock(&lk_vfs);
	return fd;
}
ssize_t vfs_read(int fd, void *buf, size_t nbyte){
	return 0;
}
ssize_t vfs_write(int fd, void *buf, size_t nbyte){
	kmt->spin_lock(&lk_vfs);
//printf("fd = %d\n",fd);
	extern task_t** current_task[16];
	task_t** cur = current_task[_cpu()];
	file_t* file = (*cur)->fildes[fd];
//LOG("BEST MOM");
	file->inode->ops->write(file, buf, nbyte);
//LOG("BEST KID");
	kmt->spin_unlock(&lk_vfs);
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
