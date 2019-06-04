

filesystem_t* blkfs, *devfs, *procfs;
fsops_t* blkfs_ops, *devfs_ops, *procfs_ops;

#define BLOCK_SIZE 4096
#define NR_INODE 64
#define BITMAP_OFFSET 4096
#define DATA_OFFSET 8192
#define ROOT 2
#define NR_DIRE 64  //each directory contains at most 64 files
#define NR_DATA 900
#define DATA(d) (DATA_OFFSET + BLOCK_SIZE * d)
#define INODE(d) (d * sizeof(inode_t))

struct mount_point{
	const char* path;
	filesystem_t* fs;
};

typedef struct directory{
	char name[60];
	int inode_id;
}dire_t;


//devfs
int dev_inode_open(file_t *file, int flags, inode_t* inode);
ssize_t dev_inode_write(file_t *file, const char *buf, size_t size);
inode_t* devfsops_lookup(filesystem_t *fs, const char *path, int flags);
void devfs_init(filesystem_t *fs, const char *name, device_t *dev);


//procfs
void procfs_init(filesystem_t *fs, const char *name, device_t *dev);


//blkfs
void blkfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t* blkfsops_lookup(filesystem_t *fs, const char *path, int flags);
