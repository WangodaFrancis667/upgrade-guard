#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output=${1:-"$root/artifacts/source"}
version=0.1.0~beta1
mkdir -p "$output"
archive="$output/upgrade-guard_${version}.orig.tar.gz"

tar -cf - --sort=name --mtime='UTC 2026-09-01' --owner=0 --group=0 --numeric-owner \
  --exclude=.git --exclude=build --exclude=artifacts --exclude=debian \
  --exclude='*.changes' --exclude='*.buildinfo' --exclude='*.deb' --exclude='*.dsc' \
  --transform="s,^.,upgrade-guard-${version}," -C "$root" . | gzip -n >"$archive"
printf '%s\n' "$archive"
