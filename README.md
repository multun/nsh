# Introduction

nsh, also known as dish, is a work in progress POSIX shell command language
interpreter. It can execute commands, perform IO redirections and more.


# Building

```sh
meson setup builddir
meson compile -C builddir
```

To install:

```sh
meson install -C build --destdir DESTINATION_DIRECTORY
```


# Testing

```sh
make check
# or
tests/check.py -q
```


# Usage

```sh
nsh [--norc] [--token-print | --ast-print] [ -c COMMAND | FILE ]
```

For extended usage information, see
```sh
man doc/nsh.1
```


# Continuous integration

A buildbot configuration is available at `tests/master.cfg`
