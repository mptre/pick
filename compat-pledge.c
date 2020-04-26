#include "config.h"

extern int unused;

#ifndef HAVE_PLEDGE

int
pledge(const char *promises __attribute__((__unused__)),
    const char *execpromises __attribute__((__unused__)))
{
	return 0;
}

#endif /* !HAVE_PLEDGE */
