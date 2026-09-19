#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "$0")" && pwd)"
PREMAKE_BIN="${PREMAKE:-}"

if [ -z "$PREMAKE_BIN" ]; then
  if command -v premake5 >/dev/null 2>&1; then
    PREMAKE_BIN="premake5"
  elif [ -x "$DIR/premake5" ]; then
    PREMAKE_BIN="$DIR/premake5"
  else
    echo "premake5 was not found; install Premake 5 or set PREMAKE" >&2
    exit 1
  fi
fi

ARGS=("$@")

for i in "${!ARGS[@]}"; do
  if [ "${ARGS[$i]}" = "gen" ]; then
    ARGS[$i]="gmake"
  fi
done

exec "$PREMAKE_BIN" --file="$DIR/premake5.lua" "${ARGS[@]}"
