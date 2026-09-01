#!/bin/sh
set -eu

release=${UG_RELEASE:?UG_RELEASE is required}
work=/work/upgrade-guard
log_dir="/artifacts/docker/$release"
mkdir -p "$work" "$log_dir"
rsync -rlt --delete --exclude build --exclude artifacts --exclude .git /src/ "$work/"
cd "$work"
exec >"$log_dir/test.log" 2>&1

case "$release" in
  22.04) series=jammy ;;
  24.04) series=noble ;;
  26.04) series=resolute ;;
esac
sed -i "1s/ unstable;/ $series;/" debian/changelog

echo "Ubuntu release: $release"
uname -a
cmake -S . -B build/container -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/container
ctest --test-dir build/container --output-on-failure
build/container/upgrade-guard --version
build/container/upgrade-guard --help >/dev/null
build/container/upgrade-guard list-checks | grep -q UG-RBT-001
build/container/upgrade-guard scan --format json >"$log_dir/report.json" || test "$?" -le 3
python3 -m json.tool "$log_dir/report.json" >/dev/null
python3 - <<'PY'
import json
from jsonschema import validate
with open('/artifacts/docker/' + __import__('os').environ['UG_RELEASE'] + '/report.json') as report:
    instance = json.load(report)
with open('schemas/report-v1.schema.json') as schema:
    validate(instance, json.load(schema))
PY
dpkg-buildpackage -b -us -uc
package=$(find /work -maxdepth 1 -name 'upgrade-guard_*_*.deb' -type f | head -1)
test -n "$package"
UPGRADE_GUARD_DISPOSABLE=1 scripts/test-installed-package.sh "$package"
lintian --pedantic /work/upgrade-guard_*_*.changes
if ! autopkgtest /work/upgrade-guard_*_*.changes -- null >"$log_dir/autopkgtest.log" 2>&1; then
  grep -Eq '^smoke[[:space:]]+PASS' "$log_dir/autopkgtest.log"
fi
cp "$package" "$log_dir/"
cp /work/upgrade-guard_*_*.changes /work/upgrade-guard_*_*.buildinfo "$log_dir/"
echo "Container validation completed for Ubuntu $release"
