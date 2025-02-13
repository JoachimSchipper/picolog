# An utterly trivial framework for deferred logging

You probably want Nanolog, Rust's defmt, or something similar.

However, this *is* simple, and is fairly easy to extend into either
- high-speed deferred formatting; note that our monitor runs as fork() parent,
  and therefore doesn't need to do anything to recover static strings;
- no-limits dynamic formatting, by changing picolog() to explicitly take a
  format string.
