#include "CppUnitTest.h"

#include "..\SymbolDiff\Lexer.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Lexer
{
	TEST_CLASS(tokenize)
	{
	public:
		TEST_METHOD(BasicExpression)
		{
			auto actual = Tokenize("3x+6");

			decltype(actual) expected = {
				Token::CreateConstant(3),
				Token::CreateOperator('*'),
				Token::CreateVariable('x'),
				Token::CreateOperator('+'),
				Token::CreateConstant(6)
			};

			Assert::IsTrue(actual == expected);
		}

		TEST_METHOD(BasicExpressionWithWhitespace)
		{
			auto actual = Tokenize("3x - 6");

			decltype(actual) expected = {
				Token::CreateConstant(3),
				Token::CreateOperator('*'),
				Token::CreateVariable('x'),
				Token::CreateOperator('-'),
				Token::CreateConstant(6)
			};

			Assert::IsTrue(actual == expected);
		}

		TEST_METHOD(MultiDigitConstants)
		{
			auto actual = Tokenize("33x * 66");

			decltype(actual) expected = {
				Token::CreateConstant(33),
				Token::CreateOperator('*'),
				Token::CreateVariable('x'),
				Token::CreateOperator('*'),
				Token::CreateConstant(66)
			};

			Assert::IsTrue(actual == expected);
		}

		// TODO: Implement functionality to make this test pass
		/*TEST_METHOD(InvalidWhitespaceBetweenDigits)
		{
			// Clearly an invalid expression, but it's not the role of the lexer to see that.
			// We don't want '6 6' to become '66'

			auto actual = Tokenize("33x + 6 6");

			decltype(actual) expected = {
				Token::CreateConstant(33),
				Token::CreateOperator('*'),
				Token::CreateVariable('x'),
				Token::CreateOperator('+'),
				Token::CreateConstant(6),
				Token::CreateConstant(6)
			};

			Assert::IsTrue(actual == expected);
		}*/

		TEST_METHOD(Parentheses)
		{
			auto actual = Tokenize("(a^b^(c/d/e-f)^(x*y-m*n))");

			decltype(actual) expected = {
				Token::CreateOperator('('),
				Token::CreateVariable('a'),
				Token::CreateOperator('^'),
				Token::CreateVariable('b'),
				Token::CreateOperator('^'),
				Token::CreateOperator('('),
				Token::CreateVariable('c'),
				Token::CreateOperator('/'),
				Token::CreateVariable('d'),
				Token::CreateOperator('/'),
				Token::CreateVariable('e'),
				Token::CreateOperator('-'),
				Token::CreateVariable('f'),
				Token::CreateOperator(')'),
				Token::CreateOperator('^'),
				Token::CreateOperator('('),
				Token::CreateVariable('x'),
				Token::CreateOperator('*'),
				Token::CreateVariable('y'),
				Token::CreateOperator('-'),
				Token::CreateVariable('m'),
				Token::CreateOperator('*'),
				Token::CreateVariable('n'),
				Token::CreateOperator(')'),
				Token::CreateOperator(')'),
			};

			Assert::IsTrue(actual == expected);
		}

	};
}
