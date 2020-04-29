# pick

![pick(1) usage](screencast.gif)

[pick(1)][pick]
reads a list of choices from `stdin` and outputs the selected choice to
`stdout`.
Therefore it is easily used both in pipelines and subshells:

```sh
# Select a file in the current directory to open using xdg-open(1):
$ find . -type f | pick | xargs xdg-open
# Select a command from the history to execute:
$ eval $(fc -ln 1 | pick)
```

[pick(1)][pick] can also be used from within Vim,
see the
[pick.vim][pick-vim]
plugin.

## Installation

### CRUX

Available in [`6c37/crux-ports`][crux].

### Debian and Ubuntu

A package for Pick is available As of [Debian 9][debian]
and [Ubuntu 16.04 LTS][ubuntu].

```sh
apt-get install pick
```

### Gentoo

Package is available from the [official repo][gentoo].

```sh
emerge pick
```

### Void Linux

Package is available from the [official repo][void].

```sh
xbps-install pick
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

### NetBSD via pkgsrc

```sh
pkg_add pick
```

### OpenBSD

```sh
pkg_add pick
```

### Fedora/RHEL/CentOS

The RPM builds are not yet in the official repositories, but you can install a community-built RPM.

If you are running Fedora 32 on x86_64, you can `wget` and install with `dnf`:

```sh
wget 'https://github.com/FreedomBen/pick-rpm/releases/download/v4.0.0/pick-4.0.0-1.fc32.x86_64.rpm'
sudo dnf install pick-4.0.0-1.fc32.x86_64.rpm
```

Otherwise if running CentOS, Fedora 30 or 31, take a look at the [RPM project releases page](https://github.com/FreedomBen/pick-rpm/releases).

For more information or to contribute to the RPM build, see the [RPM project page](https://github.com/FreedomBen/pick-rpm).


### From source

```sh
./configure
make install
```

## Copyright

Copyright (c) 2019 Calle Erlandsson, Anton Lindqvist & thoughtbot.

[crux]: https://github.com/6c37/crux-ports
[current]: https://github.com/mptre/pick/blob/master/CONTRIBUTING.md
[debian]: https://packages.debian.org/stable/misc/pick
[gentoo]: https://packages.gentoo.org/packages/sys-apps/pick
[pick-vim]: https://github.com/calleerlandsson/pick.vim
[pick]: https://www.basename.se/pick/
[release]: https://github.com/mptre/pick/releases/
[ubuntu]: https://packages.ubuntu.com/xenial/pick
[void]: https://github.com/void-linux/void-packages/blob/master/srcpkgs/pick/template
