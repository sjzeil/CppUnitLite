cd %1
g++ -o runTest.exe  -std=c++14 simpleTest.cpp unittest.cpp
runTest.exe > captured.txt
