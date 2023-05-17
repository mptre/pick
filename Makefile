include ${.CURDIR}/config.mk

VERSION=	4.0.0

PROG=	pick

SRCS+=	compat-pledge.c
SRCS+=	compat-reallocarray.c
SRCS+=	compat-strtonum.c
SRCS+=	pick.c

OBJS=	${SRCS:.c=.o}
DEPS=	${SRCS:.c=.d}

KNFMT+=	compat-pledge.c
KNFMT+=	pick.c

PROG_pty=	pty

SRCS_pty+=	compat-reallocarray.c
SRCS_pty+=	pty.c

OBJS_pty=	${SRCS_pty:.c=.o}
DEPS_pty=	${SRCS_pty:.c=.d}

KNFMT+=	pty.c

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
DISTFILES+=	pty.c
DISTFILES+=	tests/GNUmakefile
DISTFILES+=	tests/Makefile
DISTFILES+=	tests/key-alt-enter.sh
DISTFILES+=	tests/key-backspace.sh
DISTFILES+=	tests/key-ctrl-a.sh
DISTFILES+=	tests/key-ctrl-c.sh
DISTFILES+=	tests/key-ctrl-e.sh
DISTFILES+=	tests/key-ctrl-k.sh
DISTFILES+=	tests/key-ctrl-l.sh
DISTFILES+=	tests/key-ctrl-o.sh
DISTFILES+=	tests/key-ctrl-u.sh
DISTFILES+=	tests/key-ctrl-w.sh
DISTFILES+=	tests/key-del.sh
DISTFILES+=	tests/key-end.sh
DISTFILES+=	tests/key-enter.sh
DISTFILES+=	tests/key-home.sh
DISTFILES+=	tests/key-left.sh
DISTFILES+=	tests/key-line-down.sh
DISTFILES+=	tests/key-line-up.sh
DISTFILES+=	tests/key-page-down.sh
DISTFILES+=	tests/key-page-up.sh
DISTFILES+=	tests/key-printable.sh
DISTFILES+=	tests/key-right.sh
DISTFILES+=	tests/key-unknown.sh
DISTFILES+=	tests/misc-match.sh
DISTFILES+=	tests/misc-realloc.sh
DISTFILES+=	tests/opt-d.sh
DISTFILES+=	tests/opt-k.sh
DISTFILES+=	tests/opt-o.sh
DISTFILES+=	tests/opt-q.sh
DISTFILES+=	tests/opt-s.sh
DISTFILES+=	tests/opt-unknown.sh
DISTFILES+=	tests/opt-x.sh
DISTFILES+=	tests/t.sh
DISTFILES+=	tests/util.sh
DISTFILES+=	tests/valgrind.supp

all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${DEBUG} -o ${PROG} ${OBJS} ${LDFLAGS}

${PROG_pty}: ${OBJS_pty}
	${CC} ${DEBUG} -o ${PROG_pty} ${OBJS_pty} ${LDFLAGS}

clean:
	rm -f ${DEPS} ${OBJS} ${PROG} \
		${DEPS_pty} ${OBJS_pty} ${PROG_pty}
.PHONY: clean

cleandir: clean
	cd ${.CURDIR} && rm -f config.h config.log config.mk
.PHONY: cleandir

dist:
	set -e; \
	d=${PROG}-${VERSION}; \
	mkdir $$d; \
	for f in ${DISTFILES}; do \
		mkdir -p $$d/`dirname $$f`; \
		cp -p ${.CURDIR}/$$f $$d/$$f; \
	done; \
	find $$d -type d -exec touch -r ${.CURDIR}/Makefile {} \;; \
	tar czvf ${.CURDIR}/$$d.tar.gz $$d; \
	(cd ${.CURDIR}; sha256 $$d.tar.gz >$$d.sha256); \
	rm -r $$d
.PHONY: dist

format:
	cd ${.CURDIR} && knfmt -is ${KNFMT}
.PHONY: format

install: all
	@mkdir -p ${DESTDIR}${BINDIR}
	${INSTALL} ${PROG} ${DESTDIR}${BINDIR}
	@mkdir -p ${DESTDIR}${MANDIR}/man1
	${INSTALL_MAN} ${.CURDIR}/pick.1 ${DESTDIR}${MANDIR}/man1
.PHONY: install

lint:
	cd ${.CURDIR} && knfmt -ds ${KNFMT}
.PHONY: lint

test: ${PROG} ${PROG_pty}
	${MAKE} -C ${.CURDIR}/tests \
		"PICK=${.OBJDIR}/${PROG}" \
		"PTY=${.OBJDIR}/${PROG_pty}"
.PHONY: test

-include ${DEPS}
