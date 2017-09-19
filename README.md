# Pick

![pick(1) usage](screencast.gif)

The `pick(1)` utility allows users to choose one option from a set of choices
using an interface with fuzzy search functionality.

## Installation

### Debian and Ubuntu

A package for Pick is available as of [Debian 9] and [Ubuntu 15.10].

```sh
apt-get install pick
```

### Gentoo

Package is available from the [official repo][Gentoo].

```sh
emerge pick
```

### Mac OS X via Homebrew

```sh
brew install pick
```

### Mac OS X via MacPorts

```sh
sudo port install pick
```

### FreeBSD via Ports

```sh
cd /usr/ports/sysutils/pick
make install clean
```

### FreeBSD via pkgng

```sh
pkg install pick
```

### OpenBSD

Available in ports under `sysutils/pick`.

### From source

Download the latest [release] and follow the bundled instructions in
`INSTALL.md`.

If you want to try the latest unreleased version,
follow the instructions in [DEVELOPING.md][current].

[Debian 9]: https://packages.debian.org/stretch/pick
[Gentoo]: https://packages.gentoo.org/packages/sys-apps/pick
[Ubuntu 15.10]: http://packages.ubuntu.com/wily/pick
[current]: https://github.com/calleerlandsson/pick/blob/master/DEVELOPING.md
[release]: https://github.com/calleerlandsson/pick/releases/

## Usage

`pick(1)` reads a list of choices on `stdin` and outputs the selected choice on
`stdout`. Therefore it is easily used both in pipelines and subshells:

```sh
git ls-files | pick | xargs less # Select a file in the current git repository to view in less
cd $(find . -type d | pick)      # Select a directory to cd into
```

Pick can also easily be used from within Vim both using `system()` and `!`. For
ready-to-map functions, see [the pick.vim Vim plugin]. For examples of how to
call `pick(1)` from within Vim, see [the pick.vim source code].

***Please note:*** pick requires a fully functional terminal to run and
therefore cannot be run from within gvim or MacVim.

See the `pick(1)` man page for detailed usage instructions and more examples.

[the pick.vim Vim plugin]: https://github.com/calleerlandsson/pick.vim/
[the pick.vim source code]: https://github.com/calleerlandsson/pick.vim/blob/master/plugin/pick.vim

## Copyright

Copyright (c) 2017 Calle Erlandsson, Anton Lindqvist & thoughtbot.
