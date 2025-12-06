# picolog: simple post-crash debug logs for POSIX systems

After a program crashes, having a full suite of debug messages can be quite
useful. However, full debug logging may cost too much performance or storage to
leave enabled all the time.

Call `picolog_start_monitor()` shortly after starting, then log messages via
`picolog()`. After the process ends for any reason, the last messages are
printed.

picolog is closer to a proof-of-concept than to a mature implementation; you
likely want to use an alternative.

## Implementation

`picolog_start_monitor()` will `fork()`; the parent (monitor)
process will wait for the child process to terminate, then print out the
messages found in a ring buffer in memory shared between parent and child.

    +------------------------------+
    |                              |
    |     Parent (monitor)         |
    |                              |
    | +--------------------------+ |
    | | +----------------------+ | |
    | | | Messages (shared     | | |
    | | | memory, ring buffer) | | |
    | | +----------------------+ | |
    +-|--------------------------|-+
      |                          |
      |  Child                   |
      |  (i.e. actual program)   |
      |                          |
      +--------------------------+

The `fork()`ed child returns from `picolog_start_monitor()` and executes the
rest of the program, `picolog()`-ing as appropriate. `picolog()` itself is
basically `snprintf()`, so reasonably fast, unlikely to block, and in-memory.

## Alternatives
picolog implements
[deferred logging](https://yosefk.com/blog/delayed-printf-for-real-time-logging.html).
picolog aims at a POSIX environment, requires no integration with the build
system, and is simple enough to be hackable.

Deferred logging is a pattern that benefits from integration with the
environment and (often) the build system, and accordingly there are quite a few
implementations with different goals. Interesting families (which often have
spawned re-implementations in C) include
- C++'s [Nanolog](https://github.com/PlatformLab/NanoLog): focused on real-time
  systems (i.e. low latency); and
- Rust's [defmt](https://defmt.ferrous-systems.com/) and C's
  [trice](https://github.com/rokath/trice/): focused on running on a
  microcontroller, with a development workstation decoding the logs; trice may
  be called from inside interrupt handlers (and defmt likely too).

As an alternative to deferred logging, you may want to consider dynamic tracing
systems like [dtrace](https://dtrace.org/) or Linux'
[bpftrace](https://bpftrace.org/) and [bcc](https://iovisor.github.io/bcc/).
Developers may prefer keeping their log statements in the source code, though.

## Requirements

picolog is written to compile with gcc -std=c99 or clang -std=c99. picolog
needs libc and a Unix-like system.

`make test` requires Python (any version supported by the Python developers).
`make monitor` additionally requires `entr`.
