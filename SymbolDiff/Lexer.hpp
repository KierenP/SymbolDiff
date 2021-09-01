#pragma once
#include <variant>
#include <vector>
#include <string>

class Token
{
public:

	using constant_t = double;
	using variable_t = char;
	using operator_t = char;

	bool IsConstant() const;
	bool IsVariable() const;
	bool IsOperator() const;

	static Token CreateConstant(constant_t value);
	static Token CreateVariable(variable_t letter);
	static Token CreateOperator(operator_t op);

	bool operator==(const Token& other) const;

	constant_t GetConstant() const;
	variable_t GetVariable() const;
	operator_t GetOperator() const;

private:

	//Order must match that of data's types
	enum class Type
	{
		Constant,
		Variable,
		Operator,
	};

	std::variant<constant_t, variable_t, operator_t> data;

	Token(decltype(data) value) : data(value) {}
};

std::vector<Token> Tokenize(std::string input);