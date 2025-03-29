#!/bin/bash
# r2 -r ~/rev/profile.rr2 -d $(./run.sh)

TTY=/dev/pts/0

LD_LIBRARY_PATH=$(pwd)/glibc-2.34/build/glibc-2.34-local/lib ./wrecked <$TTY >$TTY 2>&1 &

echo $!
