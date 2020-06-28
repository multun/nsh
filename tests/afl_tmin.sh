#!/usr/bin/env bash
testdir="$(dirname "$0")"
afl-tmin -i "${1:?missing input}" -o "${2:?missing output}" -- "$testdir"/../nsh @@
