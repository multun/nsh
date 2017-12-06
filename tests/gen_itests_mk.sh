#!/bin/sh

cd "$(dirname "$0")"
echo 'itest_files = \'
find -name '*.json' -type f | sed 's/^/    \$\(srcdir\)\/tests\//;$q;s/$/ \\/'
