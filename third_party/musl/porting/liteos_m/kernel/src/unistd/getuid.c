#include <unistd.h>
#include "syscall.h"

#ifndef FOUNDATION_UID
#define FOUNDATION_UID 7
#endif
uid_t getuid(void)
{
    return FOUNDATION_UID;
	//return __syscall(SYS_getuid);
}
