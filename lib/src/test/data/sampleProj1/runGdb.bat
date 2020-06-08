cd %1
g++ -o runTest.exe  -std=c++14 simpleTest.cpp unittest.cpp
gdb -batch -command=gdbCommands.txt  -exec=runTest.exe > captured.txt
