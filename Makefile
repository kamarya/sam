all:
	g++ sam.cpp utility.cpp main.cxx -o main -O3 -std=c++11
clean:
	rm -f main
