#include <common.h>
#include "vfs.h"
#include <klib.h>
#include <devices.h>
#include "my_os.h"

static inodeops_t* blk_inode_ops;

static int dummy(){return 0;}

int blk_inode_open(file_t *file, int flags, inode_t* inode){
	/*kmt->spin_lock(&lk_dev_inode_ops);
	file->inode = inode;
	kmt->spin_unlock(&lk_dev_inode_ops);*/
	return 0;
}

int blk_inode_close(file_t *file){
	return 0;
}

ssize_t blk_inode_read(file_t *file, char *buf, size_t size){
	device_t* dev = file->inode->fs->dev;
	
	if(file->offset + size > BLOCK_SIZE){
		LOG("error: blkfs supports maximum file size :4KB. Current offset is out of range. Read fails");
		return -1;
	}
	ssize_t nread = dev->ops->read(dev, DATA(file->inode->block[0]) + file->offset, buf, size);
	file->offset += size;
	return nread;
}

ssize_t blk_inode_write(file_t *file, const char *buf, size_t size){
	device_t* dev = file->inode->fs->dev;
	
	if(file->offset + size > BLOCK_SIZE){
		LOG("error: blkfs supports maximum file size :4KB. Current offset is out of range. Read fails");
		return -1;
	}
	ssize_t nwrite = dev->ops->write(dev, DATA(file->inode->block[0]) + file->offset, buf, size);
	file->offset += size;
	return nwrite;
}

int blk_inode_link(const char *name, inode_t *inode){
	GOLDLOG("link: newpath = \"%s\"",name);
	return 0;
}













int get_data_offset(int inode_id){
	inode_t inode;
	blkfs->dev->ops->read(blkfs->dev, INODE(inode_id), &inode, sizeof(inode_t));
	printf("inode #%d get_data_offset: %d\n",inode_id,inode.block[0]);
	return 0;
}

int get_available_data_block(){
	unsigned char data_bitmap[BLOCK_SIZE];
	blkfs->dev->ops->read(blkfs->dev, BITMAP_OFFSET, data_bitmap, BLOCK_SIZE);
	for(int i = 0;i < NR_DATA;i++){
		if(!data_bitmap[i]) {
			GOLDLOG("get available data block #%d",i);
			data_bitmap[i] = 1;
			blkfs->dev->ops->write(blkfs->dev, BITMAP_OFFSET, data_bitmap, BLOCK_SIZE);
			return i;
		}
	}
	LOG("error: no available data block");
	return -1;
}

int get_available_inode(){
	inode_t inodes[NR_INODE];
	blkfs->dev->ops->read(blkfs->dev, 0, &inodes, BLOCK_SIZE);
	for(int i = 2;i < NR_INODE;i++){
		if(inodes[i].refcnt == 0) {
			inodes[i].refcnt = 1;
			inodes[i].block[0] = get_available_data_block();
			blkfs->dev->ops->write(blkfs->dev, 0, &inodes, BLOCK_SIZE);
			GOLDLOG("get available inode #%d",i);
			return i;
		}
	}
	LOG("error: no available inode");
	return -1;
}


//****************************************
//| INODES  |  DATA BITMAP |  DATA ...
//****************************************
//  BLOCK 0       BLOCK 1    BLOCK 2 - 999
void blkfsops_init(filesystem_t *fs, const char *name, device_t *dev){
	assert(sizeof(inode_t) * NR_INODE == BLOCK_SIZE); //64 * NR_INODE = BLOCK_SIZE
	inode_t inodes[NR_INODE];
    unsigned char data_bitmap[BLOCK_SIZE];
    
	for(int i = 0;i < NR_INODE;i++){
		inodes[i].refcnt = 0;
		inodes[i].ptr = NULL;
		inodes[i].fs = fs;
		inodes[i].ops = blk_inode_ops;
		for(int i = 0;i < 12;i++){
			inodes[i].block[i] = -1; //!!
		}
	}
	memset(data_bitmap,0,sizeof(data_bitmap));
	//printf("size = %d\n",sizeof(inode_t));

	
	
	//Initialize root directory. Data reside in block 2(data: 0)
	dire_t dire[NR_DIRE];  
	memset(dire,0,sizeof(dire));
	//printf("sizeof dire = %d\n",sizeof(dire));
	inodes[ROOT].refcnt = 1;
	inodes[ROOT].block[0] = 0; /*root directory resides in data block #0 */
	data_bitmap[0] = 1;
	dev->ops->write(dev, 0, inodes, BLOCK_SIZE);
	dev->ops->write(dev, BITMAP_OFFSET, data_bitmap, BLOCK_SIZE);
	
	strcpy(dire[0].name,".");
	dire[0].inode_id = ROOT;
	strcpy(dire[1].name,"..");
	dire[1].inode_id = ROOT;
	strcpy(dire[2].name,"proc");
	dire[2].inode_id = get_available_inode();
	strcpy(dire[3].name,"dev");
	dire[3].inode_id = get_available_inode();
	dev->ops->write(dev, DATA(0), dire, BLOCK_SIZE);
	dev->ops->write(dev, 0, inodes, BLOCK_SIZE);
	dev->ops->write(dev, BITMAP_OFFSET, data_bitmap, BLOCK_SIZE);
	
}

