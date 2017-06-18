# Test suite

The test suite consists of a shell script `pick-test.sh` responsible for running
tests written in the file format described below and `pick-test.c` which will
invoke pick with a pseudo-terminal.
This allows sending keyboard input sequences and reading the output on exit.

The recommended way to run the test suite is using `make(1)`:

```sh
$ make check || cat test-suite.log
```

Use the `-j` option with a value matching the number of cores on your machine to
speed-up the process:

```sh
$ make -jN check || cat test-suite.log
```

To run one or more specific tests:

```sh
$ env TESTS=tests/01.t make -e check || cat test-suite.log
```

## Format of test files

A test file consist of fields where field names are terminated with `:` and the
rest of line is recognized as the corresponding field value.
Some fields allow multi-line values.
Everything after `#` and up to the next newline is treated as a comment.

A trivial example:

```
description: select the first choice
keys: \\n
stdin:
1
2
stdout:
1
```

The following fields are recognized:

- `args`

  Arguments passed to the pick process as is.

- `description`

  Purpose of the test.

- `exit`

  Exit code of the pick process,
  defaults to `0` if omitted.

- `keys`

  Sequence of characters sent as keyboard input to the pick process.
  The value will be sent through `printf(1)` prior sending it to the pick
  process.
  Spaces will be stripped,
  if not escaped.
  A backslash character is expressed as `\\`,
  thus newline equals `\\n` and space `\\ `.

- `stdin`

  Multi-line input to the pick process.

- `stdout`

  Expected multi-line output from the pick process.
  If omitted,
  output assertion is not performed.
