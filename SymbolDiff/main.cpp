#include <iostream>

#include "Algorithms.h"
#include "Benchmark.h"

int main()
{
	std::cout << "Benchmark: " << Benchmark(Differentiate, 100000, "(x+1)^2/(x-1)^2", 'x') << "ns\n";

	std::string input;
	
	while (std::cout << "> f (x) = ", std::getline(std::cin, input))
	{
		std::string answer;

		try
		{
			answer = Differentiate(input, 'x');
		}
		catch (const std::exception& e)
		{
			answer = e.what();
		}
		catch (...)
		{
			answer = "Unhandled exception";
		}
		
		std::cout << "< f'(x) = " << answer << "\n\n";
	}
}
