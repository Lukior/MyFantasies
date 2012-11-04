
#pragma once

#include <iostream>
#include <chrono>

class Timer
{
private:
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point stop;

public:
	Timer()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	void Start()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	long long Stop()
	{
		stop = std::chrono::high_resolution_clock::now();

		return std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
	}

	void PrintTime()
	{
		std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
		std::cout << (ns.count() / 1000000000) << " s, " << (ns.count() / 1000000) % 1000 << " ms, " << (ns.count() / 1000) % 1000 << " us, " << ns.count() % 1000 << " ns" << std::endl;
	}
};
