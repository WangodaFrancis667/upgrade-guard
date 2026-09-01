#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$root/artifacts/docker"
echo "Docker matrix: Ubuntu 22.04, 24.04, 26.04; unprivileged runtime; host kernel shared"
docker compose -f "$root/docker/compose.yaml" build --no-cache
for service in ubuntu-2204 ubuntu-2404 ubuntu-2604; do
  echo "Running $service"
  docker compose -f "$root/docker/compose.yaml" run --rm "$service"
done
echo "Logs: $root/artifacts/docker"
