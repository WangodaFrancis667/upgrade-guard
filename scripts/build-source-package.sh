#!/bin/sh
set -eu

usage() {
  echo "Usage: $0 noble|resolute [--key KEYID] [--force] [--output DIR]" >&2
  exit 64
}

test $# -ge 1 || usage
series=$1
shift
case "$series" in
  noble) deb_version=0.1.0~beta1-1~ubuntu24.04.1 ;;
  resolute) deb_version=0.1.0~beta1-1~ubuntu26.04.1 ;;
  *) usage ;;
esac

key=
force=0
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output="$root/artifacts/source/$series"
while test $# -gt 0; do
  case "$1" in
    --key) test $# -ge 2 || usage; key=$2; shift 2 ;;
    --force) force=1; shift ;;
    --output) test $# -ge 2 || usage; output=$2; shift 2 ;;
    *) usage ;;
  esac
done

if test "$force" -ne 1 && test -n "$(git -C "$root" status --porcelain)"; then
  echo "Refusing a dirty release tree; commit changes or pass --force." >&2
  exit 1
fi

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
source_dir="$work/upgrade-guard-0.1.0~beta1"
mkdir -p "$source_dir" "$output"
tar --exclude=.git --exclude=build --exclude=artifacts -C "$root" -cf - . | tar -C "$source_dir" -xf -
sed -i "1s/(0.1.0~beta1-1) unstable;/(${deb_version}) ${series};/" "$source_dir/debian/changelog"
"$source_dir/scripts/create-orig-tarball.sh" "$work"

cd "$source_dir"
if test -n "$key"; then
  dpkg-buildpackage -S -k"$key"
else
  dpkg-buildpackage -S -us -uc
fi
cp "$work"/upgrade-guard_0.1.0~beta1.orig.tar.gz "$output/"
cp "$work"/upgrade-guard_"$deb_version".dsc "$output/"
cp "$work"/upgrade-guard_"$deb_version".debian.tar.* "$output/"
cp "$work"/upgrade-guard_"$deb_version"_source.changes "$output/"
cp "$work"/upgrade-guard_"$deb_version"_source.buildinfo "$output/"
printf 'Source artifacts: %s\n' "$output"
printf 'Manual upload after verification: dput <ppa-target> %s/upgrade-guard_%s_source.changes\n' "$output" "$deb_version"
