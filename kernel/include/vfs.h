static filesystem_t* blkfs, *devfs, *procfs;
static fsops_t* blkfs_ops, *devfs_ops, *procfs_ops;
static inodeops_t* dev_inode_ops;//blk_inode_ops, *dev_inode_ops, *proc_inode_ops;
static inode_t* devfs_inode[10];
