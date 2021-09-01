#include "Parser.hpp"
#include <chrono>
#include <iostream>

void BenchmarkExpressionConstructor()
{
	for (int i = 0; i < 100000; i++)
	{
		auto expr = std::make_unique<
			OperatorExponent>(
				Variable('a'),
				OperatorExponent(
					Variable('b'),
					OperatorExponent(
						OperatorMinus(
							OperatorDivide(
								OperatorDivide(
									Constant(32),
									Variable('d')),
								Variable('e')),
							Variable('f')),
						OperatorMinus(
							OperatorMultiply(
								Variable('x'),
								Constant(31)),
							OperatorMultiply(
								Variable('m'),
								Variable('n'))))));
	}
}

auto timeFuncInvocation = [](auto&& func, auto&&... params)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
};

int main()
{
	double total = 0;

	for (int i = 0; i < 10; i++)
	{
		auto time = timeFuncInvocation(BenchmarkExpressionConstructor);
		std::cout << "Time difference = " << time << "[s]" << std::endl;
		total += time;
	}

	std::cout << "-----------------" << std::endl;
	std::cout << "Average = " << total / 10 << "[s]" << std::endl;
}