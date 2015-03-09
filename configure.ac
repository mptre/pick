AC_PREREQ([2.69])
AC_INIT([pick], [1.1.1], [hello@thoughtbot.com])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_CHECK_HEADERS([fcntl.h locale.h stdlib.h string.h unistd.h])
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove setlocale strdup strlcpy strlcat])
AC_SEARCH_LIBS([initscr], [ncurses], [], [
  AC_MSG_ERROR([unable to find initscr() function. Try to install ncurses])
])

# sys/queue.h exists on most systems, but its capabilities vary a great deal.
# Test for required macros.
AC_TRY_LINK([
	#include <sys/queue.h>
], [
	#ifndef SLIST_ENTRY
	#error SLIST_ENTRY missing
	#endif
	#ifndef SLIST_HEAD
	#error SLIST_HEAD missing
	#endif
	#ifndef SLIST_INIT
	#error SLIST_INIT missing
	#endif
	#ifndef SLIST_INSERT_HEAD
	#error SLIST_INSERT_HEAD missing
	#endif
	#ifndef SLIST_FOREACH
	#error SLIST_FOREACH missing
	#endif
	#ifndef SLIST_EMPTY
	#error SLIST_EMPTY missing
	#endif
	#ifndef SLIST_FIRST
	#error SLIST_FIRST missing
	#endif
	#ifndef SLIST_REMOVE_HEAD
	#error SLIST_REMOVE_HEAD missing
	#endif
], [
AC_DEFINE(HAVE_FULL_QUEUE_H,, Define if sys/queue.h includes required macros)
])

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])
AC_OUTPUT
