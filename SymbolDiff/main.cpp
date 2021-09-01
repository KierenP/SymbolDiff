#include "Parser.hpp"
#include <iostream>

int main()
{
	std::string input;

	while (true)
	{
		std::cout << "> f (x) = ";
		std::cin >> input;

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