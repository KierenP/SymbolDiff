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
	virtual std::unique_ptr<ExpressionBase> Clone() const & = 0;
	virtual std::unique_ptr<ExpressionBase> Clone() && = 0;

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

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(value); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(value); }

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

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(pronumeral); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(pronumeral); }

	void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

private:
	bool isEqual(const ExpressionBase& other) const override;

	char pronumeral;
};

class BinaryOperator : public ExpressionBase
{
public:
	template <typename T, typename U>
	BinaryOperator(T&& l, U&& r);

	void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

protected:
	bool isEqual(const ExpressionBase& other) const override;
	std::unique_ptr<ExpressionBase> EvaluateIfPossible() const;

	std::string PrintBinary(const std::string& op, bool swap, bool leftAssosiative) const;

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

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(*left, *right); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(left.release(), right.release()); }

private:
	static std::unique_ptr<ExpressionBase> Simplify(OperatorPlus expr);
};

class OperatorMinus : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(*left, *right); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(left.release(), right.release()); }
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

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(*left, *right); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(left.release(), right.release()); }

private:
	static std::unique_ptr<ExpressionBase> Simplify(OperatorMultiply expr);
};

class OperatorDivide : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(*left, *right); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(left.release(), right.release()); }
};

class OperatorExponent : public BinaryOperator
{
public:
	using BinaryOperator::BinaryOperator;

	std::optional<double> Evaluate(const std::unordered_map<char, double>& values = {}) const override;
	std::unique_ptr<ExpressionBase> Derivative(char wrt) const override;
	std::unique_ptr<ExpressionBase> Simplified() const override;
	std::string Print() const override;

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(*left, *right); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(left.release(), right.release()); }
};

class UnaryOperator : public ExpressionBase
{
public:
	template <typename T>
	explicit UnaryOperator(T&& r);

	void FillSetOfAllSubVariables(std::unordered_set<char>& variables) const override;

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

	std::unique_ptr<ExpressionBase> Clone() const& override { return std::make_unique<std::decay_t<decltype(*this)>>(*right); }
	std::unique_ptr<ExpressionBase> Clone() && override { return std::make_unique<std::decay_t<decltype(*this)>>(right.release()); }
};

template<typename T, typename U>
inline BinaryOperator::BinaryOperator(T&& l, U&& r) :
	left([&]() { if constexpr (std::is_pointer_v<T>) return l; else return std::forward<T>(l).Clone(); }()),
	right([&]() { if constexpr (std::is_pointer_v<U>) return r; else return std::forward<U>(r).Clone(); }())
{}

template<typename T>
inline UnaryOperator::UnaryOperator(T&& r) :
	right([&]() { if constexpr (std::is_pointer_v<T>) return r; else return std::forward<T>(r).Clone(); }())
{
}
