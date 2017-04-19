AC_PREREQ([2.69])
AC_INIT([pick], [1.6.0], [pick-maintainers@calleerlandsson.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_CHECK_FUNCS([pledge reallocarray])
AC_SEARCH_LIBS([setupterm], [curses], [],
  [
    AC_SEARCH_LIBS([setupterm], [ncursesw],
      [AC_DEFINE([HAVE_NCURSESW_H], [1], [Define if ncursesw is available])],
      [AC_MSG_ERROR([unable to find setupterm function])]
    )
  ]
)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
