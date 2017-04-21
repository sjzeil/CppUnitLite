CPPFLAGS=-g -std=c++11
LFLAGS=-pthread
#
########################################################################
# Macro definitions for "standard" C and C++ compilations
#
CFLAGS=-g
TARGET=testUnitTest
CPPS=$(wildcard *.cpp)
LINK=g++ $(CPPFLAGS)
#
CC=gcc
CXX=g++
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

$(TARGET): $(OBJS)
	$(LINK) $(FLAGS) -o $(TARGET) $^ $(LFLAGS)

all: $(TARGET) documentation
	./$(TARGET)

clean:
	-/bin/rm -rf *.d *.o $(TARGET) docs

documentation:
	-mkdir docs
	doxygen Doxyfile



make.dep: $(DEPENDENCIES)
	-cat $(DEPENDENCIES) > $@

include make.dep
