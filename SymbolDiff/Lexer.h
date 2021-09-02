#pragma once
#include <variant>
#include <vector>
#include <string>

class Token
{
public:

	bool IsConstant() const;
	bool IsVariable() const;
	bool IsOperator() const;

	static Token CreateConstant(double value);
	static Token CreateVariable(char letter);
	static Token CreateOperator(char op);

	bool operator==(const Token& other) const;

	double GetConstant() const;
	char GetVariable() const;
	char GetOperator() const;

private:

	//Order must match that of data's types
	enum class Type
	{
		Constant,
		Variable,
		Operator,
	};

	std::variant<double, char, char> data;

	explicit Token(decltype(data) value) : data(value) {}
};

std::vector<Token> Tokenize(const std::string& input);