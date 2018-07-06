CC=gcc
CXX=g++
#
UsingMinGW=$(shell $(CXX) -v 2>&1 | grep "^Target: mingw" | wc -l)
ifeq ($(UsingMinGW), 1)
CPPFLAGS=-g -std=c++11
LFLAGS=
else
CPPFLAGS=-g -std=c++11
LFLAGS=-pthread
endif

#
########################################################################
# Macro definitions for "standard" C and C++ compilations
#
CFLAGS=-g
TARGET=testUnitTest
CPPS=$(wildcard *.cpp)
LINK=g++ $(CPPFLAGS)
#
#
#
#  In most cases, you should not change anything below this line.
#
#  The following is "boilerplate" to set up the standard compilation
#  commands:
#


OBJS=$(CPPS:%.cpp=%.o)
DEPENDENCIES = $(CPPS:%.cpp=%.d)



%.d: %.cpp
	touch $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -MMD -o $@ -c $*.cpp

# 
# Targets:
# 

$(TARGET): testStringRepr.o testMatchers.o testUnitTest.o unittest.o
	$(LINK) $(FLAGS) -o $(TARGET) $^ $(LFLAGS)


demoFailures: demoFailures.o unittest.o
	$(LINK) $(FLAGS) -o $@ $^ $(LFLAGS)



all: $(TARGET) demoFailures
#	./$(TARGET)






clean:
	-/bin/rm -rf *.d *.o $(TARGET) docs

documentation:
	-mkdir docs
	doxygen Doxyfile



make.dep: $(DEPENDENCIES)
	-cat $(DEPENDENCIES) > $@

include make.dep
