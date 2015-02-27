# Pick

![pick(1) usage](screencast.gif)

The `pick(1)` utility allows users to choose one option from a set of choices
using an interface with fuzzy search functionality.

## Installation

### Arch Linux

A package for Pick is available in [the AUR].

```sh
wget https://aur.archlinux.org/packages/pi/pick/pick.tar.gz
tar -xzf pick.tar.gz
cd pick
makepkg -s
pacman -U pick-VERSION-x86_64.pkg.tar.xz
```

### Mac OS X via Homebrew

```sh
brew tap thoughtbot/formulae
brew install pick
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
[the releases page]: https://github.com/thoughtbot/pick/releases/

## Usage

`pick(1)` reads a list of choices on `stdin` and outputs the selected choice on
`stdout`. Therefore it is easily used both in pipelines and subshells:

```sh
git ls-files | pick | xargs less # Select a file in the current git repository to view in less
cd $(find . -type d | pick)      # Select a directory to cd into
```

Pick can also easily be used from within Vim with this VimL function:

```viml
function! PickCommand(choice_command, pick_args, vim_command)
  try
    let selection = system(a:choice_command . " | ./src/pick" . a:pick_args)
  catch /Vim:Interrupt/
    " Swallow the ^C so that the redraw below happens; otherwise there will be
    " leftovers from pick on the screen
    redraw!
    return
  endtry
  redraw!
  exec a:vim_command . " " . selection
endfunction
```

Then map keys to call this function:

```viml
nnoremap <leader>e :call PickCommand("git ls-files", "", ":e")<cr>
```

See the `pick(1)` man page for detailed usage instructions and more examples.

## Copyright

Copyright (c) 2014 Calle Erlandsson & thoughtbot, Inc.

Lead by Calle Erlandsson & thoughtbot, Inc.
