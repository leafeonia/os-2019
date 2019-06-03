#include <common.h>
#include "vfs.h"
#include <klib.h>

#define BLOCK_SIZE 4096
#define NR_INODE 64

static inodeops_t* blk_inode_ops;

static inode_t inodes[NR_INODE];
static char data_bitmap[BLOCK_SIZE];
//****************************************
//| INODES  |  DATA BITMAP |  DATA ...
//****************************************
//  BLOCK 0       BLOCK 1    BLOCK 2 - 999
void blkfsops_init(filesystem_t *fs, const char *name, device_t *dev){
	assert(sizeof(inode_t) == 16); //16 * NR_INODE = BLOCK_SIZE
	for(int i = 0;i < NR_INODE;i++){
		inodes[i]->refcnt = 0;
		inodes[i]->ptr = NULL;
		inodes[i]->fs = fs;
		inodes[i]->ops = blk_inode_ops;
	}
	memset(data_bitmap,0,sizeof(data_bitmap));
}

inode_t* blkfsops_lookup(filesystem_t *fs, const char *path, int flags){
	return NULL;
}

int blkfsops_close(inode_t *inode){
	return 0;
}

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;
	printf("sizeof(inode_t) = %d\n",sizeof(inode_t));
	
	
	//initialize inodeops of inode of blkfs.
	blk_inode_ops = pmm->alloc(sizeof(inodeops_t));
	
	/*
	never used.
	
	blk_inode_ops->open   = 
	blk_inode_ops->close  = 
	blk_inode_ops->lseek  = 
	*/
	
	/*
	blk_inode_ops->read   = dev_inode_read;
	blk_inode_ops->write  = dev_inode_write;
	
	blk_inode_ops->mkdir  = boom;
	blk_inode_ops->rmdir  = boom;
	blk_inode_ops->link   = boom;
	blk_inode_ops->unlink = boom;
	*/
	
	blkfs_ops = pmm->alloc(sizeof(fsops_t));
	blkfs->ops = blkfs_ops;
	//rd_t* rd = dev->ptr;
	//dev->ops->write()	
	
	blkfs_ops->init = blkfsops_init;
	blkfs_ops->lookup = blkfsops_lookup;
	blkfs_ops->close = blkfsops_close;
	
	blkfs_ops->init(fs, name, dev);
}
