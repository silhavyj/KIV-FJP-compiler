#!/bin/bash

mkdir -p build >/dev/null 2>&1
cd build
cmake ..
make