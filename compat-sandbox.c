#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PLEDGE

#include <err.h>
#include <unistd.h>

#include "compat.h"

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

#elif HAVE_SECCOMP

#include <sys/ioctl.h>

#include <err.h>
#include <seccomp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat.h"

#define ALLOW(syscall)						\
	(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(syscall), 0) < 0)

#define ALLOW_IOCTL(syscall, x)						\
	(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), x,	\
	 SCMP_A1(SCMP_CMP_EQ, syscall, 0)) < 0)

/*
 * Print out the offending syscall and exit.
 * Not thread-safe and shall only be used for debugging purposes.
 */
void
handle_sigsys(int signum __attribute__((unused)), siginfo_t *info,
    void *ctx __attribute__((unused)))
{
	warnx("disallowed syscall #%d", info->si_syscall);
	fflush(stderr);
	exit(1);
}

void
sandbox_sighandler(void)
{
	struct sigaction	act;
	sigset_t		mask;

	memset(&act, 0, sizeof(act));
	sigemptyset(&mask);
	sigaddset(&mask, SIGSYS);
	act.sa_sigaction = &handle_sigsys;
	act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGSYS, &act, NULL) == -1)
		err(1, "sigaction");
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		err(1, "sigprocmask");
}

void
sandbox(int stage)
{
	scmp_filter_ctx	ctx;

	switch (stage) {
	case SANDBOX_ENTER:
#ifdef HAVE_SECCOMP_DEBUG
		sandbox_sighandler();
#endif

		if ((ctx = seccomp_init(SCMP_ACT_TRAP)) == NULL)
			err(1, "seccomp_init");

		if (ALLOW(access) ||
		    ALLOW(close) ||
		    ALLOW(exit_group) ||
		    ALLOW(fstat) ||
		    ALLOW(fstat64) ||
		    ALLOW(mmap) ||
		    ALLOW(mmap2) ||
		    ALLOW(munmap) ||
		    ALLOW(open) ||
		    ALLOW(poll) ||
		    ALLOW(read) ||
		    ALLOW(rt_sigaction) ||
		    ALLOW(sigaction) ||
		    ALLOW(sigreturn) ||
		    ALLOW(stat) ||
		    ALLOW(stat64) ||
		    ALLOW(time) ||
		    ALLOW(write) ||
		    ALLOW_IOCTL(TCGETS, 1) ||
		    ALLOW_IOCTL(TCSETS, 1) ||
		    ALLOW_IOCTL(TIOCGWINSZ, 1))
			err(1, "seccomp_rule_add");

		if (seccomp_load(ctx) < 0)
			err(1, "seccomp_load");

		seccomp_release(ctx);
		break;
	case SANDBOX_MAIN_LOOP_ENTER:
		break;
	case SANDBOX_MAIN_LOOP_EXIT:
		break;
	}
}

#else

void
sandbox(int stage __attribute__((unused)))
{
}

#endif
