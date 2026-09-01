#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
cmake --preset release
cmake --build --preset release
ctest --preset release --output-on-failure
find include src modules tests -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 |
  xargs -0 awk 'FNR==1 { if (NR>1 && lines>400) { print previous ": " lines; failed=1 } previous=FILENAME; lines=0 } { lines++ } END { if (lines>400) { print previous ": " lines; failed=1 } exit failed }'
if rg -n 'system\(|popen\(|/bin/sh|sh -c|sudo' include src modules; then
  echo "Forbidden shell invocation found." >&2
  exit 1
fi
marker_pattern='TO''DO|FIX''ME|HA''CK|PLACE''HOLDER|NOT_''IMPLEMENTED'
if rg -n "$marker_pattern" include src modules tests; then
  echo "Unresolved implementation marker found." >&2
  exit 1
fi
