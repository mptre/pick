# Contributing

## Pull Requests

We love pull requests from everyone.
By participating in this project,
you agree to abide by its [Code of Conduct][conduct].

1. Fork and clone the repo.

2. Build and make sure all tests pass:

   ```sh
   $ ./configure
   $ make test
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

Internal documentation on crafting a release.

1. Run Coverity Scan.
   In the case of discovered defects,
   perform a fix and redo this step until no further defects are detected.

   ```sh
   $ git checkout coverity_scan
   $ git rebase master
   $ git push origin coverity_scan
   ```

2. Update `CHANGELOG.md` and commit the change:

   ```sh
   $ git commit -m 'update changelog'
   ```

3. Update the version in `Makefile` and commit the change:

   ```sh
   $ git commit -m 'update version to 0.0.2'
   ```

4. Create and verify the tarball:

   ```sh
   $ make dist
   ```

5. Tag and push:

   ```sh
   $ git tag v0.0.2
   $ git push --tags origin master
   ```

6. [Announce the release on GitHub][announce].

7. Contact package maintainers:

   * Aggelos Avgerinos <evaggelos.avgerinos@gmail.com> (Debian & Ubuntu)
   * Chunyang Xu <xuchunyang.me@gmail.com> (MacPorts)
   * Fredrik Fornwall <fredrik@fornwall.net> (Homebrew)
   * Jenz Guenther <dbotw@gmx.net> (NetBSD)
   * Neel Chauhan <neel@neelc.org> (FreeBSD)
   * 6c37 team <camille@airmail.cc> (CRUX)
   * Benjamin Porter <pick@tamx.org> (unofficially Fedora/RHEL/CentOS)
   * Gustavo Costa <xfgusta@fedoraproject.org> (Fedora Linux)

   ```
   To: pick-maintainers@calleerlandsson.com
   Bcc: evaggelos.avgerinos@gmail.com, xuchunyang.me@gmail.com,
        fredrik@fornwall.net, dbotw@gmx.net, neel@neelc.org,
        camille@airmail.cc, pick@tamx.org, xfgusta@fedoraproject.org
   Subject: pick 0.0.2
   ```

[announce]: https://github.com/mptre/pick/releases/new
[conduct]: https://github.com/mptre/pick/blob/master/CODE_OF_CONDUCT.md
[manual]: https://github.com/mptre/pick/tree/master/pick.1
[style]: https://man.openbsd.org/style
[test]: https://github.com/mptre/pick/tree/master/tests#test-suite
