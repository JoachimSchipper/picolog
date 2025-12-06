#!/usr/bin/env python3
# /// script
# requires-python = ">= 3.10"
# dependencies = []
# ///
#
# Copyright (c) 2025 Joachim Schipper <joachim@joachimschipper.nl>
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

"""Test picolog on various buffer and input sizes.

You may wish to run this through python3 -m pytest [-s], to get additional
context on any failing asserts."""

import os
import random
import string
import subprocess
import sys
import time


def test_picolog():
    """Test picolog on various buffer and input sizes."""

    # To reproduce a run, copy-paste the printed seed over the randint() below:
    seed = random.randint(0, 2**64 - 1)
    sys.stderr.write(f"Random seed = {seed}\n")

    random.seed(seed)

    test_binary = os.path.join(os.path.dirname(__file__), "test")
    chars = (string.ascii_letters + string.digits).encode("ascii")

    test_params = []
    # Some hand-constructed test parameters
    test_params = [
        (msg_buf_size, n_input_lines)
        for msg_buf_size in [16, 4096, 0]
        for n_input_lines in [0, 1, 128, 4096]
    ]
    # Some randomly-generated test parameters
    test_params.extend(
        (
            random.randrange(4, 2 ** random.randint(3, 10)),
            random.randrange(0, 2 ** random.randint(0, 14)),
        )
        for _ in range(32)
    )

    for msg_buf_size, n_input_lines in test_params:
        sys.stderr.write(f"Testing {msg_buf_size=} {n_input_lines=}\n")

        datagen_at = time.monotonic_ns()
        logme = tuple(
            bytes(
                random.choices(
                    chars,
                    k=random.randrange(0, 2 ** random.randint(1, 12)),
                )
            )
            for _ in range(n_input_lines)
        )
        logme_str = b"\n".join(logme) + (b"\n" if logme else b"")
        started_at = time.monotonic_ns()
        with subprocess.Popen(
            (test_binary, str(msg_buf_size)),
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
        ) as proc:
            # Set a (long) timeout, to turn any hangs into failures
            stdout, _stderr = proc.communicate(input=logme_str, timeout=10)
            assert proc.returncode == 0
            done_at = time.monotonic_ns()

        assert _stderr is None, "stderr intentionally not captured"
        if stdout != logme_str:
            # stdout must be a reasonably-long tail of logme_str
            assert logme_str.endswith(stdout)
            assert len(stdout) >= msg_buf_size - 2 - max(
                len(msg) + 1 for msg in logme
            )

        msg = [f"Ran for {(done_at - started_at)/10**9:.3f} s"]
        if len(logme) != 0:
            ns_per_msg = (done_at - started_at) / len(logme)
            msg.append(f", {ns_per_msg:_.1f} ns/msg")
        msg.append(". Generated input in ")
        msg.append(f"{(started_at - datagen_at) / 10**9:.3f}s.\n")
        sys.stderr.write("".join(msg))


if __name__ == "__main__":
    test_picolog()
