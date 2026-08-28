#!/usr/bin/env bash
DIR="$(cd "$(dirname "$0")" && pwd)"
ARGS=("$@")

for i in "${!ARGS[@]}"; do
  if [ "${ARGS[$i]}" = "gen" ]; then
    ARGS[$i]="gmake"
  fi
done

exec "$DIR/premake5" --file="$DIR/premake5.lua" "${ARGS[@]}"
