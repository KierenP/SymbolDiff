#include "Algorithms.h"

#include <random>

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

// PRINT FUNCTIONS
//---------------------------------

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

std::string Operator::Print(const std::string& op, bool swap, bool leftAssosiative) const
{
    auto PrintExpr = [=](const ExpressionBase& left, const ExpressionBase& right)
    {
        std::string str = "";

        if (leftAssosiative && left.Priority() < Priority() ||
            !leftAssosiative && left.Priority() <= Priority())
            str += "(" + left.Print() + ")";
        else
            str += left.Print();

        str += op;

        if (leftAssosiative && right.Priority() <= Priority() ||
            !leftAssosiative && right.Priority() < Priority())
            str += "(" + right.Print() + ")";
        else
            str += right.Print();

        return str;
    };

    if (swap)
        return PrintExpr(*right, *left);
    else
        return PrintExpr(*left, *right);
}

std::string OperatorPlus::Print() const
{
    return Operator::Print("+", false, true);
}

std::string OperatorMinus::Print() const
{
    return Operator::Print("-", false, true);
}

std::string OperatorDivide::Print() const
{
    return Operator::Print("/", false, true);
}

std::string OperatorMultiply::Print() const
{
    // If we are going to print x*31 instead print out 31x
    if (dynamic_cast<Variable*>(left.get()) && dynamic_cast<Constant*>(right.get()))
        return Operator::Print("", true, true);
    else
        return Operator::Print("", false, true);
}

std::string OperatorExponent::Print() const
{
    return Operator::Print("^", false, false);
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
    return std::make_unique<OperatorPlus>(*left->Derivative(wrt), *right->Derivative(wrt));
}

std::unique_ptr<ExpressionBase> OperatorMinus::Derivative(char wrt) const
{
    return std::make_unique<OperatorMinus>(*left->Derivative(wrt), *right->Derivative(wrt));
}

std::unique_ptr<ExpressionBase> OperatorMultiply::Derivative(char wrt) const
{
    return std::make_unique<
        OperatorPlus>(
            OperatorMultiply(
                *left->Clone(),
                *right->Derivative(wrt)),
            OperatorMultiply(
                *right->Clone(),
                *left->Derivative(wrt)));
}

std::unique_ptr<ExpressionBase> OperatorDivide::Derivative(char wrt) const
{
    return std::make_unique<
        OperatorDivide>(
            OperatorMinus(
                OperatorMultiply(
                    *right->Clone(),
                    *left->Derivative(wrt)),
                OperatorMultiply(
                    *left->Clone(),
                    *right->Derivative(wrt))),
            OperatorExponent(
                *right->Clone(),
                Constant(2)));
}

std::unique_ptr<ExpressionBase> OperatorExponent::Derivative(char wrt) const
{
    //if (right->FunctionOf(wrt))
    //    throw "cannot differentiate expression with function in exponent";

    return std::make_unique<
        OperatorMultiply>(
            *right->Clone(),
            OperatorMultiply(
                *left->Derivative(wrt),
                *std::make_unique<OperatorExponent>(
                    *left->Clone(),
                    *std::make_unique<OperatorMinus>(
                        *right->Clone(),
                        Constant(1)))));
}

//---------------------------------