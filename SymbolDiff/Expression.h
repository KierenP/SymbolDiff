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
	virtual void GetSetOfAllSubVariables(std::unordered_set<char>& variables) const;

	int Priority() const;

private:
	virtual bool isEqual(const ExpressionBase& other) const = 0;
};

class Constant : public ExpressionBase
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

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }

private:
	bool isEqual(const ExpressionBase& other) const override;

	double value;
};

class Variable : public ExpressionBase
{
public:
	explicit Variable(char val) : pronumeral(val) {}

	auto GetVariable() const { return pronumeral; };

	std::string Print() const override;
	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }

	void GetSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

private:
	bool isEqual(const ExpressionBase& other) const override;

	char pronumeral;
};

class BinaryOperator : public ExpressionBase
{
public:
	BinaryOperator(const ExpressionBase& l, const ExpressionBase& r) : left(l.Clone()), right(r.Clone()) {}
	BinaryOperator(const ExpressionBase& l, ExpressionBase* r) : left(l.Clone()), right(r) {}
	BinaryOperator(ExpressionBase* l, const ExpressionBase& r) : left(l), right(r.Clone()) {}
	BinaryOperator(ExpressionBase* l, ExpressionBase* r) : left(l), right(r) {}

	BinaryOperator(const BinaryOperator& other) : left(other.left->Clone()), right(other.right->Clone()) {}
	BinaryOperator(BinaryOperator&& other) = default;

	void GetSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

protected:
	bool isEqual(const ExpressionBase& other) const override;
	std::unique_ptr<ExpressionBase> EvaluateIfPossible() const;

	std::string Print(const std::string& op, bool swap, bool leftAssosiative) const;

	std::unique_ptr<ExpressionBase> left;
	std::unique_ptr<ExpressionBase> right;
};

class OperatorPlus : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	void GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes) override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorMinus : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorMultiply : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	void GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes) override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorDivide : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class OperatorExponent : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};

class UnaryOperator : public ExpressionBase
{
public:
	explicit UnaryOperator(const ExpressionBase& r) : right(r.Clone()) {}
	explicit UnaryOperator(ExpressionBase* r) : right(r) {}

	UnaryOperator(const UnaryOperator& other) : right(other.right->Clone()) {};
	UnaryOperator(UnaryOperator&& other) = default;

	void GetSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

protected:
	bool isEqual(const ExpressionBase& other) const override;
	std::unique_ptr<ExpressionBase> EvaluateIfPossible() const;

	std::unique_ptr<ExpressionBase> right;
};

class OperatorUnaryMinus : public UnaryOperator
{
public:
	using UnaryOperator::UnaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const override { return std::make_unique<std::decay_t<decltype(*this)>>(*this); }
};


