/*
 * This file needs a "translation unit" - it needs anything for the compiler to
 * compile. Since the point of this file is to not exist, declare an unused
 * variable here.
 */
int unused;

#include "config.h"

#ifndef HAVE_REALLOCARRAY

void	*reallocarray(void *, size_t, size_t);

#endif /* !HAVE_REALLOCARRAY */
