#pragma once

#include <chrono>
#include <utility>

template<typename F, typename... Args>
double funcTime(F func, Args&&... args) 
{
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	func(std::forward<Args>(args)...);
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - t1).count();
}

template<typename F, typename... Args>
double Benchmark(F func, int rep, Args&&... args)
{
	double total = 0;

	for (int i = 0; i < rep; i++)
	{
		total += funcTime(func, std::forward<Args>(args)...);
	}

	return total / rep;
}

