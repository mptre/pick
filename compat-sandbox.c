#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PLEDGE

#include "compat.h"

#include <err.h>
#include <unistd.h>

void
sandbox(int stage)
{
	switch (stage) {
	case SANDBOX_ENTER:
		if (pledge("stdio tty rpath wpath cpath", NULL) == -1)
			err(1, "pledge");
		break;
	case SANDBOX_MAIN_LOOP_ENTER:
		if (pledge("stdio tty", NULL) == -1)
			err(1, "pledge");
		break;
	case SANDBOX_MAIN_LOOP_EXIT:
		if (pledge("stdio", NULL) == -1)
			err(1, "pledge");
		break;
	}
}

#else

void
sandbox(int stage __attribute__((unused)))
{
}

#endif
