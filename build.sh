#!/usr/bin/env bash

dirname=$(dirname $0)
cd $dirname
test -d build/ || mkdir build/
cd build/
cmake .. && make
echo "Executable in $dirname/build/bin/"
