AC_PREREQ([2.69])
AC_INIT([pick], [1.5.2], [calle@calleerlandsson.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_CHECK_HEADERS([locale.h stdlib.h string.h unistd.h])
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove pledge reallocarray setlocale strdup])
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
