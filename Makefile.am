AUTOMAKE_OPTIONS=foreign

AM_CFLAGS=-Wall
AM_CPPFLAGS=-D_GNU_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE

bin_PROGRAMS=pick
pick_SOURCES=pick.c compat-reallocarray.c compat-strtonum.c compat.h
pick_CPPFLAGS=$(AM_CPPFLAGS) $(NCURSES_CFLAGS)
pick_LDADD=$(NCURSES_LIBS)

dist_man_MANS=pick.1

TESTS=tests/key-alt-enter.t \
      tests/key-backspace.t \
      tests/key-ctrl-a.t \
      tests/key-ctrl-c.t \
      tests/key-ctrl-e.t \
      tests/key-ctrl-k.t \
      tests/key-ctrl-l.t \
      tests/key-ctrl-o.t \
      tests/key-ctrl-u.t \
      tests/key-ctrl-w.t \
      tests/key-del.t \
      tests/key-end.t \
      tests/key-enter.t \
      tests/key-home.t \
      tests/key-left.t \
      tests/key-line-down.t \
      tests/key-line-up.t \
      tests/key-page-down.t \
      tests/key-page-up.t \
      tests/key-printable.t \
      tests/key-right.t \
      tests/key-unknown.t \
      tests/misc-match.t \
      tests/misc-realloc.t \
      tests/opt-d.t \
      tests/opt-h.t \
      tests/opt-k.t \
      tests/opt-o.t \
      tests/opt-q.t \
      tests/opt-s.t \
      tests/opt-unknown.t \
      tests/opt-v.t \
      tests/opt-x.t
TEST_EXTENSIONS=.t
T_LOG_COMPILER=$(top_srcdir)/tests/pick-test.sh \
	-e MALLOC_OPTIONS=@MALLOC_OPTIONS@
AM_COLOR_TESTS=no
check_PROGRAMS=tests/pick-test
tests_pick_test_SOURCES=tests/pick-test.c compat-reallocarray.c
tests_pick_test_CFLAGS=$(AM_CFLAGS)

EXTRA_DIST=INSTALL.md INSTALL.md.in LICENSE README.md tests/pick-test.sh $(TESTS)
DISTCLEANFILES=INSTALL.md

INSTALL.md: INSTALL.md.in
	sed -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
		$(top_srcdir)/$@.in > $@
