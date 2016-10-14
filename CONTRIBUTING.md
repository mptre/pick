We love pull requests from everyone. By participating in this project, you agree
to abide by its [code of conduct].

[code of conduct]: https://github.com/calleerlandsson/pick/blob/master/CODE_OF_CONDUCT.md

## Contributing

- Fork and clone the repo.
- Set up your build environment in accordance to the instructions in
  `DEVELOPING.md`.
- Make your changes.
- Update `pick.1` and `README.md` in accordance to your changes.
- Make sure `pick(1)` builds and runs as expected: `make && printf
  "hello\ngoodbye" | src/pick`.
- Make sure `make distcheck` runs successfully.
- Submit a pull request on GitHub.

At this point you're waiting on us. We like to at least comment on, if not
accept, pull requests within three business days (and, typically, one business
day). We may suggest some changes or improvements or alternatives.
