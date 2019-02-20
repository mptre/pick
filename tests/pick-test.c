#include "config.h"

#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

__dead static void	 child(int, int);
static void		 parent(int, int, const char *);
static char		*parsekeys(const char *);
static void		 sighandler(int);
__dead static void	 usage(void);

static char		**pickargv;
/*
 * Mandatory environment variables required by pick to operate correctly.
 * Any existing value will be overwritten.
 */
static const char	 *pickenv[] = {
	"LC_ALL",	"en_US.UTF-8",
	"TERM",		"xterm",
	NULL,
};
static int		  gotsig;

int
main(int argc, char *argv[])
{
	char	*pick = NULL;
	char	*keys = "";
	pid_t	 pid;
	int	 c, i, master, slave, status;

	while ((c = getopt(argc, argv, "b:k:")) != -1)
		switch (c) {
		case 'b':
			pick = optarg;
			break;
		case 'k':
			keys = parsekeys(optarg);
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (pick == NULL)
		usage();

	/* Ensure room for program and null terminator. */
	if ((pickargv = calloc(argc + 2, sizeof(const char **))) == NULL)
		err(1, NULL);
	pickargv[0] = pick;
	for (i = 0; i < argc; i++)
		pickargv[i + 1] = argv[i];

	if (signal(SIGCHLD, sighandler) == SIG_ERR)
		err(1, "signal");
	if ((master = posix_openpt(O_RDWR)) == -1)
		err(1, "posix_openpt");
	if (grantpt(master) == -1)
		err(1, "grantpt");
	if (unlockpt(master) == -1)
		err(1, "unlockpt");
	if ((slave = open(ptsname(master), O_RDWR)) == -1)
		err(1, "%s", ptsname(master));

	switch ((pid = fork())) {
	case -1:
		err(1, "fork");
		/* NOTREACHED */
	case 0:
		child(master, slave);
		/* NOTREACHED */
	default:
		parent(master, slave, keys);
		/* Wait and exit with code of the child process. */
		waitpid(pid, &status, 0);
		if (WIFSIGNALED(status))
			exit(128 + WTERMSIG(status));
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
	}

	return 0;
}

__dead static void
usage(void)
{
        fprintf(stderr, "usage: pick-test -b binary [-k path] [-- arg ...]\n");
	exit(1);
}

static char *
parsekeys(const char *path)
{
	FILE	*fh;
	char	*buf;
	size_t	 len = 0;
	size_t	 size = 16;
	int	 c, esc;

	if ((fh = fopen(path, "r")) == NULL)
		err(1, "fopen: %s", path);

	if ((buf = malloc(size)) == NULL)
		err(1, NULL);

	esc = 0;
	while ((c = fgetc(fh)) != EOF) {
		if (c == '\\') {
			esc = 1;
		} else if (!esc && c == ' ') {
			continue;
		} else {
			buf[len++] = c;
			esc = 0;
		}

		if (size <= len) {
			if ((buf = reallocarray(buf, 2, size)) == NULL)
				err(1, NULL);
			size *= 2;
		}
	}
	if (ferror(fh))
		err(1, "fgetc: %s", path);
	fclose(fh);
	buf[len] = '\0';

	return buf;
}

static void
sighandler(int sig)
{
	gotsig = sig == SIGCHLD;
}

__dead static void
child(int master, int slave)
{
	const char	**env;
	struct winsize	  ws;
	int		  fd;

	close(master);

	/* Disconnect the controlling tty, if present. */
	if ((fd = open("/dev/tty", O_RDWR | O_NOCTTY)) >= 0) {
		/* Ignore any error. */
		(void)ioctl(fd, TIOCNOTTY, NULL);
		close(fd);
	}

	/* Make the current process the session leader. */
	if (setsid() == -1)
		err(1, "setsid");

	/* Connect the slave as the controlling tty. */
	if (ioctl(slave, TIOCSCTTY, NULL) == -1)
		err(1, "TIOCSCTTY");

	/*
	 * Set window size to known dimensions, necessary in order to deduce
	 * when scrolling should occur.
	 */
	memset(&ws, 0, sizeof(ws));
	ws.ws_col = 80, ws.ws_row = 24;
	if (ioctl(slave, TIOCSWINSZ, &ws) == -1)
		err(1, "TIOCSWINSZ");

	for (env = pickenv; *env != NULL; env += 2)
		if (setenv(env[0], env[1], 1) == -1)
			err(1, "setenv: %s", env[0]);

	execv(pickargv[0], pickargv);
	err(1, "%s", pickargv[0]);
}

static void
parent(int master, int slave, const char *keys)
{
	char		buf[BUFSIZ];
	fd_set		rfd;
	struct timeval	timeout;
	ssize_t		n;
	size_t		len;
	size_t		written = 0;

	len = strlen(keys);

	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 2;
	while (gotsig == 0) {
		FD_ZERO(&rfd);
		FD_SET(master, &rfd);
		switch (select(master + 1, &rfd, NULL, NULL, &timeout)) {
		case -1:
			if (errno == EINTR)
				continue;
			err(1, "select");
			/* NOTREACHED */
		case 0:
			errx(1, "time limit exceeded");
			/* NOTREACHED */
		default:
			if (!FD_ISSET(master, &rfd))
				continue;
		}

		/*
		 * Read and discard output from child process, necessary since
		 * it flushes.
		 */
		if (read(master, buf, sizeof(buf)) == -1)
			err(1, "read");

		/*
		 * When the pick process has flushed its output we can ensure
		 * the call to tcsetattr has been completed and canonical mode
		 * is disabled. At this point input can be written without any
		 * line editing taking place.
		 */
		if (written < len) {
			if ((n = write(master, keys + written,
					    len - written)) == -1)
				err(1, "write");
			written += n;
		}
	}

	/*
	 * If the last slave file descriptor closes while a read call is in
	 * progress, the read may fail with EIO. To avoid that happening in the
	 * above loop, this copy of the slave file descriptor is left open until
	 * now.
	 */
	close(slave);
}
