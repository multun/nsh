#!/bin/sh

rootdir="$(dirname "$0")"

exec valgrind --leak-check=full --show-leak-kinds=all --suppressions="$rootdir"/valgrind.supp "$@"
