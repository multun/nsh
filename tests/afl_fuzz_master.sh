#!/usr/bin/env bash

testdir="$(dirname "$0")"
exec "$testdir"/run_chroot.sh afl-fuzz -i in -o out -M master ./nsh @@
