#!/usr/bin/env bash

testdir="$(dirname "$0")"
rootdir="${testdir}/.."
chroot="$rootdir"/chroot

sudo chroot --userspec 1000:1000 "$chroot" "$@"
