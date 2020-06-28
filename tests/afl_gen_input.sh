#!/usr/bin/env bash

set -e

testdir="$(dirname "$0")"

export output_dir="${1:?missing output dir}"

mkdir -p -- "$output_dir"

find "$testdir" -name '*.json' -exec sh -c '
     set -e
     test_name="$(basename "$1" .json)"
     jq -r .stdin "$1" > "${output_dir}/${test_name}"
' -- {} \;
