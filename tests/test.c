#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

static const char **argv;
static int gotsig;

static struct {
	size_t nmemb;
	size_t written;
	char v[128];
} input;

static void args(int, const char **);
static void child(int, int);
static void parent(int, int);
static void parseinput(const char *);
static void pick(void);
static void sighandler(int);

void
args(int ac, const char **av)
{
	int c, i;

	while ((c = getopt(ac, (char *const *)av, "i:")) != -1 ) {
		switch (c) {
		case 'i':
			parseinput(optarg);
			break;
		default:
			fputs("usage: test [-i input] [-- argument ...] \n",
			      stderr);
			exit(2);
		}
	}
	while (optind-- > 0)
		ac--, av++;

	/* Ensure room for program and null terminator. */
	if (!(argv = calloc(ac + 2, sizeof(const char **))))
		err(1, "calloc");
	argv[0] = "pick";
	for (i = 0; i < ac; i++)
		argv[i + 1] = av[i];
}

void
parseinput(const char *s)
{
	int c;

	for (; *s; s++) {
		c = *s;
		switch (c) {
		case '^':
			if (!*++s)
				return;
			c = 0x40^*s;
			break;
		case 0x5C: /* \ */
			if (!*++s)
				return;
			switch (*s) {
			case 'b':
				c = 0x7F;
				break;
			case 'e':
				c = 0x1B;
				break;
			case 'n':
				c = 0x0A;
				break;
			default:
				errx(1, "\\%c: unknown escape sequence", *s);
			}
			break;
		}
		input.v[input.nmemb++] = c;
	}
}

void
sighandler(int sig)
{
	gotsig = sig == SIGCHLD;
}

void
child(int master, int slave)
{
	struct winsize ws;
	int fd, e;

	close(master);

	/* Disconnect the controlling tty. */
	if ((fd = open("/dev/tty", O_RDWR|O_NOCTTY)) < 0)
		err(1, "open");
	/* Ignore any error. */
	(void)ioctl(fd, TIOCNOTTY, NULL);
	close(fd);

	/* Make the current process the session leader. */
	if (setsid() < 0)
		err(1, "setsid");

	/* Connect the slave as the controlling tty. */
	if (ioctl(slave, TIOCSCTTY, NULL) < 0)
		err(1, "ioctl");

	/* Set window size to known dimensions, necessary in order to deduce
	 * when scrolling should occur. */
	ws.ws_col = 80, ws.ws_row = 24;
	if (ioctl(slave, TIOCSWINSZ, &ws) < 0)
		err(1, "ioctl");

	/* Enable malloc.conf(5) options on OpenBSD which will abort the pick
	 * process when reading/writing out-of-bounds or accessing already freed
	 * memory. */
	if (setenv("MALLOC_OPTIONS", "S", 0) < 0)
		err(1, "setenv");

	execvp(argv[0], (char *const *)argv);
	e = errno;
	err(1, "%s", argv[0]);
	_exit(126 + (e == ENOENT));
}

void
parent(int master, int slave)
{
	char buf[BUFSIZ];
	fd_set rfd;
	struct timeval timeout;
	ssize_t n;

	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 1;
	while (!gotsig) {
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

		/* Read and discard output from child process, necessary since
		 * it flushes. */
		if (read(master, buf, sizeof(buf)) < 0)
			err(1, "read");

		/* When the pick process has flushed its output we can ensure
		 * the call to tcsetattr has been completed and canonical mode
		 * is disabled. At this point input can be written without any
		 * line editing taking place. */
		if (input.written < input.nmemb) {
			if ((n = write(master, input.v + input.written,
					    input.nmemb - input.written)) == -1)
				err(1, "write");
			input.written += n;
		}
	}

	/* If the last slave file descriptor closes while a 'read' call is in
	 * progress, the read may fail with EIO. To avoid that happening in
	 * the above loop, this copy of the slave file descriptor is left open
	 * until now. */
	close(slave);
}

void
pick(void)
{
	pid_t pid;
	int master, slave, status;

	if (signal(SIGCHLD, sighandler) == SIG_ERR)
		err(1, "signal");
	if ((master = posix_openpt(O_RDWR)) < 0)
		err(1, "posix_openpt");
	if (grantpt(master) < 0)
		err(1, "grantpt");
	if (unlockpt(master) < 0)
		err(1, "unlockpt");
	if ((slave = open(ptsname(master), O_RDWR)) < 0)
		err(1, "open");

	if ((pid = fork()) < 0)
		err(1, "fork");
	if (pid) {
		parent(master, slave);
		/* Wait and exit with code of the child process. */
		waitpid(pid, &status, 0);
		if (WIFSIGNALED(status))
			exit(128 + WTERMSIG(status));
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
	} else {
		child(master, slave);
	}
}

int
main(int ac, const char *av[])
{
	args(ac, av);
	pick();
	return 0;
}
