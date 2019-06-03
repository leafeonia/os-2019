static filesystem_t* blkfs, *devfs, *procfs;
static fsops_t* blkfs_ops, *devfs_ops, *procfs_ops;
static inodeops_t* dev_inode_ops;//blk_inode_ops, *dev_inode_ops, *proc_inode_ops;
static inode_t* devfs_inode[10];

struct mount_point{
	const char* path;
	filesystem_t* fs;
};
static struct mount_point mt_list[5];
static int mt_idx = 0;
static spinlock_t lk_vfs;
static spinlock_t lk_dev_inode_ops;


//devfs
int dev_inode_open(file_t *file, int flags, inode_t* inode);
