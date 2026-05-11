#
# PROGRAM : cpu-scheduler-simulation
# AUTHOR : Cyrus Johnson
#

CXX=g++
CXXFLAGS=-Wall -Wextra -std=c++11

cpu-scheduler-simulation: cpu-scheduler-simulation.o
	$(CXX) $(CXXFLAGS) -o cpu-scheduler-simulation cpu-scheduler-simulation.o

cpu-scheduler-simulation.o: cpu-scheduler-simulation.cpp
	$(CXX) $(CXXFLAGS) -c cpu-scheduler-simulation.cpp

clean:
	rm -f *.o
	rm -f cpu-scheduler-simulation