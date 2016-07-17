AUTOMAKE_OPTIONS=foreign
EXTRA_DIST=INSTALL.md INSTALL.md.in LICENSE README.md

AM_CFLAGS=-Wall -Wextra -pedantic-errors -Wno-unused-parameter -Werror

bin_PROGRAMS=src/pick
dist_src_pick_SOURCES=src/pick.c src/compat.h src/compat.c
dist_man_MANS=man/man1/pick.1

TESTS=tests/01.t tests/02.t tests/03.t tests/04.t tests/05.t tests/06.t \
      tests/07.t tests/08.t tests/09.t tests/10.t tests/11.t tests/12.t \
      tests/13.t tests/14.t tests/15.t tests/16.t tests/17.t tests/18.t \
      tests/19.t tests/20.t tests/21.t tests/22.t tests/23.t tests/24.t \
      tests/25.t tests/26.t tests/27.t tests/28.t tests/29.t tests/30.t \
      tests/31.t tests/32.t
TEST_EXTENSIONS=.t
T_LOG_COMPILER=sh tests/test.sh
check_PROGRAMS=tests/pick-test
tests_pick_test_SOURCES=tests/pick-test.c
# Required by posix_openpt on Linux.
tests_pick_test_CFLAGS=-D_XOPEN_SOURCE=600

INSTALL.md: INSTALL.md.in
	sed -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' $@.in > $@
