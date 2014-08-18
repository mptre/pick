pick
====

`pick` allows users to select from set of choices using an `ncurses(3X)`
interface with fuzzy search functionality.

Installation
------------

1. Download the latest release from [the `pick` Releases
   page](https://github.com/thoughtbot/pick/releases) and extract the archive:

        tar -zxvf pick-VERSION.tar.gz

2. Follow the bundled installation instructions:

        cd pick-VERSION
        less INSTALL.md

Usage
-----

    pick [-h] [-v] [-q QUERY]

`pick` accepts a list of choices as input and produces the selected choice as
output.

Supply the `-h` option to display a usage message with a list of the available
options.

Supply the `-v` option to display the version of `pick` that is running.

Supply the `-q` option and a string to pre-populate the search query.

The `pick` `ncurses(3X)` interface is operated with the following keys:

* <kbd>Printable characters</kbd>

  Printable characters are added to the search query input field and will refine
  the current search query.

* <kbd>Left</kbd>/<kbd>Right</kbd>

  Move the cursor left and right in the search query input field.

* <kbd>Backspace</kbd>

  Delete one character to the left of the cursor in the search query input
  field.

* <kbd>Up</kbd>/<kbd>Down</kbd>

  Select between choices matching the current search query.

* <kbd>Enter</kbd>

  Output the currently selected choice and exit.

`pick` can be used to select anything and is very effective when combined with
utilities like `xargs(1)`.

Select a file in the current directory to open using `xdg-open(1)`:

    find -type f | pick | xargs xdg-open

Copyright
---------

Copyright (c) 2014 Calle Erlandsson & thoughtbot, Inc.

Lead by Calle Erlandsson & thoughtbot, Inc.
