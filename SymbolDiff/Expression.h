#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <optional>

class Constant;

class ExpressionBase
{
public:
	virtual ~ExpressionBase() = default;

	virtual std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const = 0;
	virtual std::string Print() const = 0;
	virtual std::unique_ptr<ExpressionBase> Clone() const = 0;

	virtual std::unique_ptr<ExpressionBase> Derivative(char wrt) const = 0;
	virtual std::unique_ptr<ExpressionBase> Simplified() const;

	bool operator==(const ExpressionBase& other) const;

	virtual void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes);
	virtual void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes);

	virtual std::unordered_set<char> GetSetOfAllSubVariables() const;
	virtual void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const;

	int Priority() const;

private:
	virtual bool isEqual(const ExpressionBase& other) const = 0;
};

template <typename Derived>
class Expression : public ExpressionBase
{
public:
	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<Derived>(static_cast<Derived const&>(*this)); }
};

class Constant : public Expression<Constant>
{
public:
	explicit Constant(double val) : value(val) {}

	auto GetConstant() const { return value; };
	void SetConstant(double val) { value = val; }

	std::string Print() const override;
	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;

	void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes) override;
	void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes) override;

private:
	bool isEqual(const ExpressionBase& other) const override;

	double value;
};

class Variable : public Expression<Variable>
{
public:
	explicit Variable(char val) : pronumeral(val) {}

	auto GetVariable() const { return pronumeral; };

	std::string Print() const override;
	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;

	void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

private:
	bool isEqual(const ExpressionBase& other) const override;

	char pronumeral;
};

template <typename Derived>
class BinaryOperator : public ExpressionBase
{
public:
	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<Derived>(left->Clone(), right->Clone()); }

	BinaryOperator(std::unique_ptr<ExpressionBase>&& l, std::unique_ptr<ExpressionBase>&& r);

	void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

protected:
	bool isEqual(const ExpressionBase& other) const override;
	std::unique_ptr<ExpressionBase> EvaluateIfPossible() const;

	std::string PrintBinary(const std::string& op, bool swap, bool leftAssosiative) const;

	std::unique_ptr<ExpressionBase> left;
	std::unique_ptr<ExpressionBase> right;
};

class OperatorPlus : public BinaryOperator<OperatorPlus>
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes) override;

private:
	static std::unique_ptr<ExpressionBase> Simplify(std::unique_ptr<OperatorPlus>&& expr);
};

class OperatorMinus : public BinaryOperator<OperatorMinus>
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;
};

class OperatorMultiply : public BinaryOperator<OperatorMultiply>
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes) override;

private:
	static std::unique_ptr<ExpressionBase> Simplify(std::unique_ptr<OperatorMultiply>&& expr);
};

class OperatorDivide : public BinaryOperator<OperatorDivide>
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;
};

class OperatorExponent : public BinaryOperator<OperatorExponent>
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

private:
	static std::unique_ptr<ExpressionBase> Simplify(std::unique_ptr<OperatorExponent>&& expr);
};

template <typename Derived>
class UnaryOperator : public ExpressionBase
{
public:
	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<Derived>(right->Clone()); }

	explicit UnaryOperator(std::unique_ptr<ExpressionBase>&& r);

	void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

protected:
	bool isEqual(const ExpressionBase& other) const override;
	std::unique_ptr<ExpressionBase> EvaluateIfPossible() const;

	std::unique_ptr<ExpressionBase> right;
};

class OperatorUnaryMinus : public UnaryOperator<OperatorUnaryMinus>
{
public:
	using UnaryOperator::UnaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;
};

