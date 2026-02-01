# Copyright (c) 2025, 2026 Joachim Schipper <joachim@joachimschipper.nl>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

.PHONY: all clean test monitor

SRC=src/picolog.c
HEADERS=include/picolog.h
TESTS=tests/test.c

# We leave CC at default; gcc or clang both work
CFLAGS=-std=c99 -W -Wall -Wno-sign-compare -Wno-unused-parameter -Wbad-function-cast -Wcast-align -Wcast-qual -Wchar-subscripts -Wfloat-equal -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wwrite-strings -Wundef -Werror -g -O2 -I.

OBJS=${SRC:.c=.o}
TEST_OBJS=${TESTS:.c=.o}

all: tests/test

test: tests/test tests/test.py
	./tests/test.py

clean:
	rm -f ${OBJS} ${TEST_OBJS} ./tests/test

monitor:
	ls ${SRC} ${HEADERS} ${TESTS} tests/test.py Makefile | entr -acs "${MAKE} ${MAKEFLAGS} test"

tests/test: ${OBJS} ${TEST_OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ./tests/test ${OBJS} ${TEST_OBJS}

${OBJS} ${TEST_OBJS}: ${HEADERS}
