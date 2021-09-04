#include "Algorithms.h"
#include <iostream>

int main()
{
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