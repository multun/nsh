# Introduction

nsh is a work in progress POSIX shell command language interpreter.
It can execute commands, perform IO redirections and more.

# Building

```sh
meson setup builddir
meson compile -C builddir  # or ninja -C builddir
builddir/nsh -c 'echo "Hello world!"'
```

Installing is done using `meson install` (you probably shouldn't use this command directly unless you know what you're doing).

# Running tests

```sh
tests/run_tests -q builddir/nsh
```

# Building documentation

```sh
# enable documentation support
meson setup -Ddoc=true builddir  # --reconfigure might be needed
# build the documentation
meson compile -C builddir doxygen_doc
# open the documentation in the browser
xdg-open builddir/doxygen_doc/index.html
```

# Debugging

```
meson setup --buildtype=debug --werror --warnlevel=3 debugbuild
meson compile -C debugbuild
gdb -arg debugbuild/nsh -c 'echo test'
```
