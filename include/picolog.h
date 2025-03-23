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

#ifndef PICOLOG_H_INCLUDED
#define PICOLOG_H_INCLUDED 1
/*
 * fork() and return in the child; the parent (monitor) process prints picolog
 * messages after child terminates, and exits with the child's exit status (or
 * 126 if the child exited in any other way, e.g. via a signal).
 *
 * msg_buf_size is 0 (to let the library choose a default) or the approximate
 * number of bytes to allocate.
 */
void picolog_start_monitor(size_t msg_buf_size);

/* Log a message; arguments are as for printf(). */
__attribute__((format(printf, 1, 2)))
void picolog(const char *format, ...);

/* For debugging: visualize current state of internal buffer to stderr. */
void picolog_dump(void);
#endif /* PICOLOG_H_INCLUDED */
