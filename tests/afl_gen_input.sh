#!/usr/bin/env bash

set -e

testdir="$(dirname "$0")"
rootdir="$testdir"/..
chroot="$rootdir"/chroot

export output_dir="${1:-big_in}"
export cmin_dir="${2:-in}"
export chroot

mkdir -p -- "${chroot}/${output_dir}" "${chroot}/${cmin_dir}"

find "$testdir" -name '*.json' -exec sh -c '
     set -e
     test_name="$(basename "$1" .json)"
     jq -r .stdin "$1" > "${chroot}/${output_dir}/${test_name}"
' -- {} \;

"$testdir"/run_chroot.sh \
    afl-cmin \
    -i "${output_dir}" \
    -o "${cmin_dir}" \
    ./nsh @@ cmin_dir
