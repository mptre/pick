AUTOMAKE_OPTIONS=foreign

AM_CFLAGS=-Wall -Werror -Wextra
AM_CPPFLAGS=-D_GNU_SOURCE

bin_PROGRAMS=pick
pick_SOURCES=pick.c compat-reallocarray.c compat-strtonum.c compat.h
pick_CPPFLAGS=$(AM_CPPFLAGS) $(NCURSES_CFLAGS)
pick_LDADD=$(NCURSES_LIBS)

dist_man_MANS=pick.1

TESTS=tests/01.t tests/02.t tests/03.t tests/04.t tests/05.t tests/06.t \
      tests/07.t tests/08.t tests/09.t tests/10.t tests/11.t tests/12.t \
      tests/13.t tests/14.t tests/15.t tests/16.t tests/17.t tests/18.t \
      tests/19.t tests/20.t tests/21.t tests/22.t tests/23.t tests/24.t \
      tests/25.t tests/26.t tests/27.t tests/28.t tests/29.t tests/30.t \
      tests/31.t tests/32.t tests/33.t tests/34.t tests/35.t tests/36.t \
      tests/37.t tests/38.t tests/39.t tests/40.t tests/41.t tests/42.t \
      tests/43.t tests/44.t tests/45.t tests/46.t tests/47.t tests/48.t \
      tests/49.t tests/50.t tests/51.t tests/52.t tests/53.t tests/54.t \
      tests/55.t tests/56.t tests/57.t tests/58.t tests/59.t tests/60.t \
      tests/61.t tests/62.t tests/63.t
TEST_EXTENSIONS=.t
T_LOG_COMPILER=$(top_srcdir)/tests/pick-test.sh
AM_COLOR_TESTS=no
check_PROGRAMS=tests/pick-test
tests_pick_test_SOURCES=tests/pick-test.c compat-reallocarray.c
tests_pick_test_CFLAGS=$(AM_CFLAGS)

EXTRA_DIST=INSTALL.md INSTALL.md.in LICENSE README.md tests/pick-test.sh $(TESTS)
DISTCLEANFILES=INSTALL.md

INSTALL.md: INSTALL.md.in
	sed -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
		$(top_srcdir)/$@.in > $@
