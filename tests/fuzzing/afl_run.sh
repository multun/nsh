#!/usr/bin/env bash
testdir="$(dirname "$0")"
AFL_SKIP_CPUFREQ=1 afl-fuzz -i in -o out "$testdir"/../nsh @@
