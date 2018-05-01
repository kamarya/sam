all:
	g++ sam.cpp utility.cpp main.cxx -o samx -O3 -Wall -std=c++11 -lpthread
clean:
	rm -f samx
doxygen:
	doxygen -s doxygen.cfg
