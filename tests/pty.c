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

__dead static void	 child(int, int, int, char **);
static void		 parent(int, int, const char *);
static char		*parsekeys(const char *);
static void		 sighandler(int);
__dead static void	 usage(void);

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
	char	*keys = NULL;
	pid_t	 pid;
	int	 c, master, slave, status;

	while ((c = getopt(argc, argv, "k:")) != -1)
		switch (c) {
		case 'k':
			keys = parsekeys(optarg);
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		usage();

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
		child(master, slave, argc, argv);
		/* NOTREACHED */
	default:
		parent(master, slave, keys != NULL ? keys : "");
		/* Wait and exit with code of the child process. */
		waitpid(pid, &status, 0);
		if (WIFSIGNALED(status))
			exit(128 + WTERMSIG(status));
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
	}

	free(keys);

	return 0;
}

__dead static void
usage(void)
{
        fprintf(stderr, "usage: pick-test [-k path] -- utility "
	    "[argument ...]\n");
	exit(1);
}

static char *
parsekeys(const char *path)
{
	FILE	*fh;
	char	*buf;
	size_t	 len = 0;
	size_t	 size = 16;
	int	 c, esc, ctrl;

	if ((fh = fopen(path, "r")) == NULL)
		err(1, "fopen: %s", path);

	if ((buf = malloc(size)) == NULL)
		err(1, NULL);

	ctrl = esc = 0;
	while ((c = fgetc(fh)) != EOF) {
		if (c == '\\') {
			esc = 1;
		} else if (!esc && c == ' ') {
			continue;
		} else if (c == '^') {
			ctrl = 'A' - 1;
			continue;
		} else {
			buf[len++] = c - ctrl;
			ctrl = esc = 0;
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
child(int master, int slave, int argc, char **argv)
{
	const char	**env;
	char		 *cmd = NULL;
	struct winsize	  ws;
	size_t	 	  siz = 0;
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

	/* Join command. */
	for (; argc > 0; argc--, argv++) {
		size_t len;

		if (siz > 0) {
			cmd[siz] = ' ';
			siz++;
		}

		len = strlen(argv[0]);
		cmd = reallocarray(cmd, 1, siz + len + 1);
		if (cmd == NULL)
			err(1, NULL);
		memcpy(&cmd[siz], argv[0], len);
		siz += len;
		cmd[siz] = '\0';
	}

	execlp("sh", "sh", "-c", cmd, NULL);
	err(1, "sh");
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
