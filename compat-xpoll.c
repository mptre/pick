#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <poll.h>

#include "compat.h"

#ifdef HAVE_BROKEN_POLL

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static int	tty_peekc(int);

int
xpoll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	static int	state;
	int		flags, inready, ttyready;
	int		nready = 0;

	/* XXX filter_choices() poll disabled */
	if (nfds == 1 && timeout == 0)
		return 0;

	if (state == 0) {
		assert(nfds == 2);

		flags = fcntl(fds[0].fd, F_GETFL, 0);
		if (flags == -1)
			return -1;
		flags |= O_NONBLOCK;
		if (fcntl(fds[0].fd, F_SETFL, flags) == -1)
			return -1;
		state++;
	}

	if (state == 1) {
		if (timeout < 0)
			timeout = 100;
		while (nready == 0) {
			if (nfds == 1) {
				state++;
				break;
			}

			inready = poll(&fds[1], 1, timeout);
			if (inready == -1)
				return -1;
			nready += inready;

			ttyready = tty_peekc(fds[0].fd);
			if (ttyready == -1) {
				return -1;
			} else if (ttyready > 0) {
				fds[0].revents = POLLIN;
				nready++;
			} else {
				fds[0].revents = 0;
			}
		}
	}

	if (state == 2) {
		flags = fcntl(fds[0].fd, F_GETFL, 0);
		if (flags == -1)
			return -1;
		flags &= ~O_NONBLOCK;
		if (fcntl(fds[0].fd, F_SETFL, flags) == -1)
			return -1;
		state++;
	}

	if (state == 3) {
		ttyready = tty_peekc(fds[0].fd);
		if (ttyready == -1) {
			return -1;
		} else if (ttyready > 0) {
			fds[0].revents = POLLIN;
			nready++;
		} else {
			fds[0].revents = 0;
		}
	}

	return nready;
}

static int
tty_peekc(int fd)
{
	extern int	tty_getc_peek;
	ssize_t		n;
	unsigned char	c;

	n = read(fd, &c, sizeof(c));
	if (n == -1) {
		if (errno == EAGAIN)
			return 0;
		return -1;
	}
	if (n > 0) {
		assert(tty_getc_peek == -1);
		tty_getc_peek = c;
	}
	return n;
}

#else

int
xpoll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	return poll(fds, nfds, timeout);
}

#endif /* HAVE_BROKEN_POLL */
