#include "Algorithms.h"

#include <random>
#include <typeindex>

std::string Differentiate(const std::string& str, char wrt)
{
    return BuildExpression(Tokenize(str))->Derivative(wrt)->Simplified()->Print();
}

bool ExpressionsNumericallyEqual(const ExpressionBase& lhs, const ExpressionBase& rhs)
{
    // Exact match saves us work
    if (lhs == rhs) return true;

    auto l = lhs.GetSetOfAllSubVariables();
    auto r = rhs.GetSetOfAllSubVariables();

    // Check expressions contain a the same set of variables
    if (l != r) return false;

    for (int i = 0; i < 1000; i++)
    {
        // Constant seed for determinism
        std::default_random_engine eng(0);

        // Makes sense to have more numbers closer to zero for numerical stability
        std::normal_distribution<> distr(0, 10);

        std::unordered_map<char, double> variables;

        for (const auto& var : l)
            variables[var] = distr(eng);

        auto approximatelyEqual = [](double a, double b, double epsilon)
        {
            return abs(a - b) <= (std::max(abs(a), abs(b)) * epsilon);
        };

        if (!approximatelyEqual(*lhs.Evaluate(variables), *rhs.Evaluate(variables), 0.001))
            return false;
    }

    return true;
}

// EVALUATE FUNCTIONS
//---------------------------------

std::optional<double> Constant::Evaluate(const std::unordered_map<char, double>& /*values*/) const
{
    return value;
}

std::optional<double> Variable::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto pos = values.find(pronumeral);

    if (pos == values.end())
    {
        return std::nullopt;
    }
    else
    {
        return pos->second;
    }
}

std::optional<double> OperatorPlus::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto l = left->Evaluate(values);
    auto r = right->Evaluate(values);

    if (l && r)
        return (*l) + (*r);
    else
        return std::nullopt;
}

std::optional<double> OperatorMinus::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto l = left->Evaluate(values);
    auto r = right->Evaluate(values);

    if (l && r)
        return (*l) - (*r);
    else
        return std::nullopt;
}

std::optional<double> OperatorMultiply::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto l = left->Evaluate(values);
    auto r = right->Evaluate(values);

    if (l && r)
        return (*l) * (*r);
    else
        return std::nullopt;
}

std::optional<double> OperatorDivide::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto l = left->Evaluate(values);
    auto r = right->Evaluate(values);

    if (l && r)
        return (*l) / (*r);
    else
        return std::nullopt;
}

std::optional<double> OperatorExponent::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto l = left->Evaluate(values);
    auto r = right->Evaluate(values);

    if (l && r)
        return std::pow(*l, *r);
    else
        return std::nullopt;
}

std::optional<double> OperatorUnaryMinus::Evaluate(const std::unordered_map<char, double>& values) const
{
    auto r = right->Evaluate(values);

    if (r)
        return -(*r);
    else
        return std::nullopt;
}

// PRINT FUNCTIONS
//---------------------------------

int ExpressionBase::Priority() const
{
    static const std::unordered_map<std::type_index, int> priority =
    {
        { typeid(Constant),             10 },
        { typeid(Variable),             10 },
        { typeid(OperatorPlus),         1 },
        { typeid(OperatorMinus),        1 },
        { typeid(OperatorMultiply),     2 },
        { typeid(OperatorDivide),       2 },
        { typeid(OperatorUnaryMinus),   3 },
        { typeid(OperatorExponent),     4 },
        
    };

    return priority.at(typeid(*this));
}

std::string Constant::Print() const
{
    auto PrintWithoutTrailingZeros = [](std::string str)
    {
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        str.erase(str.find_last_not_of('.') + 1, std::string::npos);
        return str;
    };

    return PrintWithoutTrailingZeros(std::to_string(value));
}

std::string Variable::Print() const
{
    return std::string(1, pronumeral);
}

