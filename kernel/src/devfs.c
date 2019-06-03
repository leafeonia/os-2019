int dev_inode_open(file_t *file, int flags, inode_t* inode){
	kmt->spin_lock(&lk_dev_inode_ops);
	file->inode = inode;
	kmt->spin_unlock(&lk_dev_inode_ops);
	return 0;
}
