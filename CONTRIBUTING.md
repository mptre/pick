Contributing
============

Pull requests are very welcome. Here's a quick guide on how to contribute:

- Fork and clone the repo.
- Set up your build environment in accordance to the instructions in
  `DEVELOPING.md`.
- Make your changes.
- Update `man/man1/pick.1` and `README.md` in accordance to your changes.
- Make sure `pick(1)` builds and runs as expected: `make && printf
  "hello\ngoodbye" | src/pick`.
- Make sure `make distcheck` runs successfully.
- Submit a pull request on GitHub.