inode_t* blkfsops_lookup(filesystem_t *fs, const char *path, int flags){
	//int dir_inode = ROOT; //current directory inode id
	int inode_id = ROOT; //return inode id
	dire_t dir[NR_DIRE];
	
	//remove const
	char tmp_path[100];
	strcpy(tmp_path,path);
	
	
	char* left_path = tmp_path;
	char cur_path[100];
	//GOLDLOG(fs->dev->name);
	while(strlen(left_path)){
		GOLDLOG("left_path: %s",left_path);
		if(left_path[0] == '/') left_path += 1;
		memset(cur_path, 0, sizeof(cur_path));
		for(int i = 0;i < 100;i++){
			if(*(left_path + i) == '\0' || *(left_path + i) == '/') break;
			cur_path[i] = *(left_path + i);
		}
		fs->dev->ops->read(fs->dev, DATA(get_data_offset(inode_id)), dir, BLOCK_SIZE);
		for(int i = 0;i <= NR_DIRE;i++){
			//printf("dir[%d] name = %s, inode_id = %d\n",i ,dir[i].name, dir[i].inode_id);
			if(i == NR_DIRE){
				if(!(flags && O_CREAT)){
					LOG("error when lookup: path \"%s\" not found",path);
					return NULL;
				}
				else{ //create new file
					for(int j = 0;j < strlen(left_path);j++){
						if(*(left_path + j) == '/'){
							LOG("error: create file in non-existing directory. Please use mkdir to create the directory first.");
							return NULL;
						}
					}
					for(int j = 0;j <= NR_DIRE;j++){
						if(j == NR_DIRE){
							LOG("error when create new file: no available inode");	
							return NULL;
						}
						if(dir[j].inode_id == 0){
							GOLDLOG("create file \"%s\" successfully",left_path);
							strcpy(dir[j].name,left_path);
							dir[j].inode_id = get_available_inode();
							blkfs->dev->ops->write(blkfs->dev, DATA(get_data_offset(inode_id)), dir, BLOCK_SIZE);
							inode_id = dir[j].inode_id;
							break;
						}
					}
				}
				
			}
			if(strcmp(cur_path, dir[i].name) == 0){
				printf("found %s, inode_id = %d\n",cur_path, dir[i].inode_id);
				inode_id = dir[i].inode_id;
				break;
			}
		}
		left_path += strlen(cur_path);
	}
	//printf("finally : %d\n", inode_id);
	inode_t* ret = pmm->alloc(sizeof(inode_t));
	fs->dev->ops->read(fs->dev, INODE(inode_id), ret, sizeof(inode_t));
	//CYANLOG("%d",ret->block[0]);
	return ret;
}

int blkfsops_close(inode_t *inode){
	//CYANLOG("%d",inode->refcnt);
	if(inode->refcnt == 0)pmm->free(inode);
	return 0;
}

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;
	//printf("sizeof(inode_t) = %d\n",sizeof(inode_t));
	
	
	//initialize inodeops of inode of blkfs.
	blk_inode_ops = pmm->alloc(sizeof(inodeops_t));
	
	
	
	
	blk_inode_ops->open   = dummy;//blk_inode_open;
	blk_inode_ops->read   = blk_inode_read;
	blk_inode_ops->write  = blk_inode_write;
	blk_inode_ops->close  = dummy;//blk_inode_close;
	blk_inode_ops->lseek  = dummy;
	
	
	/*	
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
