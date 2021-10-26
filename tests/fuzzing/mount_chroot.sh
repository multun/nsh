#!/usr/bin/env bash

testdir="$(dirname "$0")"
rootdir="${testdir}/.."
chroot="$rootdir"/chroot

mkdir -p "$chroot"

sudo mount -t tmpfs -o size=512m tmpfs "$chroot"

mkdir -p "$chroot"/{dev,etc,proc,nix}

touch "$chroot"/etc/resolv.conf

for path in /proc \
            /dev \
            /etc/resolv.conf \
            /nix
do
    sudo mount -o bind,ro "$path" "$chroot/$path"
done
