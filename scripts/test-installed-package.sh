#!/bin/sh
set -eux

if test "${UPGRADE_GUARD_DISPOSABLE:-0}" != 1; then
  echo "Refusing package mutation outside an explicitly disposable container/VM." >&2
  exit 1
fi
test $# -eq 1 || { echo "Usage: $0 PACKAGE.deb" >&2; exit 64; }
package=$1
dpkg -i "$package"
dpkg -L upgrade-guard
upgrade-guard --version
upgrade-guard --help >/dev/null
upgrade-guard list-checks | grep -q UG-RBT-001
test -x /usr/bin/upgrade-guard
dpkg -L upgrade-guard | grep -Eq '^/usr/share/man/man1/upgrade-guard\.1(\.gz)?$'
test -r /usr/share/bash-completion/completions/upgrade-guard
test -r /usr/share/upgrade-guard/schemas/report-v1.schema.json
dpkg -r upgrade-guard
dpkg -i "$package"
upgrade-guard scan --format json >/tmp/upgrade-guard-installed.json || test "$?" -le 3
python3 -m json.tool /tmp/upgrade-guard-installed.json >/dev/null
