#!/bin/sh
# Cube on a PC: windowed 1280x720 by default, cube's own options (-t windowed, -w/-h size,
# -d dedicated server...) are passed through.
DIR=$(cd "$(dirname "$0")" && pwd -P)
cd "$DIR" || exit 1
[ $# -eq 0 ] && set -- -t -w1280 -h720
exec ./cube "$@"
