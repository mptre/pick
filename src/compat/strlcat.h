/*
 * This file needs a "translation unit" - it needs anything for the compiler to
 * compile. Since the point of this file is to not exist, declare an unused
 * variable here.
 */
int unused;

#include "config.h"

#ifndef HAVE_STRLCAT

#include <sys/types.h>

size_t   strlcat(char *, const char *, size_t);

#endif /* !HAVE_STRLCAT */
