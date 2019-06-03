#include <common.h>
#include <klib.h>
#include <devices.h>
#include "my_os.h"
#include "vfs.h"

static spinlock_t lk_dev_inode_ops;
static inodeops_t* dev_inode_ops;
static inode_t* devfs_inode[10];

int boom(){
	LOG("The file system doesn't support this function");
	return -1;
}

int dev_inode_open(file_t *file, int flags, inode_t* inode){
	kmt->spin_lock(&lk_dev_inode_ops);
	file->inode = inode;
	kmt->spin_unlock(&lk_dev_inode_ops);
	return 0;
}

ssize_t dev_inode_write(file_t *file, const char *buf, size_t size){
	kmt->spin_lock(&lk_dev_inode_ops);
	//LOG("YEAH");
	device_t* dev = file->inode->ptr;
	//printf("inode = 0x%x, dev = 0x%x, dev->name = %s, dev->ops = 0x%x \n",file->inode, dev, dev->name, dev->ops);
	/*extern device_t* devices[8];
	for(int i = 0;i < 8;i++){
		printf("devices. id = %d, ptr = 0x%x, name = %s\n",devices[i]->id,devices[i]->ptr,devices[i]->name);
	}*/
	dev->ops->write(dev, 0, buf, strlen(buf));
	//LOG("YEAH2");
	//device_t* dev2 = dev_lookup("tty2");
	//dev2->ops->write(dev2, 0, text, strlen(text));
	kmt->spin_unlock(&lk_dev_inode_ops);
	return 0;
}


inode_t* devfsops_lookup(filesystem_t *fs, const char *path, int flags){
	kmt->spin_lock(&lk_dev_inode_ops);
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
	kmt->spin_unlock(&lk_dev_inode_ops);
	return devfs_inode[id];
}



void devfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
	
	kmt->spin_init(&lk_dev_inode_ops,"lk_dev_inode_ops");
	
	
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


