# Developing

## Contributing

We love pull requests from everyone.
By participating in this project,
you agree to abide by its [Code of Conduct][conduct].

1. Fork and clone the repo.

2. Build and make sure all tests pass:

   ```sh
   $ ./autogen.sh
   $ ./configure
   $ make check
   ```

3. Make your changes.

   Respect the existing formatting and indentation,
   when in doubt consult [style(9)][style].

4. If your changes can be captured by a [test],
   make sure to add one.
   Changes that only concern the interface can be harder to test but feel free
   to ask for help.

   Again,
   make sure all tests still pass.

5. If your changes for instance adds an option or key binding,
   make sure to update the [manual].

6. Submit a pull request on GitHub.
   This will in addition to sharing your work run it through our continuous
   integration which performs both static and runtime analysis in order to catch
   bugs early on.

7. At this point you're waiting on us.

## Release

1. Run Coverity Scan.
   In the case of discovered defects,
   perform a fix and redo this step until no further defects are detected.

   ```sh
   $ git checkout coverity_scan
   $ git rebase master
   $ git push origin coverity_scan
   ```

2. Update the version in `configure.ac`:

   ```
   AC_INIT([pick], [0.0.2], [pick-maintainers@calleerlandsson.com])
   ```

3. Create and verify the tarball:

   ```sh
   $ make distcheck
   $ tar tvzf pick-0.0.2.tar.gz
   ```

4. Generate checksum:

   ```sh
   $ sha256 pick-0.0.2.tar.gz >pick-0.0.2.sha256
   ```

5. Tag the repo:

   ```sh
   $ git tag v0.0.2
   $ git push origin master --tags
   ```

6. [Announce the release on GitHub][announce].

7. Contact package maintainers:

  * Aggelos Avgerinos <evaggelos.avgerinos@gmail.com> (Debian & Ubuntu)
  * Chunyang Xu <xuchunyang.me@gmail.com> (MacPorts)
  * Fredrik Fornwall <fredrik@fornwall.net> (Homebrew)
  * Neel Chauhan <neel@neelc.org> (FreeBSD)

[announce]: https://github.com/calleerlandsson/pick/releases/new
[conduct]: https://github.com/calleerlandsson/pick/blob/master/CODE_OF_CONDUCT.md
[manual]: https://github.com/calleerlandsson/pick/tree/master/pick.1
[style]: https://man.openbsd.org/style
[test]: https://github.com/calleerlandsson/pick/tree/master/tests#test-suite
