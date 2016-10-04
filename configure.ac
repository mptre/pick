AC_PREREQ([2.69])
AC_INIT([pick], [1.5.1], [hello@thoughtbot.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_CHECK_HEADERS([locale.h stdlib.h string.h unistd.h])
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove pledge reallocarray setlocale strdup])
AC_SEARCH_LIBS([initscr], [ncursesw ncurses], [], [
  AC_MSG_ERROR([unable to find initscr() function. Try to install ncurses])
])

AC_TRY_LINK([
	#include <ncursesw/curses.h>
	#include <ncursesw/term.h>
], [], [
AC_DEFINE(HAVE_NCURSESW_H,, Define if ncursesw is available)
])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
