# vX.Y.Z - YYYY-MM-DD

## New features

- Add support for HOME and END keys (10f04ae, 0a545bc, 1402ecc, #195) (Jenz)

- Read keys from the `terminfo(3)` database, currently limited to HOME and END
  (19ff2ea, 71f9505, 1ee1a1d, b8857fd, #197) (Anton Lindqvist)

## Bug fixes

- Don't move the cursor upwards if no choices where printed (ecdca3d, #196,
  #198) (Anton Lindqvist)

- Selection bar vanishes on Page-Down (92e492b, #199, #200) (Jenz)

## Changes

- Refactoring and cleanup (74aa733, 5b237d3, bfe26ae, c16475f, 7b99e29, #189)
  (Anton Lindqvist)

- Run `mandoc(1)` and UBSan on Travis (f37e5e8, 368d37d, #190, #193) (Anton
  Lindqvist)

- Update test suite documentation (9086de2) (Anton Lindqvist)

- Enable `ICRNL` input mode (61a5ae5, #192) (Anton Lindqvist)

# v1.6.1 - 2017-04-24

## Bug fixes

- Fix tab output bug (db2d9f4) (Anton Lindqvist)

# v1.6.0 - 2017-04-19

## Bug fixes

- Fix memory leak (63c7da5) (Anton Lindqvist)

- Postpone initial pledge call (bb0ef4f) (Anton Lindqvist)

- Don't match strings inside escape sequences (3c0b407) (Anton Lindqvist)

- Calculate the width of each displayed character (369350b, #184, #185) (Anton
  Lindqvist)

- Define `_GNU_SOURCE` in order to make `wcwidth(3)` available on Linux
  (ffb4ca4, #185) (Anton Lindqvist)

## New features

- Test suite improvements (5c339d8, 2a5196a, 618a490, fd824f5) (Anton Lindqvist)

- Exit with `0` on success and `1` on failure instead of using the exit codes
  defined in `sysexits.h` (e3a5207, #183) (Anton Lindqvist)

# v1.5.4 - 2017-01-23

## Bug fixes

- Fix undefined behavior caused by shifting a signed integer (b4c8656) (Anton
  Lindqvist)

- Only define __dead if it's missing (#180) (Anton Lindqvist)

# v1.5.3 - 2016-12-07

## New features

- Favor linking against curses instead of ncursesw (#167) (Anton Lindqvist)

- Improve performance (647b734) (Anton Lindqvist)

## Bug fixes

- Cleanup man page (#172) (Anton Lindqvist)

- Fix off-by-one error related to query memory allocation (485b436) (Anton
  Lindqvist)

- Fix underline spill over bug (#174) (Anton Lindqvist)

- Fix shortest left-most match bug (42c8197) (Anton Lindqvist)

- Recognize carriage return as enter (#178) (Anton Lindqvist)

# v1.5.2 - 2016-10-13

## Bug fixes

- Refactoring and cleanup (#164) (Anton Lindqvist)

- Disconnect the controlling terminal if present when running the tests
  (506a393) (Anton Lindqvist)

# v1.5.1 - 2016-10-04

## New features

- Improve readability of tests by adding comment support and cleanup test code
  (#161) (Anton Lindqvist)

## Bug fixes

- Refactoring and cleanup (#162) (Anton Lindqvist)

- Fix issues on ancient terminals (#163) (Anton Lindqvist)

# v1.5.0 - 2016-09-19

## New features

- Add FreeBSD installation instructions (#130) (Neel Chauhan)

- Improve test coverage (#137, #150, #151) (Anton Lindqvist)

- Improve search performance (#142) (Anton Lindqvist)

- Add support for vertical scrolling (#144) (Anton Lindqvist)

- Improve test suite #148, (#150, #151, #159) (Anton Lindqvist, Calle Erlandsson)

- Add support for page keys (#149) (Anton Lindqvist)

- Improve escape sequence handling (#155) (Anton Lindqvist)

- Improve cursor handling (#156) (Anton Lindqvist)

- Add a changelog (#158) (Anton Lindqvist)

## Bug fixes

- Fix building on Android (#128) (Fredrik Fornwall)

- Fix output of tabs (#141) (Anton Lindqvist)

- Fix issues on big endian machines (#143) (borstel)

- Fix potential integer overflow (#154) (Anton Lindqvist)

- Refactoring and cleanup (#133, #137, #150, #151, #152) (Anton Lindqvist)

# v1.4.0 - 2016-06-03

## New features

- Add a test suite (Anton Lindqvist, prahlad, Score_Under)

- Add an `-S` option for disable sorting of choices (Calle Erlandsson)

- Add UTF-8 support (Anton Lindqvist)

- Add support for using the delete key and Ctrl-D interchangeably (prahlad)

- Make use of `pledge(1)` on OpenBSD (Anton Lindqvist)

## Bug fixes

- Fix compatibility issues with the Android NDK and the musl libc (Fredrik
  Fornwall)

- Fix issues with newlines caused by colored input (Calle Erlandsson)

- Fix standout rendering of the last line (Anton Lindqvist)

- Refactoring and cleanup (Anton Lindqvist)

## Removed features

- Remove altscreen condition if invoked from Vim (Anton Lindqvist)

# v1.3.0 - 2016-02-12

## New features

- Highlight the matched substring of choices (Anton Lindqvist)

- Scroll query horizontally as needed (Calle Erlandsson)

## Bug fixes

- Add Debian/Ubuntu installation instructions to the README (Scott Stevenson)

- Improve error messages for missing terminfo capabilities (Anton Lindqvist)

- Don't clear the screen unless using the alternate screen (Anton Lindqvist)

- When the query is empty, preserve the input order of choices (Anton Lindqvist)

- Performance improvements and implementation simplifications (Anton Lindqvist)

- Correct Ctrl-K behavior (Calle Erlandsson)

# v1.2.1 - 2015-10-16

## Bug fixes

- Prevent flickering on some ttys by explicitly flushing output after drawing a
  full screen (Score_Under)

- Prevent segfaults on systems where `strtok_r` sets the context pointer to
  `NULL` when the delimiter is not found (Artem Chistyakov)

- Make the `-d` option behave as it is documented (Artem Chistyakov)

- Prevent scrolling (Calle Erlandsson)

# v1.2.0 - 2015-08-07

## New features

- Allow users to pick the current input query. (Ross Hadden)

- Add installation instructions for Mac OS X via MacPorts. (Chunyang Xu)

- Sort choices with the same score lexicographically making sort order
  deterministic. (Calle Erlandsson)

## Bug fixes

- Improve error handling. (Calle Erlandsson)

- Don't drop user input under high load. (Calle Erlandsson)

- Fix build failures on systems where `ncurses` does not exists but `ncursesw`
  does. (Aggelos Avgerinos)

- Fix build failures on Cygwin. (Gabor Buella)

# v1.1.1 - 2015-03-09

## Bug fixes

- Fix checksum issues in Homebrew formula and AUR PKGBUILD. (Calle Erlandsson)

- Avoid "Illegal seek" errors when processing many choices. (Calle Erlandsson)

# v1.1.0 - 2015-02-27

## New features

- Automatically disable alternate screen in Vim. (Calle Erlandsson)

- Add the `-x` option to enable alternate screen. (Calle Erlandsson)

- Add Homebrew formula. (Teo Ljungberg)

- Add AUR PKGBUILD. (Calle Erlandsson)

- Improve README (Matt Jankowski, Calle Erlandsson)

## Bug fixes

- Add missing options to usage message. (Calle Erlandsson)

# v1.0.0 - 2015-02-26

## New features

- Emacs key bindings. (Keith Smiley, Calle Erlandsson)

- Descriptions support. (Calle Erlandsson)

## Bug fixes

- Handle SIGINT (Keith Smiley)

- Fix build issues on certain platforms (Ruben Laguna, Keith Smiley, Calle
  Erlandsson)

- Fix issues related to Vim `system()` and usage of the alternate screen (Keith
  Smiley, Calle Erlandsson)

# v0.0.1 - 2014-08-18

## New features

- Fuzzy select anything. (Calle Erlandsson, Mike Burns)

- Man page. (Calle Erlandsson)