std::string BinaryOperator::PrintBinary(const std::string& op, bool swap, bool leftAssosiative) const
{
    auto PrintExpr = [op, leftAssosiative](const ExpressionBase& l, const ExpressionBase& r, int priority)
    {
        std::string str = "";

        if (leftAssosiative && l.Priority() < priority ||
            !leftAssosiative && l.Priority() <= priority)
            str += "(" + l.Print() + ")";
        else
            str += l.Print();

        str += op;

        if (leftAssosiative && r.Priority() <= priority ||
            !leftAssosiative && r.Priority() < priority)
            str += "(" + r.Print() + ")";
        else
            str += r.Print();

        return str;
    };

    if (swap)
        return PrintExpr(*right, *left, Priority());
    else
        return PrintExpr(*left, *right, Priority());
}

std::string OperatorPlus::Print() const
{
    return BinaryOperator::PrintBinary("+", false, true);
}

std::string OperatorMinus::Print() const
{
    return BinaryOperator::PrintBinary("-", false, true);
}

std::string OperatorDivide::Print() const
{
    return BinaryOperator::PrintBinary("/", false, true);
}

std::string OperatorMultiply::Print() const
{
    // If we are going to print x*31 instead print out 31x
    if (dynamic_cast<Variable*>(left.get()) && dynamic_cast<Constant*>(right.get()))
        return BinaryOperator::PrintBinary("", true, true);
    else
        return BinaryOperator::PrintBinary("", false, true);
}

std::string OperatorExponent::Print() const
{
    return BinaryOperator::PrintBinary("^", false, false);
}

std::string OperatorUnaryMinus::Print() const
{
    if (right->Priority() <= Priority())
        return "-(" + right->Print() + ")";
    else
       return "-" + right->Print();
}

// DERIVATIVE FUNCTIONS
//---------------------------------

std::unique_ptr<ExpressionBase> Constant::Derivative(char wrt) const
{
    return std::make_unique<Constant>(0);
}

std::unique_ptr<ExpressionBase> Variable::Derivative(char wrt) const
{
    return std::make_unique<Constant>(wrt == pronumeral ? 1 : 0);
}

std::unique_ptr<ExpressionBase> OperatorPlus::Derivative(char wrt) const
{
    return std::make_unique<OperatorPlus>(left->Derivative(wrt), right->Derivative(wrt));
}

std::unique_ptr<ExpressionBase> OperatorMinus::Derivative(char wrt) const
{
    return std::make_unique<OperatorMinus>(left->Derivative(wrt), right->Derivative(wrt));
}

std::unique_ptr<ExpressionBase> OperatorMultiply::Derivative(char wrt) const
{
    return
        std::make_unique<OperatorPlus>(
            std::make_unique<OperatorMultiply>(
                left->Clone(),
                right->Derivative(wrt)),
            std::make_unique<OperatorMultiply>(
                right->Clone(),
                left->Derivative(wrt)));
}

std::unique_ptr<ExpressionBase> OperatorDivide::Derivative(char wrt) const
{
    return 
        std::make_unique<OperatorDivide>(
            std::make_unique<OperatorMinus>(
                std::make_unique<OperatorMultiply>(
                    right->Clone(),
                    left->Derivative(wrt)),
                std::make_unique<OperatorMultiply>(
                    left->Clone(),
                    right->Derivative(wrt))),
            std::make_unique<OperatorExponent>(
                right->Clone(),
                std::make_unique<Constant>(2)));
}

std::unique_ptr<ExpressionBase> OperatorExponent::Derivative(char wrt) const
{
    return 
        std::make_unique<OperatorMultiply>(
            right->Clone(),
            std::make_unique<OperatorMultiply>(
                left->Derivative(wrt),
                std::make_unique<OperatorExponent>(
                    left->Clone(),
                    std::make_unique<OperatorMinus>(
                        right->Clone(),
                        std::make_unique<Constant>(1)))));
}

std::unique_ptr<ExpressionBase> OperatorUnaryMinus::Derivative(char wrt) const
{
    return 
        std::make_unique<OperatorUnaryMinus>(
            right->Derivative(wrt));
}

//---------------------------------