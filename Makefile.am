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

.PHONY: release \
	release_build release_push release_clean \
	release_build_arch \
	release_push_arch \
	release_clean_arch

edit_package = sed \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@DIST_ARCHIVES[@]|$(DIST_ARCHIVES)|g' \
	-e 's|@DIST_SHA[@]|$(DIST_SHA)|g'

release: release_build release_push release_clean

release_build: release_build_arch

release_push: release_push_arch

release_clean: release_clean_arch

release_build_arch: DIST_SHA
	git clone --branch master "git@github.com:thoughtbot/$(PACKAGE).git" release-arch
	cd release-arch &&\
		$(edit_package) arch/PKGBUILD.in > arch/PKGBUILD &&\
		git commit -m "Update AUR PKGBUILD for version $(PACKAGE_VERSION)." -- arch/PKGBUILD

release_push_arch:
	cd release-arch &&\
		git push

release_clean_arch:
	rm -rf release-arch

DIST_SHA: Makefile
	rm $(srcdir)/$(DIST_ARCHIVES)
	wget "https://github.com/thoughtbot/$(PACKAGE)/releases/download/v$(PACKAGE_VERSION)/$(DIST_ARCHIVES)" -O $(srcdir)/$(DIST_ARCHIVES)
	$(eval DIST_SHA := $(shell shasum $(srcdir)/$(DIST_ARCHIVES) | cut -d' ' -f1))
