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

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/picolog.h"

[[noreturn]]
static void
usage(void)
{
	errx(127, "usage: test msg_buf_size");
}

int
main(int argc, char **argv)
{
	size_t msg_buf_size;

	if (argc == 1) {
		msg_buf_size = 0;
	} else if (argc == 2 || argc == 4) {
		errno = 0;
		char *endptr;
		const uintmax_t msg_buf_size_um = strtoumax(argv[1], &endptr, 0);
		if (errno != 0 || *endptr != '\0' || msg_buf_size_um > SIZE_MAX)
			usage();
		msg_buf_size = msg_buf_size_um;
	} else {
		usage();
	}

	picolog_start_monitor(msg_buf_size);

	{
	char *line = NULL;
	size_t line_size = 0;
	ssize_t line_len = 0;
	errno = 0;
	while ((line_len = getline(&line, &line_size, stdin)) != -1) {
		assert(line_len == strlen(line));  /* \0's will break test */
		picolog("%s", line);
	}
	if (errno != 0 || !feof(stdin)) {
		/* errno is set by getline(), not by ferror() */
		err(1, "failed to read stdin");
	}
	free(line);
	}

	/* To look at the state of the internal buffer: */
	/* picolog_dump(); */

	return EXIT_SUCCESS;
}
