/*
 * Copyright (c) 2025 Joachim Schipper <joachim@joachimschipper.nl>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "include/picolog.h"

/*
 * For debugging, you may want to #define PICOLOG_EXPENSIVE_ASSERTS and/or add
 * (or uncomment) a call to picolog_dump().
 */

/*
 * Internally, the child writes into a msgs[] shared with its parent:
 * +-----------------------------------------------------------+
 * | Foobar\nYet another msg\n\0 tail end of older msg\n\0\0\0 +
 * +-----------------------------------------------------------+
 *                            ^ msgs_idx (in child)            ^ msgs_size
 *
 * The child writes at &msgs[msgs_idx]. msgs[] always ends in \0\0, but may
 * have more \0. The oldest data may be cut off (" tail end of older msg\n").
 */
static char *msgs = NULL;
static size_t msgs_size = 0;
static size_t msgs_idx = 0;

static void
picolog_assert_consistent(void)
{
	assert(msgs != NULL);

#ifdef PICOLOG_EXPENSIVE_ASSERTS
#  ifdef NDEBUG
#    error PICOLOG_EXPENSIVE_ASSERTS requires assert(), i.e. !defined(NDEBUG)
#  endif
	const char *p = memchr(msgs, '\0', msgs_size);
	assert(p != NULL);
	assert(p < &msgs[msgs_size]);
	p = memchr(p + 1, '\0', &msgs[msgs_size] - p - 1);
	assert(p != NULL);
	for ( ; p < &msgs[msgs_size]; p++)
		assert(*p == '\0');
#endif

	assert(msgs[msgs_size - 2] == '\0');
	assert(msgs[msgs_size - 1] == '\0');
}

void
picolog_start_monitor(size_t msg_buf_size)
{
	if (msg_buf_size == 0)
		msg_buf_size = 2 * 1024 * 1024;

	if (msg_buf_size < 4)
		errx(1, "message buffer size %zu too small", msg_buf_size);

	assert(msgs == NULL);
	if ((msgs = mmap(NULL, msg_buf_size, PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED)
		err(1, "failed to allocate shared memory");
	msgs_size = msg_buf_size;

	pid_t pid;
	if ((pid = fork()) == -1)
		err(1, "failed to fork monitor");
	else if (pid == 0)
		return;

	int status;
	if (waitpid(pid, &status, 0) == -1)
		err(1, "failed to wait for child %jd\n", (intmax_t)pid);

	printf("%s%s", &msgs[strlen(msgs) + 1], msgs);

	if (WIFEXITED(status)) {
		exit(WEXITSTATUS(status));
	} else if WIFSIGNALED(status) {
		printf("Child terminated with signal %d\n", WTERMSIG(status));
		exit(126);
	} else {
		printf("Child terminated, wait status = %d\n", status);
		exit(126);
	}
	/* NOTREACHED */
	abort();
}

void
picolog(const char *format, ...)
{
	picolog_assert_consistent();
	unsigned int tried_again;
	for (tried_again = 0; tried_again < 2; tried_again++) {
		const size_t available_size = msgs_size - msgs_idx;
		int needed;

		if (available_size >= 1) {
			va_list ap;
			va_start(ap, format);
			needed = vsnprintf(&msgs[msgs_idx], available_size - 1,
			    format, ap);
			va_end(ap);
		}

		if (available_size >= 1 && needed < available_size - 1) {
			msgs_idx += needed;
			picolog_assert_consistent();
			return;
		} else {
			/* To look at the state of the internal buffer: */
			/* picolog_dump(); */

			/* Erase cut-off msg and restart at start of buffer. */
			bzero(&msgs[msgs_idx], available_size);
			msgs_idx = 0;
			picolog_assert_consistent();
		}
	}

	/*
	 * Currently, logging a message longer than the buffer clears the
	 * buffer. That's arguably correct: an empty buffer is a tail of the
	 * stream of messages.
	 *
	 * FIXME can we handle long messages better? Options:
	 * - leaving the final bytes of the message in the buffer would best
	 *   fit with the idea of being a cut-off log file, but is not trivial
	 *   to do with printf() without allocating extra memory (and for
	 *   reliability, let's not allocate arbitrarily-large amounts of extra
	 *   memory if we receive a larger-than-expected msg!)
	 * - ... unless we double-mmap() the buffer, but that's a bit clever,
	 *   and relies on virtual memory / POSIX much more fundamentally than
	 *   the rest of this code.
	 */
	assert(tried_again == 2);
	assert(msgs_idx == 0);
}

void
picolog_dump(void)
{
	for (size_t i = 0; i < msgs_size; i++) {
		const char *suffix;
		if (i % 32 == 31)
			suffix = "\n";
		else if (i % 8 == 7)
			suffix = "  ";
		else
			suffix = " ";

		if (isprint(msgs[i]))
			fprintf(stderr, " %c%s", msgs[i], suffix);
		else if (msgs[i] == '\n')
			fprintf(stderr, "\\n%s", suffix);
		else if (msgs[i] == '\0')
			fprintf(stderr, "\\0%s", suffix);
		else
			fprintf(stderr, "%02hhu%s", msgs[i], suffix);
	}
	fprintf(stderr, "\nmsgs_idx = %zu\n", msgs_idx);

	picolog_assert_consistent();
}
