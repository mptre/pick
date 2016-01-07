#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

static const char **argv;

static struct {
	size_t nmemb;
	char v[BUFSIZ];
} input;

static void args(int, const char **);
static void parseinput(const char *);
static void pick(void);

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

	if (!(argv = calloc(ac, sizeof(const char **))))
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
				c = '\b';
				break;
			case 'e':
				c = 0x1B;
				break;
			case 'n':
				c = '\n';
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
pick(void)
{
	char null[BUFSIZ];
	pid_t pid;
	int e, fd, master, slave, status;

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
	if (pid) { /* parent */
		close(slave);

		if (write(master, input.v, input.nmemb) < 0)
			err(1, "write");

		/* Read and discard output from child process, necessary since
		 * it flushes. */
		while ((read(master, null, sizeof(null))) > 0)
			/* NOP */;

		/* Wait and exit with code of the child process. */
		waitpid(pid, &status, 0);
		if (WIFSIGNALED(status))
			exit(128 + WTERMSIG(status));
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
	} else { /* child */
		close(master);

		/* Disconnect the controlling tty. */
		if ((fd = open("/dev/tty", O_RDWR|O_NOCTTY)) < 0)
			err(1, "open");
		/* Ignore any error. */
		(void) ioctl(fd, TIOCNOTTY, NULL);
		close(fd);

		/* Make the current process the session leader. */
		if (setsid() < 0)
			err(1, "setsid");

		/* Connect the slave as the controlling tty. */
		if (ioctl(slave, TIOCSCTTY, NULL) < 0)
			err(1, "ioctl");

		execvp(argv[0], (char *const *)argv);
		e = errno;
		err(1, "%s", argv[0]);
		_exit(126 + (e == ENOENT));
	}
}

int
main(int ac, const char *av[])
{
	args(ac, av);
	pick();
	return 0;
}
