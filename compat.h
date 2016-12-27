#ifndef COMPAT_H
#define COMPAT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __dead
#define __dead
#endif

#ifndef HAVE_REALLOCARRAY

void	*reallocarray(void *, size_t, size_t);

#endif /* !HAVE_REALLOCARRAY */

#endif /* COMPAT_H */
