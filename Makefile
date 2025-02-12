.PHONY: all clean test monitor

SRC=src/picolog.c
HEADERS=include/picolog.h
TESTS=tests/test.c

CC=gcc
CFLAGS=-std=c99 -W -Wall -Wno-sign-compare -Wno-unused-parameter -Wbad-function-cast -Wcast-align -Wcast-qual -Wchar-subscripts -Wfloat-equal -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wwrite-strings -Wundef -Werror -g -O2 -I.

OBJS=${SRC:.c=.o}
TEST_OBJS=${TESTS:.c=.o}

all: tests/test

test: tests/test tests/test.py
	./tests/test.py

clean:
	rm -f ${OBJS} ${TEST_OBJS} ./tests/test ./tests/test.out

monitor:
	ls ${SRC} ${HEADERS} ${TESTS} tests/test.py Makefile | entr -acs "${MAKE} ${MAKEFLAGS} test"

tests/test: ${OBJS} ${TEST_OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ./tests/test ${OBJS} ${TEST_OBJS}

${OBJS} ${TEST_OBJS}: ${HEADERS}
