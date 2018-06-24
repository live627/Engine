#ifndef __ELAPSED_H__
#define __ELAPSED_H__

/*
Simple C++11 timer class to profile your function calls, etc.
Usage:
Elapsed e;

e.begin();
myFunctionToBeProfiled();
auto microseconds = e.end();
*/


#include <chrono>

class Elapsed {
protected:
	std::chrono::high_resolution_clock::time_point _begin;
public:
	void begin() { _begin = std::chrono::high_resolution_clock::now(); }
	std::chrono::microseconds end() {
		auto diff = std::chrono::high_resolution_clock::now() - _begin;
		return std::chrono::duration_cast<std::chrono::microseconds>(diff);
	}
};

#endif