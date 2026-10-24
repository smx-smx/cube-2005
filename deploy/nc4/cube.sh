#!/bin/sh
# Cube for LG NetCast 4.x, started by the TV app manager (e.g. webos-userland's
# "nc4app run cube/cube.sh"). The --service/--auid arguments must reach cube:
# SDL's nc4 video driver registers with them (cube ignores unknown options).
DIR=$(cd "$(dirname "$0")" && pwd -P)
cd "$DIR" || exit 1
exec ./cube "$@" >> "$DIR/cube.log" 2>&1
