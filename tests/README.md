Test suite
==========

Test runner
-----------

The `test.c` runner creates a pseudo-terminal used to control the forked pick
process. This allows sending keyboard input sequences and reading the output on
exit.

Input File Format
-----------------

### Format

A input file consist of 0 or more fields, a empty line, followed by the input
sent as is on `stdin` to the pick process.

A field name is terminated with a `:` character and rest of line is recognized
as the corresponding field value. The following fields are recognized:

  - arguments

    Arguments passed to the pick process as is.

  - description

    Purpose of the test.

  - exit

    Expected exit code of the pick process. Defaults to `0` if omitted.

  - input

    Sequence of characters sent as keyboard input to the pick process. The
    following characters are recognized:

      - `^c`

        `^` followed by any character will be interpreted as a `Ctrl` keyboard
        sequence, thus `^A` equals `Ctrl-A`.

      - `\b`

        Backspace

      - `\e`

        Escape

      - `\n`

        Carriage return

    Any other character will be interpreted as is.

  - output

    Expected output from the pick process.

### Examples

A trivial test case:

```
description: select the first choice
input: \n
output: 1

1
2
```

Consult the `*.in` files in this directory for more extensive examples.
