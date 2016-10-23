AUTOMAKE_OPTIONS=foreign

AM_CFLAGS=-pedantic -Wall -Werror -Wextra

bin_PROGRAMS=pick
dist_pick_SOURCES=pick.c compat.h compat.c
dist_man_MANS=pick.1

TESTS=tests/01.t tests/02.t tests/03.t tests/04.t tests/05.t tests/06.t \
      tests/07.t tests/08.t tests/09.t tests/10.t tests/11.t tests/12.t \
      tests/13.t tests/14.t tests/15.t tests/16.t tests/17.t tests/18.t \
      tests/19.t tests/20.t tests/21.t tests/22.t tests/23.t tests/24.t \
      tests/25.t tests/26.t tests/27.t tests/28.t tests/29.t tests/30.t \
      tests/31.t tests/32.t tests/33.t tests/34.t tests/35.t tests/36.t \
      tests/37.t tests/38.t tests/39.t tests/40.t tests/41.t tests/42.t \
      tests/43.t
TEST_EXTENSIONS=.t
T_LOG_COMPILER=$(top_srcdir)/tests/test.sh
check_PROGRAMS=tests/pick-test
tests_pick_test_SOURCES=tests/pick-test.c
# Required by posix_openpt on Linux.
tests_pick_test_CFLAGS=-D_XOPEN_SOURCE=600

EXTRA_DIST=INSTALL.md INSTALL.md.in LICENSE README.md tests/test.sh $(TESTS)

INSTALL.md: INSTALL.md.in
	sed -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' $@.in > $@
