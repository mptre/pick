#ifndef COMPAT_H
#define COMPAT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __FreeBSD__
#define __dead __dead2
#endif /* __FreeBSD__ */

#if defined(__linux__) || defined(__CYGWIN__)
#ifdef __GNUC__
#define __dead		__attribute__((__noreturn__))
#else
#define __dead
#endif
#endif /* __linux__ || __CYGWIN__ */

#ifndef HAVE_STRLCPY

#include <sys/types.h>

size_t	strlcpy(char *, const char *, size_t);

#endif /* !HAVE_STRLCPY */

#ifndef HAVE_STRLCAT

#include <sys/types.h>

size_t	strlcat(char *, const char *, size_t);

#endif /* !HAVE_STRLCAT */

#ifndef HAVE_REALLOCARRAY

void	*reallocarray(void *, size_t, size_t);

#endif /* !HAVE_REALLOCARRAY */

#endif /* COMPAT_H */
