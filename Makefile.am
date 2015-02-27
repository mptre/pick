AUTOMAKE_OPTIONS=foreign
SUBDIRS=src man
EXTRA_DIST=INSTALL.md INSTALL.md.in LICENSE README.md

INSTALL.md: INSTALL.md.in
	sed -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' $@.in > $@

.PHONY: release \
	release_build release_push release_clean \
	release_build_homebrew \
	release_push_homebrew \
	release_clean_homebrew

edit_package = sed \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@DIST_ARCHIVES[@]|$(DIST_ARCHIVES)|g' \
	-e 's|@DIST_SHA[@]|$(DIST_SHA)|g'

release: release_build release_push release_clean

release_build: release_build_homebrew

release_push: release_push_homebrew

release_clean: release_clean_homebrew

release_build_homebrew: DIST_SHA
	git clone git@github.com:thoughtbot/homebrew-formulae.git homebrew-formulae
	$(edit_package) homebrew/pick.rb.in > homebrew-formulae/Formula/pick.rb
	cd homebrew-formulae &&\
		git addd Formula/pick.rb &&\
		git commit -am "pick: Release version $(PACKAGE_VERSION)"

release_push_homebrew:
	cd homebrew-formulae &&\
		git push

release_clean_homebrew:
	rm -rf homebrew-formulae

DIST_SHA: Makefile distcheck
	$(eval DIST_SHA := $(shell shasum $(srcdir)/$(DIST_ARCHIVES) | cut -d' ' -f1))
