AC_PREREQ([2.61])
AC_INIT([pick], [1.8.0], [pick-maintainers@calleerlandsson.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AM_PROG_CC_C_O
AC_CHECK_FUNCS([pledge reallocarray strtonum])
PKG_CHECK_MODULES([NCURSES], [ncursesw],
  [AC_DEFINE([HAVE_NCURSESW_H], [1], [Define if ncursesw is available])],
  [PKG_CHECK_MODULES([NCURSES], [ncurses])])
CFLAGS="$CFLAGS $NCURSES_CFLAGS"
LIBS="$LIBS $NCURSES_LIBS"
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
