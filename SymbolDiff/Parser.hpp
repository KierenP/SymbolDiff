#pragma once
#include "Lexer.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <optional>

class Constant;

class ExpressionBase
{
public:
	virtual std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const = 0;
	virtual std::string Print() const = 0;
	virtual std::unique_ptr<ExpressionBase> Clone() const = 0;

	virtual std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const = 0;
	virtual std::unique_ptr<ExpressionBase> Simplified() const;

	bool operator==(const ExpressionBase& other) const;

	virtual void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes);
	virtual void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes);

	virtual std::unordered_set<Token::variable_t> GetSetOfAllSubVariables() const;
	virtual void GetSetOfAllSubVariables(std::unordered_set<Token::variable_t>& variables) const;

	int Priority() const;

private:
	virtual bool isEqual(const ExpressionBase& other) const = 0;
};

class Constant : public ExpressionBase
{
public:
	Constant(double val) : value(val) {}

	auto GetConstant() const { return value; };
	void SetConstant(double val) { value = val; }

	std::string Print() const override;
	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;

	void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes) override;
	void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes) override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }

private:
	bool isEqual(const ExpressionBase& other) const override;

	double value;
};

class Variable : public ExpressionBase
{
public:
	Variable(char val) : pronumeral(val) {}

	auto GetVariable() const { return pronumeral; };

	std::string Print() const override;
	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
	
	void GetSetOfAllSubVariables(std::unordered_set<Token::variable_t>& variables) const override;

private:
	bool isEqual(const ExpressionBase& other) const override;

	char pronumeral;
};

class Operator : public ExpressionBase
{
public:
	Operator(const ExpressionBase& l, const ExpressionBase& r) : left(l.Clone()), right(r.Clone()) {}

	Operator(const Operator& other);
	Operator(Operator&& other) = default;

	void GetSetOfAllSubVariables(std::unordered_set<Token::variable_t>& variables) const override;

protected:
	bool isEqual(const ExpressionBase& other) const override;
	std::unique_ptr<ExpressionBase> EvaluateIfPossible() const;

	std::string Print(std::string op, bool swap, bool leftAssosiative) const;

	std::unique_ptr<ExpressionBase> left;
	std::unique_ptr<ExpressionBase> right;
};

class OperatorPlus : public Operator
{
public:
	using Operator::Operator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes) override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorMinus : public Operator
{
public:
	using Operator::Operator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorMultiply : public Operator
{
public:
	using Operator::Operator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes) override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorDivide : public Operator
{
public:
	using Operator::Operator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorExponent : public Operator
{
public:
	using Operator::Operator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(Token::variable_t wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

std::unique_ptr<ExpressionBase> BuildExpression(std::vector<Token> input);
std::string Differentiate(std::string str, Token::variable_t wrt);

bool ExpressionsNumericallyEqual(ExpressionBase& lhs, ExpressionBase& rhs);