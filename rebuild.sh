#!/bin/sh

rm -rvf build/
mkdir build/
cd build/
cmake .. && make
