#include <common.h>
#include "vfs.h"

void procfs_init(filesystem_t *fs, const char *name, device_t *dev){
	fs->name = name;
	fs->dev = dev;	
	procfs_ops = pmm->alloc(sizeof(fsops_t));
	procfs->ops = procfs_ops;
}
