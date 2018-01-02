#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int unused;

#ifndef HAVE_PLEDGE

#if HAVE_SECCOMP
#include <sys/ioctl.h>

#include <seccomp.h>
#include <errno.h>
#include <string.h>

/* Most parts are taken from: <https://github.com/aggsol/linux-pledge/> */

#define RULE(syscall)							 \
	rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(syscall), 0);\
	if (rc < 0)							 \
		goto out

#define RULE_IOCTL(syscall, x)						 \
	rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), x,	 \
	    SCMP_A1(SCMP_CMP_EQ, syscall));				 \
	if (rc < 0)							 \
		goto out


enum {
	PLEDGE_STDIO		= 1 << 0,	/* stdio */
	PLEDGE_RPATH		= 1 << 1,	/* rpath */
	PLEDGE_WPATH		= 1 << 2,	/* wpath */
	PLEDGE_CPATH		= 1 << 3,	/* cpath */
	PLEDGE_TTY		= 1 << 4	/* tty */
};

typedef struct {
	char	*name;
	int	 mask;
} promise;

promise ptable[] = {
	{ "stdio",		PLEDGE_STDIO },
	{ "rpath",		PLEDGE_RPATH },
	{ "wpath",		PLEDGE_WPATH },
	{ "cpath",		PLEDGE_CPATH },
	{ "tty",		PLEDGE_TTY }
};

int
pledge(const char *promises, const char *execpromises __attribute__((unused)))
{
	int			n, token;
	int			flags = 0;
	int			rc = -1;
	size_t			i;
	scmp_filter_ctx		ctx;

	ctx = seccomp_init(SCMP_ACT_TRAP);
	if (!ctx) {
		errno = EACCES;
		goto out;
	}

	while (*promises) {
		/* Skip spaces */
		while (*promises && *promises == ' ')
			promises++;

		/* Look for a token */
		token = 0;
		for (i = 0; i < sizeof(ptable) / sizeof(*ptable); i++) {
			n = strlen(ptable[i].name);
			if (!strncmp(promises, ptable[i].name, n)) {
				flags |= ptable[i].mask;
				promises += n;
				token = 1;
				break;
			}
		}

		/* What we saw was not any valid token */
		if (!token) {
			errno = EINVAL;
			goto out;
		}

		/* Ensure the token is terminated by a space or end of string */
		if (*promises && *promises != ' ') {
			errno = EINVAL;
			goto out;
		}
	}

	/*
	 * Define rules
	 */

	/* Everyone is allowed to exit */
	RULE(exit_group);

	/* Necessary for SECCOMP */
	RULE(prctl);

	if (flags & PLEDGE_STDIO) {
		RULE(close);
		RULE(fstat);
		RULE(fstat64);
		RULE(mmap);
		RULE(mmap2);
		RULE(munmap);
		RULE(poll);
		RULE(read);
		RULE(rt_sigaction);
		RULE(sigaction);
		RULE(sigreturn);
		RULE(stat64);
		RULE(time);
		RULE(write);
	}

	if (flags & PLEDGE_TTY) {
		RULE(access);		/* Necessary for terminfo database */
		RULE(open);		/* Necessary for fopen("/dev/tty") */
		/* Allows subset of ioctl */
		RULE_IOCTL(TCGETS, 1);
		RULE_IOCTL(TCSETS, 1);
		RULE_IOCTL(TIOCGWINSZ, 1);
	}

	/* Build and load the filter */
	rc = seccomp_load(ctx);
	if (rc < 0)
		goto out;

	rc = 0;

out:
	seccomp_release(ctx);
	return rc;
}
#endif /* HAVE_SECCOMP */

#endif /* !HAVE_PLEDGE */
