#include "CppUnitTest.h"

#include "..\SymbolDiff\Algorithms.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Parser
{
	TEST_CLASS(buildExpressionTree)
	{
	public:

		TEST_METHOD(BasicExpression)
		{
			auto actual = BuildExpression(Tokenize("a+b"));

			decltype(actual) expected = 
				std::make_unique<OperatorPlus>(
					std::make_unique<Variable>('a'),
					std::make_unique<Variable>('b'));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(OrderOfOperations1)
		{
			auto actual = BuildExpression(Tokenize("a*b+c"));

			decltype(actual) expected = 
				std::make_unique<OperatorPlus>(
					std::make_unique<OperatorMultiply>(
						std::make_unique<Variable>('a'),
						std::make_unique<Variable>('b')),
					std::make_unique<Variable>('c'));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(OrderOfOperations2)
		{
			auto actual = BuildExpression(Tokenize("c+a*b"));

			decltype(actual) expected = 
				std::make_unique<OperatorPlus>(
					std::make_unique<Variable>('c'),
					std::make_unique<OperatorMultiply>(
						std::make_unique<Variable>('a'),
						std::make_unique<Variable>('b')));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(OrderOfOperations3)
		{
			auto actual = BuildExpression(Tokenize("(c+a)*b"));

			decltype(actual) expected =
				std::make_unique<OperatorMultiply>(
					std::make_unique<OperatorPlus>(
						std::make_unique<Variable>('c'),
						std::make_unique<Variable>('a')),
					std::make_unique<Variable>('b'));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(rightAssociativeOrdering)
		{
			//a^b^c is a^(b^c) not (a^b)^c

			auto actual = BuildExpression(Tokenize("a^b^c"));

			decltype(actual) expected =
				std::make_unique<OperatorExponent>(
					std::make_unique<Variable>('a'),
					std::make_unique<OperatorExponent>(
						std::make_unique<Variable>('b'),
						std::make_unique<Variable>('c')));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(rightAssociativeOrderingWithBrackets)
		{
			auto actual = BuildExpression(Tokenize("(a^b)^c"));

			decltype(actual) expected =
				std::make_unique<OperatorExponent>(
					std::make_unique<OperatorExponent>(
						std::make_unique<Variable>('a'),
						std::make_unique<Variable>('b')),
					std::make_unique<Variable>('c'));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(complexExpression)
		{
			auto actual = BuildExpression(Tokenize("(a^b^(32/d/e-f)^(x*31-m*n))"));

			decltype(actual) expected =
				std::make_unique<OperatorExponent>(
					std::make_unique<Variable>('a'),
					std::make_unique<OperatorExponent>(
						std::make_unique<Variable>('b'),
						std::make_unique<OperatorExponent>(
							std::make_unique<OperatorMinus>(
								std::make_unique<OperatorDivide>(
									std::make_unique<OperatorDivide>(
										std::make_unique<Constant>(32),
										std::make_unique<Variable>('d')),
									std::make_unique<Variable>('e')),
								std::make_unique<Variable>('f')),
							std::make_unique<OperatorMinus>(
								std::make_unique<OperatorMultiply>(
									std::make_unique<Variable>('x'),
									std::make_unique<Constant>(31)),
								std::make_unique<OperatorMultiply>(
									std::make_unique<Variable>('m'),
									std::make_unique<Variable>('n'))))));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(multivariable_expression)
		{
			auto actual = BuildExpression(Tokenize("3ax^a"));

			decltype(actual) expected =
				std::make_unique<OperatorMultiply>(
					std::make_unique<OperatorMultiply>(
						std::make_unique<Constant>(3),
						std::make_unique<Variable>('a')),
					std::make_unique<OperatorExponent>(
						std::make_unique<Variable>('x'), 
						std::make_unique<Variable>('a')));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(multivariable_expression_with_brackets)
		{
			auto actual = BuildExpression(Tokenize("3(a(x^a))"));

			decltype(actual) expected =
				std::make_unique<OperatorMultiply>(
					std::make_unique<Constant>(3),
					std::make_unique<OperatorMultiply>(
						std::make_unique<Variable>('a'),
						std::make_unique<OperatorExponent>(
							std::make_unique<Variable>('x'),
							std::make_unique<Variable>('a'))));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(unary_minus)
		{
			auto actual = BuildExpression(Tokenize("-x"));

			decltype(actual) expected =
				std::make_unique<OperatorUnaryMinus>(
					std::make_unique<Variable>('x'));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(unary_minus_in_expression)
		{
			auto actual = BuildExpression(Tokenize("y*-x"));

			decltype(actual) expected = 
				std::make_unique<OperatorMultiply>(
					std::make_unique<Variable>('y'),
					std::make_unique<OperatorUnaryMinus>(
						std::make_unique<Variable>('x')));

			Assert::IsTrue(*actual == *expected);
		}

		TEST_METHOD(invalid_expressions_1)
		{
			bool threwError;

			try
			{
				threwError = false;
				BuildExpression(Tokenize("y++x"));
			}
			catch (const std::invalid_argument&)
			{
				threwError = true;
			}
			
			Assert::IsTrue(threwError);
		}

		TEST_METHOD(invalid_expressions_2)
		{
			bool threwError;

			try
			{
				threwError = false;
				BuildExpression(Tokenize("(x"));
			}
			catch (const std::invalid_argument&)
			{
				threwError = true;
			}

			Assert::IsTrue(threwError);
		}

		TEST_METHOD(invalid_expressions_3)
		{
			bool threwError;

			try
			{
				threwError = false;
				BuildExpression(Tokenize("x)"));
			}
			catch (const std::invalid_argument&)
			{
				threwError = true;
			}

			Assert::IsTrue(threwError);
		}

		TEST_METHOD(invalid_expressions_4)
		{
			bool threwError;

			try
			{
				threwError = false;
				BuildExpression(Tokenize("y#x"));
			}
			catch (const std::invalid_argument&)
			{
				threwError = true;
			}

			Assert::IsTrue(threwError);
		}

		TEST_METHOD(invalid_expressions_5)
		{
			bool threwError;

			try
			{
				threwError = false;
				BuildExpression(Tokenize("y()"));
			}
			catch (const std::invalid_argument&)
			{
				threwError = true;
			}

			Assert::IsTrue(threwError);
		}

		TEST_METHOD(invalid_expressions_6)
		{
			bool threwError;

			try
			{
				threwError = false;
				BuildExpression(Tokenize("3 3"));
			}
			catch (const std::invalid_argument&)
			{
				threwError = true;
			}

			Assert::IsTrue(threwError);
		}
	};

	TEST_CLASS(expression_Print)
	{
	public:

		TEST_METHOD(complexExpression)
		{
			std::string input = "a^b^(32/d/e-f)^(x*31-m*n)";

			auto actual = BuildExpression(Tokenize(input))->Print();
			decltype(actual) expected = "a^b^(32/d/e-f)^(31x-mn)";

			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(powerAndProduct)
		{
			std::string input = "3x^5";

			auto actual = BuildExpression(Tokenize(input))->Print();
			decltype(actual) expected = input;

			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(unaryMinusWithBrackets)
		{
			std::string input = "-(b+c)";

			auto actual = BuildExpression(Tokenize(input))->Print();
			decltype(actual) expected = input;

			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(unaryMinusWithExponent)
		{
			std::string input = "-b^c";

			auto actual = BuildExpression(Tokenize(input))->Print();
			decltype(actual) expected = input;

			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(unaryMinusWithExponentAndBrackets)
		{
			std::string input = "(-b)^c";

			auto actual = BuildExpression(Tokenize(input))->Print();
			decltype(actual) expected = input;

			Assert::AreEqual(expected, actual);
		}
	};

	TEST_CLASS(expression_evaluate)
	{
	public:

		TEST_METHOD(complexExpression)
		{
			std::string input = "a^b^(32/d/e-f)^(x*31-m*n)";
			std::unordered_map<char, double> values = {
				{ 'a', 2 },
				{ 'b', 3 },
				{ 'd', 8 },
				{ 'e', 2 },
				{ 'f', 1 },
				{ 'x', 1.0 / 31.0 },
				{ 'm', 0.5 },
				{ 'n', 2 },
			};

			auto actual = BuildExpression(Tokenize(input))->Evaluate(values);
			double expected = 8.0;

			Assert::IsTrue(actual.has_value());
			Assert::AreEqual(expected, *actual);
		}

		TEST_METHOD(sqrt2)
		{
			std::string input = "2^0.5";

			auto actual = BuildExpression(Tokenize(input))->Evaluate();
			double expected = sqrt(2.0);

			Assert::IsTrue(actual.has_value());
			Assert::AreEqual(expected, *actual);
		}

		TEST_METHOD(invalidExpression)
		{
			std::string input = "x";

			auto actual = BuildExpression(Tokenize(input))->Evaluate();

			Assert::IsTrue(!actual.has_value());
		}

		TEST_METHOD(multivariable_expression)
		{
			auto actual = BuildExpression(Tokenize("3ax^a"))->Evaluate({ {'x', 2}, {'a', 3}});
			double expected = 72;

			Assert::IsTrue(actual.has_value());
			Assert::AreEqual(expected, *actual);
		}

		TEST_METHOD(unary_minus)
		{
			auto actual = BuildExpression(Tokenize("3a(-x)^a"))->Evaluate({ {'x', 2}, {'a', 3} });
			double expected = -72;

			Assert::IsTrue(actual.has_value());
			Assert::AreEqual(expected, *actual);
		}
	};

	TEST_CLASS(expressionsNumericallyEqual)
	{
	public:

		TEST_METHOD(perfectly_equal)
		{
			auto actual = BuildExpression(Tokenize("3x+5"));
			decltype(actual) expected = BuildExpression(Tokenize("3x+5"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(simple_equal)
		{
			auto actual = BuildExpression(Tokenize("3x+5"));
			decltype(actual) expected = BuildExpression(Tokenize("3x+5+1*0"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(simple_unequal)
		{
			auto actual = BuildExpression(Tokenize("3x+5"));
			decltype(actual) expected = BuildExpression(Tokenize("3x+5+1*1"));

			Assert::IsFalse(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(multivariable_equal)
		{
			auto actual = BuildExpression(Tokenize("3ax^a"));
			decltype(actual) expected = BuildExpression(Tokenize("3(a(x^a))"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(multivariable_unequal)
		{
			auto actual = BuildExpression(Tokenize("3a/x^a"));
			decltype(actual) expected = BuildExpression(Tokenize("3/x^a"));

			Assert::IsFalse(ExpressionsNumericallyEqual(*expected, *actual));
		}
	};

	TEST_CLASS(getSetOfAllSubVariables)
	{
	public:

		TEST_METHOD(complexExpression)
		{
			auto actual = BuildExpression(Tokenize("a^b^(32/d/e-f)^(x*31-m*n)"))->GetSetOfAllSubVariables();
			decltype(actual) expected = { 'a', 'b', 'd', 'e', 'f', 'x', 'm', 'n' };

			Assert::IsTrue(expected == actual);
		}

		TEST_METHOD(noVariables)
		{

			auto actual = BuildExpression(Tokenize("0"))->GetSetOfAllSubVariables();
			decltype(actual) expected = {};

			Assert::IsTrue(expected == actual);
		}
	};


	TEST_CLASS(expression_differentiate)
	{
	public:

		TEST_METHOD(BasicExpression)
		{		
			auto actual = BuildExpression(Tokenize("3x+5"))->Derivative('x')->Simplified();
			decltype(actual) expected = BuildExpression(Tokenize("3"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(PowerRule)
		{
			auto actual = BuildExpression(Tokenize("3x^5"))->Derivative('x')->Simplified();
			decltype(actual) expected = BuildExpression(Tokenize("15x^4"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(ChainRule)
		{
			auto actual = BuildExpression(Tokenize("3(x^2+2)^5"))->Derivative('x')->Simplified();
			decltype(actual) expected = BuildExpression(Tokenize("30x(x^2+2)^4"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}

		TEST_METHOD(QuotientRule)
		{
			auto actual = BuildExpression(Tokenize("(x+1)/(x-1)"))->Derivative('x')->Simplified();
			decltype(actual) expected = BuildExpression(Tokenize("-2/(x-1)^2"));

			Assert::IsTrue(ExpressionsNumericallyEqual(*expected, *actual));
		}
	};

	TEST_CLASS(expression_equality)
	{
	public:

		TEST_METHOD(same)
		{
			auto a = BuildExpression(Tokenize("x+y"));
			auto b = BuildExpression(Tokenize("x+y"));

			Assert::IsTrue(*a == *b);
		}		
		
		TEST_METHOD(different)
		{
			auto a = BuildExpression(Tokenize("x+y"));
			auto b = BuildExpression(Tokenize("x*y"));

			Assert::IsFalse(*a == *b);
		}
	};
}
