#!/bin/sh
cd $1
g++ -o runTest  -std=c++14 -pthread simpleTest.cpp unittest.cpp
gdb -batch -command=gdbCommands.txt  -exec=./runTest > captured.txt
