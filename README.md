# HST refinement checker

HST is an open-source refinement checker for the CSP (Communicating Sequential
Processes) process calculus.  The CSP language and the refinement algorithm that
HST implements are both described in [*The Theory and Practice of
Concurrency*](https://www.cs.ox.ac.uk/bill.roscoe/publications/68b.pdf), by Bill
Roscoe.

This project is not meant to be a replacement for [FDR][], the canonical CSP
refinement checker.  It's more a playground for me to learn more about the
refinement algorithm by trying to implement it.  If it turns out to be generally
useful, that would be a pleasant side effect!

[FDR]: http://www.cs.ox.ac.uk/projects/fdr/

## Building

HST is implemented in C, and uses the [autotools][] for its build environment.
Its only dependency is on [Judy][], which provides some useful data structures.
To install it, look for a package called `judy`, `libjudy` or `libjudy-dev` in
your package manager.

As an example, to install all of the build tools and dependencies on [Arch
Linux][], you'd use something like:

    $ sudo pacman -S base-devel judy

[Arch Linux]: https://www.archlinux.org/
[autotools]: https://autotools.io/index.html
[Judy]: http://judy.sourceforge.net/

Once all of the dependencies are installed, build the code:

    $ cd [path to local copy of repo]
    $ mkdir .build
    $ cd .build
    $ ../autogen.sh
    $ ../configure
    $ make
    $ make check

## Hacking

If you want to hack on the code, please feel free!  *[TODO: Write an overview
map of all of the existing pieces of code.]*  You can run `make && make check`
as much as you want from in your `.build` directory; if you're adding
interesting new features or fixing any bugs, please make sure to add test cases!

You'll probably also want to install [Valgrind][], to ensure that there are no
memory leaks.  If Valgrind is installed, `configure` will automatically activate
a new `make check-valgrind` target.  This runs all of the same tests as `make
check`, but inside of Valgrind, to make sure that there are no memory leaks or
corruption errors.

[Valgrind]: http://valgrind.org/
