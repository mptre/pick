# Pick

![pick(1) usage](screencast.gif)

The `pick(1)` utility allows users to choose one option from a set of choices
using an interface with fuzzy search functionality.

## Installation

### Arch Linux

A package for Pick is available in [the AUR].

```sh
curl -O https://aur.archlinux.org/cgit/aur.git/snapshot/pick.tar.gz
tar -xzf pick.tar.gz
cd pick
makepkg -sri
```

### Debian and Ubuntu

A package for Pick is available as of [Debian 9] and [Ubuntu 15.10].

```sh
apt-get install pick
```

### Mac OS X via Homebrew

```sh
brew tap thoughtbot/formulae
brew install pick
```

### Mac OS X via MacPorts

```sh
sudo port install pick
```

### From source

1. Download the latest release from [the releases page], optionally check the
   signature and extract the archive:

```sh
wget https://github.com/thoughtbot/pick/releases/download/vVERSION/pick-VERSION.tar.gz
wget https://github.com/thoughtbot/pick/releases/download/vVERSION/pick-VERSION.tar.gz.asc
gpg --verify pick-VERSION.tar.gz.asc
tar -xzf pick-VERSION.tar.gz
```

2. Follow the bundled installation instructions:

```sh
cd pick-VERSION
less INSTALL.md
```

[the AUR]: https://aur.archlinux.org/packages/pick/
[Debian 9]: https://packages.debian.org/stretch/pick
[Ubuntu 15.10]: http://packages.ubuntu.com/wily/pick
[the releases page]: https://github.com/thoughtbot/pick/releases/

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

[the pick.vim Vim plugin]: https://github.com/thoughtbot/pick.vim/
[the pick.vim source code]: https://github.com/thoughtbot/pick.vim/blob/master/plugin/pick.vim

## Copyright

Copyright (c) 2015 Calle Erlandsson & thoughtbot, Inc.

Lead by Calle Erlandsson & thoughtbot, Inc.
