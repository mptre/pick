AC_PREREQ([2.61])
AC_INIT([pick], [2.0.1], [pick-maintainers@calleerlandsson.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AM_PROG_CC_C_O
AC_ARG_ENABLE([seccomp-debug],
  [AS_HELP_STRING([--enable-seccomp-debug], [enable seccomp debugging])],
  [AC_DEFINE([HAVE_SECCOMP_DEBUG], [1], [Define to debug seccomp])],
  [])
AC_CHECK_FUNCS([pledge reallocarray strtonum])
AC_SEARCH_LIBS([seccomp_init], [seccomp],
  [AC_DEFINE([HAVE_SECCOMP], [1], [Define if seccomp is available])])
AC_SEARCH_LIBS([setupterm], [curses], [], [
  AC_SEARCH_LIBS([setupterm], [ncursesw],
    [AC_DEFINE([HAVE_NCURSESW_H], [1], [Define if ncursesw is available])],
    [PKG_CHECK_MODULES([NCURSES], [ncurses], [], [
      PKG_CHECK_MODULES([NCURSES], [ncursesw],
        [AC_DEFINE([HAVE_NCURSESW_H], [1], [Define if ncursesw is available])]
      )]
    )]
  )
])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
