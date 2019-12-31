include ${.CURDIR}/Makefile.inc

VERSION=	3.0.1

PROG=	pick

SRCS+= compat-pledge.c
SRCS+= compat-reallocarray.c
SRCS+= compat-strtonum.c
SRCS+= pick.c

OBJS=	${SRCS:.c=.o}
DEPS=   ${SRCS:.c=.d}

CFLAGS+=	${DEBUG}

DISTFILES+=	CHANGELOG.md
DISTFILES+=	CODE_OF_CONDUCT.md
DISTFILES+=	CONTRIBUTING.md
DISTFILES+=	GNUmakefile
DISTFILES+=	LICENSE
DISTFILES+=	Makefile
DISTFILES+=	README.md
DISTFILES+=	compat-pledge.c
DISTFILES+=	compat-reallocarray.c
DISTFILES+=	compat-strtonum.c
DISTFILES+=	configure
DISTFILES+=	pick.1
DISTFILES+=	pick.c
DISTFILES+=	tests/GNUmakefile
DISTFILES+=	tests/Makefile
DISTFILES+=	tests/README.md
DISTFILES+=	tests/key-alt-enter.t
DISTFILES+=	tests/key-backspace.t
DISTFILES+=	tests/key-ctrl-a.t
DISTFILES+=	tests/key-ctrl-c.t
DISTFILES+=	tests/key-ctrl-e.t
DISTFILES+=	tests/key-ctrl-k.t
DISTFILES+=	tests/key-ctrl-l.t
DISTFILES+=	tests/key-ctrl-o.t
DISTFILES+=	tests/key-ctrl-u.t
DISTFILES+=	tests/key-ctrl-w.t
DISTFILES+=	tests/key-del.t
DISTFILES+=	tests/key-end.t
DISTFILES+=	tests/key-enter.t
DISTFILES+=	tests/key-home.t
DISTFILES+=	tests/key-left.t
DISTFILES+=	tests/key-line-down.t
DISTFILES+=	tests/key-line-up.t
DISTFILES+=	tests/key-page-down.t
DISTFILES+=	tests/key-page-up.t
DISTFILES+=	tests/key-printable.t
DISTFILES+=	tests/key-right.t
DISTFILES+=	tests/key-unknown.t
DISTFILES+=	tests/misc-match.t
DISTFILES+=	tests/misc-realloc.t
DISTFILES+=	tests/opt-d.t
DISTFILES+=	tests/opt-k.t
DISTFILES+=	tests/opt-o.t
DISTFILES+=	tests/opt-q.t
DISTFILES+=	tests/opt-s.t
DISTFILES+=	tests/opt-unknown.t
DISTFILES+=	tests/opt-x.t
DISTFILES+=	tests/pick-test.c
DISTFILES+=	tests/pick-test.sh

all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${DEBUG} -o ${PROG} ${OBJS} ${LDFLAGS}

clean:
	rm -f ${DEPS} ${OBJS} ${PROG}
.PHONY: clean

dist:
	set -e; \
	d=${PROG}-${VERSION}; \
	mkdir $$d; \
	for f in ${DISTFILES}; do \
		mkdir -p $$d/`dirname $$f`; \
		cp ${.CURDIR}/$$f $$d/$$f; \
	done; \
	tar czvf ${.CURDIR}/$$d.tar.gz $$d; \
	(cd ${.CURDIR}; sha256 $$d.tar.gz >$$d.sha256); \
	rm -r $$d
.PHONY: dist

distclean: clean
	rm -f ${.CURDIR}/Makefile.inc ${.CURDIR}/config.h \
		${.CURDIR}/config.log ${.CURDIR}/${PROG}-${VERSION}.tar.gz \
		${.CURDIR}/${PROG}-${VERSION}.sha256
.PHONY: distclean

install: ${PROG}
	@mkdir -p ${DESTDIR}${BINDIR}
	${INSTALL} ${PROG} ${DESTDIR}${BINDIR}
	@mkdir -p ${DESTDIR}${MANDIR}/man1
	${INSTALL_MAN} ${.CURDIR}/pick.1 ${DESTDIR}${MANDIR}/man1
.PHONY: install

test: ${PROG}
	${MAKE} -C ${.CURDIR}/tests \
		"MALLOC_OPTIONS=${MALLOC_OPTIONS}" "PICK=${.OBJDIR}/${PROG}"
.PHONY: test

-include ${DEPS}
