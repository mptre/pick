#ifndef COMPAT_H
#define COMPAT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __FreeBSD__
#define __dead __dead2
#endif /* __FreeBSD__ */

#if defined(__linux__) || defined(__CYGWIN__)
#ifndef __dead
#ifdef __GNUC__
#define __dead		__attribute__((__noreturn__))
#else
#define __dead
#endif
#endif
#endif /* __linux__ || __CYGWIN__ */

#ifndef HAVE_PLEDGE
#ifdef HAVE_SECCOMP

#define HAVE_PLEDGE	1
int	pledge(const char *promises, const char *);

#endif /* HAVE_SECCOMP */
#endif /* !HAVE_PLEDGE */

#ifndef HAVE_REALLOCARRAY

void	*reallocarray(void *, size_t, size_t);

#endif /* !HAVE_REALLOCARRAY */

#ifndef HAVE_STRTONUM

long long	strtonum(const char *, long long, long long, const char **);

#endif /* !HAVE_STRTONUM */

#endif /* COMPAT_H */
