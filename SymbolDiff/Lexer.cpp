#include "Lexer.hpp"
#include <variant>
#include <assert.h>
#include <stdexcept>

std::vector<std::string> SplitInputToTokenStrings(std::string input)
{
	std::vector<std::string> tokens;

	size_t index = 0;
	
	while (index < input.size())
	{
		std::string token;

		if (isspace(input[index]))
		{
			// Do nothing
		}

		else if (isdigit(input[index]))
		{
			// Read the whole number, then continue to the next token
			while (index < input.size() && (isdigit(input[index]) || input[index] == '.'))
			{
				token.push_back(input[index]);
				index++;
			}

			tokens.push_back(token);
			continue;
		}

		else
		{
			tokens.push_back({ input[index] });
		}

		index++;
	}

	return tokens;
}

// We assume that all numbers are non zero, as '-33' would be lexed as a unary minus '-' and a constant '33'.

std::vector<Token> ConvertStringsToTokens(std::vector<std::string> input)
{
	std::vector<Token> tokens;

	// https://stackoverflow.com/a/29169409
	auto is_number = [](const std::string& s)
	{
		char* end = nullptr;
		double val = strtod(s.c_str(), &end);
		return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
	};

	auto is_operator = [](const std::string& s)
	{
		assert(s.length() >= 1);

		static const std::vector<char> operators = { '+', '-', '*', '/', '^', '(', ')' };
		return std::find(operators.begin(), operators.end(), s[0]) != operators.end();
	};

	auto is_variable = [](const std::string& s)
	{
		assert(s.length() >= 1);

		return isalpha(s[0]);
	};

	for (const auto& str : input)
	{
		assert(str.length() > 0);

		if (is_number(str))
			tokens.push_back(Token::CreateConstant(std::stod(str)));
		else if (is_operator(str))
			tokens.push_back(Token::CreateOperator(str[0]));
		else if (is_variable(str))
			tokens.push_back(Token::CreateVariable(str[0]));
		else
			throw std::invalid_argument("Unknown token: " + str);
	}			

	return tokens;
}

bool Token::IsConstant() const
{
	return data.index() == static_cast<size_t>(Type::Constant);
}

bool Token::IsVariable() const
{
	return data.index() == static_cast<size_t>(Type::Variable);
}

bool Token::IsOperator() const
{
	return data.index() == static_cast<size_t>(Type::Operator);
}

Token Token::CreateConstant(Token::constant_t value)
{
	return Token(decltype(data){ std::in_place_index<static_cast<size_t>(Type::Constant)>, value });
}

Token Token::CreateVariable(Token::variable_t letter)
{
	assert(isalpha(letter));
	return Token(decltype(data){ std::in_place_index<static_cast<size_t>(Type::Variable)>, letter });
}

Token Token::CreateOperator(Token::operator_t op)
{
	assert(!isalpha(op));
	return Token(decltype(data){ std::in_place_index<static_cast<size_t>(Type::Operator)>, op });
}

bool Token::operator==(const Token& other) const
{
	return data == other.data;
}

Token::constant_t Token::GetConstant() const
{
	return std::get<static_cast<size_t>(Type::Constant)>(data);
}

Token::variable_t Token::GetVariable() const
{
	return std::get<static_cast<size_t>(Type::Variable)>(data);
}

Token::operator_t Token::GetOperator() const
{
	return std::get<static_cast<size_t>(Type::Operator)>(data);
}

void AddImplicitMultiplication(std::vector<Token>& tokens)
{
	// c = constant
	// v = variable
	// op = operator but not whichever braket follows
	// (, ) = respective open or close bracket

	//             right
	//          c  v  op (
	//         ____________
	//   l  c | ?  x  .  x
	//   e  v | x  x  .  x
	//   f op | .  .  .  .
	//   t  ) | x  x  .  x
	//
	// constant - constant should probably be an error detected elsewhere

	for (size_t i = 0; i < tokens.size() - 1; i++)
		if ((!tokens[i].IsOperator() || tokens[i].GetOperator() == ')') &&
			(!tokens[i+1].IsOperator() || tokens[i+1].GetOperator() == '('))
			tokens.insert(tokens.begin() + i + 1, Token::CreateOperator('*'));
}

std::vector<Token> Tokenize(std::string input)
{
	auto tmp1 = SplitInputToTokenStrings(input);
	auto tmp2 = ConvertStringsToTokens(tmp1);

	AddImplicitMultiplication(tmp2);

	return tmp2;
}
