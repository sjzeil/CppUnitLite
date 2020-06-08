/**
 *  Unit test of the unit test framework
 */

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <chrono>
#include <thread>
#include <mutex>

#include <filesystem>
#include <cstdlib>

#include "unittest.h"

using namespace std;

UnitTest(testPlatformIdentification) {
#ifdef __MINGW32__
	cout << "# _MINGW32_ is defined" << endl;
#endif
#ifdef __MINGW64__
	cout << "# _MINGW64_ is defined" << endl;
#endif
#ifdef __CYGWIN__
	cout << "# _CYGWIN_ is defined" << endl;
#endif
#ifdef __amd64__
	cout << "# _amd64_ is defined" << endl;
#endif
#ifdef __i386__
	cout << "# _i386_ is defined" << endl;
#endif
    assertTrue(0 == 0);
}


void cdToProjectRoot()
{
	using namespace std::filesystem;

	path cwd = current_path();
	cout << "# current directory is " << cwd.string() << endl;
	path srcDir = cwd;
	srcDir /= "src";
	path upperLimit = "/src";
	while ((srcDir != upperLimit) && !exists(srcDir))
	{
		srcDir = srcDir.parent_path().parent_path();
		srcDir /= "src";
	}
	current_path(srcDir.parent_path());
	cout << "# switching to " << current_path().string() << endl;
}


void clearOut (string directory)
{
	using namespace std::filesystem;
    path dirPath (directory);

    if (exists(dirPath) && is_directory(dirPath))
    {
    	remove_all(dirPath);
    }
    assertFalse(exists(dirPath));

}

void copyFiles (string testDir, string destinationDir)
{
	using namespace std::filesystem;


	path testPath = testDir;
	path destPath = destinationDir;
	create_directories(destPath);
	copy (testPath, destPath, copy_options::recursive);
	path utCppPath = "src/main/cpp/unittest.cpp";
	copy (utCppPath, destPath);
	path utHPath = "src/main/public/unittest.h";
	copy (utHPath, destPath);


    assertTrue(exists(destPath));
    path script = destPath;
    script.append("runGdb.sh");
    assertTrue(exists(script));
}

void runScript (string script)
{
	using namespace std::filesystem;

#if defined(_WIN32)
	string scriptTerminator(".bat");
#elif defined(_WIN64)
	string scriptTerminator(".bat");
#elif defined(__CYGWIN__)
	string scriptTerminator(".sh");
#else
	string scriptTerminator(".sh");
#endif



	path scriptpath = absolute(path(script + scriptTerminator));
	path scriptDir = scriptpath.parent_path();

	string command = scriptpath.string() + " " + scriptDir.string();
	system(command.c_str());
}

string readFile(filesystem::path file)
{
	string result;
	ifstream in (file.string());
	string line;
	getline(in, line);
	while (in)
	{
		result += line;
		result += "\n";
		getline(in, line);
	}
	return result;
}


UnitTestTimed(testDebuggerDetection, 10000) {
	cdToProjectRoot();
	assertTrue (filesystem::exists(filesystem::path("src")));
	assertTrue (filesystem::exists(filesystem::path("src/test/data/sampleProj1")));
	clearOut ("build/testArea");
	copyFiles("src/test/data/sampleProj1", "build/testArea/");
	runScript("build/testArea/runGdb");
	filesystem::path capturedFile = "build/testArea/captured.txt";
	assertTrue(filesystem::exists(capturedFile));
	string captured = readFile(capturedFile);
	assertThat(captured, contains("# Debugger detected"));
}

UnitTestTimed(testNoDebuggerDetection, 10000) {
	cdToProjectRoot();
	clearOut ("build/testArea");
	copyFiles("src/test/data/sampleProj1", "build/testArea/");
	runScript("build/testArea/runBinary");
	filesystem::path capturedFile = "build/testArea/captured.txt";
	assertTrue(filesystem::exists(capturedFile));
	string captured = readFile(capturedFile);
	assertThat(captured, !contains("# Debugger detected"));
}

UnitTest(testThreadSupport) {
	std::mutex m;
	int timeLimit = 10000;
	std::chrono::duration<int,std::milli> limit (timeLimit);
	std::chrono::duration<int,std::milli> incr (100);
	std::chrono::duration<int,std::milli> elapsed (0);
	int testResult = -1;

	std::thread t([&m, &testResult](){
		{
			int result = 1;
			std::unique_lock<std::mutex> l2(m);
			testResult = result;
		}
	});
	t.detach();

	bool finished = false;
	do {
		{
			std::unique_lock<std::mutex> l(m);
			finished = (testResult >= -1 || elapsed >= limit);
			elapsed += incr;
		}
		std::this_thread::sleep_for( incr );
	} while (!finished);

	assertThat(testResult, isEqualTo(1));
}
