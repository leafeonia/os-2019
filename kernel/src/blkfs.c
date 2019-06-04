#include <common.h>
#include "vfs.h"
#include <klib.h>
#include <devices.h>
#include "my_os.h"

#define BLOCK_SIZE 4096
#define NR_INODE 64
#define BITMAP_OFFSET 4096
#define DATA_OFFSET 8192
#define ROOT 2
#define NR_DIRE 64  //each directory contains at most 64 files
#define NR_DATA 900
#define DATA(d) (DATA_OFFSET + BLOCK_SIZE * d)
#define INODE(d) (d * sizeof(inode_t))

static inodeops_t* blk_inode_ops;


typedef struct directory{
	char name[60];
	int inode_id;
}dire_t;

//****************************************
//| INODES  |  DATA BITMAP |  DATA ...
//****************************************
//  BLOCK 0       BLOCK 1    BLOCK 2 - 999
void blkfsops_init(filesystem_t *fs, const char *name, device_t *dev){
	assert(sizeof(inode_t) == 64); //64 * NR_INODE = BLOCK_SIZE
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
	dev->ops->write(dev, 0, inodes, BLOCK_SIZE);
	dev->ops->write(dev, BITMAP_OFFSET, data_bitmap, BLOCK_SIZE);
	
	
	//Initialize root directory. Data reside in block 2(data: 0)
	dire_t dire[NR_DIRE];  
	memset(dire,0,sizeof(dire));
	//printf("sizeof dire = %d\n",sizeof(dire));
	inodes[ROOT].refcnt = 1;
	inodes[ROOT].block[0] = 0;
	data_bitmap[0] = 1;
	strcpy(dire[0].name,".");
	dire[0].inode_id = ROOT;
	strcpy(dire[1].name,"..");
	dire[1].inode_id = ROOT;
	dev->ops->write(dev, DATA_OFFSET, dire, BLOCK_SIZE);
	
}

int get_data_offset(int inode_id){
	inode_t inode;
	blkfs->dev->ops->read(blkfs->dev, INODE(inode_id), &inode, sizeof(inode_t));
	printf("get_data_offset: %d\n",inode.block[0]);
	return 0;
}

int get_available_data_block(){
	unsigned char data_bitmap[BLOCK_SIZE];
	blkfs->dev->ops->read(blkfs->dev, BITMAP_OFFSET, data_bitmap, BLOCK_SIZE);
	for(int i = 0;i < NR_DATA;i++){
		if(!data_bitmap[i]) return i;
	}
	LOG("error: no available data block");
	return -1;
}

int get_available_inode(){
	inode_t inodes[NR_INODE];
	blkfs->dev->ops->read(blkfs->dev, 0, &inodes, BLOCK_SIZE);
	for(int i = 0;i < NR_INODE;i++){
		if(inodes[i].refcnt == 0) {
			inodes[i].refcnt = 1;
			inodes[i].block[0] = get_available_data_block();
			return i;
		}
	}
	LOG("error: no availabe inode");
	return -1;
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
					LOG("error when lookup: path \"%s\" not found\n",path);
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
	printf("finally : %d\n", inode_id);
	return NULL;
}

int blkfsops_close(inode_t *inode){
	return 0;
}

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;
	//printf("sizeof(inode_t) = %d\n",sizeof(inode_t));
	
	
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
