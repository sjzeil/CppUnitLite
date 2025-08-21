#!/bin/sh
cd $1
g++ -o runTest  -std=c++14 -pthread simpleTest.cpp unittest.cpp
./runTest > captured.txt
