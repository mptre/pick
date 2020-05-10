include ${.CURDIR}/../config.mk

PROG=	pty

SRCS+=	pty.c
SRCS+=	compat-reallocarray.c

OBJS=	${SRCS:.c=.o}
DEPS=	${SRCS:.c=.d}

CFLAGS+=	-I${.CURDIR}/..

TESTS+=	key-alt-enter.sh
TESTS+=	key-backspace.sh
TESTS+=	key-ctrl-a.sh
TESTS+=	key-ctrl-c.sh
TESTS+=	key-ctrl-e.sh
TESTS+=	key-ctrl-k.sh
TESTS+=	key-ctrl-l.sh
TESTS+=	key-ctrl-o.sh
TESTS+=	key-ctrl-u.sh
TESTS+=	key-ctrl-w.sh
TESTS+=	key-del.sh
TESTS+=	key-end.sh
TESTS+=	key-enter.sh
TESTS+=	key-home.sh
TESTS+=	key-left.sh
TESTS+=	key-line-down.sh
TESTS+=	key-line-up.sh
TESTS+=	key-page-down.sh
TESTS+=	key-page-up.sh
TESTS+=	key-printable.sh
TESTS+=	key-right.sh
TESTS+=	key-unknown.sh
TESTS+=	misc-match.sh
TESTS+=	misc-realloc.sh
TESTS+=	opt-d.sh
TESTS+=	opt-k.sh
TESTS+=	opt-o.sh
TESTS+=	opt-q.sh
TESTS+=	opt-s.sh
TESTS+=	opt-unknown.sh
TESTS+=	opt-x.sh

.SUFFIXES: .sh .fake

all: ${PROG}
	${MAKE} -C ${.CURDIR} \
		"PTY=${.OBJDIR}/${PROG}" \
		test

${PROG}: ${OBJS}
	${CC} ${DEBUG} -o ${PROG} ${OBJS}

compat-reallocarray.c: ${.CURDIR}/../compat-reallocarray.c
	@cp ${.CURDIR}/../compat-reallocarray.c .

clean:
	rm -f ${DEPS} ${OBJS} ${PROG} compat-reallocarray.c
.PHONY: clean

test: ${TESTS:.sh=.fake}

.sh.fake:
	sh ${.CURDIR}/t.sh ${TESTFLAGS} ${.CURDIR}/util.sh ${.CURDIR}/$<

-include ${DEPS}
