#include "stdio_impl.h"
#include <unistd.h>

#ifndef LOSCFG_FS_VFS
#define close(...) (-1)
#endif

int __stdio_close(FILE *f)
{
	return close(f->fd);
}
