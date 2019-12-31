include ${.CURDIR}/../Makefile.inc

PROG=	pick-test

SRCS+=	pick-test.c
SRCS+=	compat-reallocarray.c

OBJS=	${SRCS:.c=.o}
DEPS=	${SRCS:.c=.d}

CFLAGS+=	-I${.CURDIR}/..

TESTS+=	key-alt-enter.t
TESTS+=	key-backspace.t
TESTS+=	key-ctrl-a.t
TESTS+=	key-ctrl-c.t
TESTS+=	key-ctrl-e.t
TESTS+=	key-ctrl-k.t
TESTS+=	key-ctrl-l.t
TESTS+=	key-ctrl-o.t
TESTS+=	key-ctrl-u.t
TESTS+=	key-ctrl-w.t
TESTS+=	key-del.t
TESTS+=	key-end.t
TESTS+=	key-enter.t
TESTS+=	key-home.t
TESTS+=	key-left.t
TESTS+=	key-line-down.t
TESTS+=	key-line-up.t
TESTS+=	key-page-down.t
TESTS+=	key-page-up.t
TESTS+=	key-printable.t
TESTS+=	key-right.t
TESTS+=	key-unknown.t
TESTS+=	misc-match.t
TESTS+=	misc-realloc.t
TESTS+=	opt-d.t
TESTS+=	opt-h.t
TESTS+=	opt-k.t
TESTS+=	opt-o.t
TESTS+=	opt-q.t
TESTS+=	opt-s.t
TESTS+=	opt-unknown.t
TESTS+=	opt-x.t

all: ${PROG}
	${MAKE} -C ${.CURDIR} test

.SUFFIXES: .t .out

.t.out:
	sh ${.CURDIR}/pick-test.sh -b ${.OBJDIR}/${PROG} $<

${PROG}: ${OBJS}
	${CC} ${DEBUG} -o ${PROG} ${OBJS}

compat-reallocarray.c: ${.CURDIR}/../compat-reallocarray.c
	@cp ${.CURDIR}/../compat-reallocarray.c .

clean:
	rm -f ${DEPS} ${OBJS} ${PROG}
.PHONY: clean

test: ${TESTS:.t=.out}

-include ${DEPS}
