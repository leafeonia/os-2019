void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
}
