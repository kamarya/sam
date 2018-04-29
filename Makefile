all:
	g++ sam.cpp utility.cpp main.cxx -o main -O3 -std=c++11 -lpthread
clean:
	rm -f main
doxygen:
	doxygen -s doxygen.cfg
