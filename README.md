# Introduction

nsh, also known as dish, is a work in progress POSIX shell command language
interpreter. It can execute commands, perform IO redirections and more.


# Building

```sh
meson setup builddir
meson compile -C builddir
```

Installing is done using `meson install` (you probably shouldn't use this command directly unless you know what you're doing).

# Running tests

```sh
tests/run_tests -q builddir/nsh
```
