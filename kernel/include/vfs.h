extern filesystem_t* blkfs, *devfs, *procfs;
extern fsops_t* blkfs_ops, *devfs_ops, *procfs_ops;
//static inodeops_t* blk_inode_ops, *dev_inode_ops, *proc_inode_ops;


struct mount_point{
	const char* path;
	filesystem_t* fs;
};




//devfs
int dev_inode_open(file_t *file, int flags, inode_t* inode);
ssize_t dev_inode_write(file_t *file, const char *buf, size_t size);
inode_t* devfsops_lookup(filesystem_t *fs, const char *path, int flags);
void devfs_init(filesystem_t *fs, const char *name, device_t *dev);
