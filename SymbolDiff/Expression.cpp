#include "Expression.h"

#include <numeric>
#include <string>

bool ExpressionBase::operator==(const ExpressionBase& other) const
{
    return typeid(*this) == typeid(other) && isEqual(other);
}

bool Constant::isEqual(const ExpressionBase& other) const
{
    return value == static_cast<decltype(*this)>(other).value;
}

bool Variable::isEqual(const ExpressionBase& other) const
{
    return pronumeral == static_cast<decltype(*this)>(other).pronumeral;
}

bool BinaryOperator::isEqual(const ExpressionBase& other) const
{
    const auto& converted_other = static_cast<decltype(*this)>(other);

    return (left == converted_other.left || (left && converted_other.left && *left == *converted_other.left)) &&
        (right == converted_other.right || (right && converted_other.right && *right == *converted_other.right));
}

bool UnaryOperator::isEqual(const ExpressionBase& other) const
{
    const auto& converted_other = static_cast<decltype(*this)>(other);

    return (right == converted_other.right || (right && converted_other.right && *right == *converted_other.right));
}

std::unordered_set<char> ExpressionBase::GetSetOfAllSubVariables() const
{
    std::unordered_set<char> variables;
    FillSetOfAllSubVariables(variables);
    return variables;
}

BinaryOperator::BinaryOperator(std::unique_ptr<ExpressionBase>&& l, std::unique_ptr<ExpressionBase>&& r) :
    left(std::move(l)), right(std::move(r)) {}

UnaryOperator::UnaryOperator(std::unique_ptr<ExpressionBase>&& r) : 
    right(std::move(r)) {}

void UnaryOperator::FillSetOfAllSubVariables(std::unordered_set<char>& variables) const
{
    right->FillSetOfAllSubVariables(variables);
}

void BinaryOperator::FillSetOfAllSubVariables(std::unordered_set<char>& variables) const
{
    left->FillSetOfAllSubVariables(variables);
    right->FillSetOfAllSubVariables(variables);
}

void Variable::FillSetOfAllSubVariables(std::unordered_set<char>& variables) const
{
    variables.insert(pronumeral);
}

void ExpressionBase::FillSetOfAllSubVariables(std::unordered_set<char>&) const
{
    // Do Nothing
}

//---------------------------------

void ExpressionBase::GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes)
{
    // Do Nothing
}

void ExpressionBase::GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes)
{
    // Do Nothing
}

void Constant::GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes)
{
    nodes.push_back(this);
}

void Constant::GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes)
{
    nodes.push_back(this);
}

void OperatorPlus::GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes)
{
    left->GetConstantSubNodesFromPlus(nodes);
    right->GetConstantSubNodesFromPlus(nodes);
}

void OperatorMultiply::GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes)
{
    left->GetConstantSubNodesFromMultiply(nodes);
    right->GetConstantSubNodesFromMultiply(nodes);
}

//---------------------------------

std::unique_ptr<ExpressionBase> ExpressionBase::Simplified() const
{
    // Do nothing
    return Clone();
}

std::unique_ptr<ExpressionBase> OperatorPlus::Simplified() const
{
    return Simplify(OperatorPlus(left->Simplified(), right->Simplified()));
}

std::unique_ptr<ExpressionBase> OperatorPlus::Simplify(OperatorPlus expr)
{
    auto GetSum = [](std::vector<Constant*> vals)
    {
        return std::accumulate(vals.begin(), vals.end(), 0.0, [](double a, const Constant* b)
            {
                return a + b->GetConstant();
            });
    };

    // Multiplication/addition can be done in any order e.g 3*(5*x) can be simplified
    // This is complex, because we could have any number of multiplications
    // or additions in order with constants or variables or whole expressions with brackets.
    // The algorithm recursively finds all constant value leaf nodes from successive 
    // multiplication or additions and then combines them into one constant, leaving the others 
    // to be 1 for multiplication or 0 for addition. e.g: 3*x*4 -> 12*x*1 or 3+x+4 -> 7+x+0. 
    // We do this simplification first because it could allow for further simplifications later

    std::vector<Constant*> constantLeafs;
    expr.GetConstantSubNodesFromPlus(constantLeafs);

    double total = GetSum(constantLeafs);

    for (size_t i = 0; i < constantLeafs.size(); i++)
        constantLeafs[i]->SetConstant(i == 0 ? total : 0);

    expr.left = expr.left->Simplified();
    expr.right = expr.right->Simplified();

    // x+0 -> x
    if (dynamic_cast<Constant*>(expr.left.get()) && dynamic_cast<Constant*>(expr.left.get())->GetConstant() == 0)
    {
        return expr.right->Clone();
    }

    if (dynamic_cast<Constant*>(expr.right.get()) && dynamic_cast<Constant*>(expr.right.get())->GetConstant() == 0)
    {
        return expr.left->Clone();
    }

    auto evaluated = expr.EvaluateIfPossible();
    if (evaluated) return evaluated;

    return expr.Clone();
}

std::unique_ptr<ExpressionBase> OperatorMinus::Simplified() const
{
    auto copy = std::make_unique<OperatorMinus>(left->Simplified(), right->Simplified());

    auto evaluated = copy->EvaluateIfPossible();
    if (evaluated) return evaluated;

    return copy;
}

std::unique_ptr<ExpressionBase> OperatorDivide::Simplified() const
{
    auto copy = std::make_unique<OperatorDivide>(left->Simplified(), right->Simplified());

    auto evaluated = copy->EvaluateIfPossible();
    if (evaluated) return evaluated;

    return copy;
}

std::unique_ptr<ExpressionBase> OperatorMultiply::Simplified() const
{
    return Simplify(OperatorMultiply(left->Simplified(), right->Simplified()));
}

std::unique_ptr<ExpressionBase> OperatorMultiply::Simplify(OperatorMultiply expr)
{
    auto GetProduct = [](std::vector<Constant*> vals)
    {
        return std::accumulate(vals.begin(), vals.end(), 1.0, [](double a, const Constant* b)
            {
                return a * b->GetConstant();
            });
    };

    // Multiplication/addition can be done in any order e.g 3*(5*x) can be simplified
    // This is complex, because we could have any number of multiplications
    // or additions in order with constants or variables or whole expressions with brackets.
    // The algorithm recursively finds all constant value leaf nodes from successive 
    // multiplication or additions and then combines them into one constant, leaving the others 
    // to be 1 for multiplication or 0 for addition. e.g: 3*x*4 -> 12*x*1 or 3+x+4 -> 7+x+0. 
    // We do this simplification first because it could allow for further simplifications later

    std::vector<Constant*> constantLeafs;
    expr.GetConstantSubNodesFromMultiply(constantLeafs);

    double total = GetProduct(constantLeafs);

    for (size_t i = 0; i < constantLeafs.size(); i++)
        constantLeafs[i]->SetConstant(i == 0 ? total : 1);

    expr.left = expr.left->Simplified();
    expr.right = expr.right->Simplified();

    // x*0 -> 0
    if ((dynamic_cast<Constant*>(expr.left.get()) && dynamic_cast<Constant*>(expr.left.get())->GetConstant() == 0) ||
        (dynamic_cast<Constant*>(expr.right.get()) && dynamic_cast<Constant*>(expr.right.get())->GetConstant() == 0))
    {
        return std::make_unique<Constant>(0);
    }

    // x*1 -> x
    if (dynamic_cast<Constant*>(expr.left.get()) && dynamic_cast<Constant*>(expr.left.get())->GetConstant() == 1)
    {
        return expr.right->Clone();
    }

    if (dynamic_cast<Constant*>(expr.right.get()) && dynamic_cast<Constant*>(expr.right.get())->GetConstant() == 1)
    {
        return expr.left->Clone();
    }

    auto evaluated = expr.EvaluateIfPossible();
    if (evaluated) return evaluated;

    return expr.Clone();
}

std::unique_ptr<ExpressionBase> OperatorExponent::Simplified() const
{
    return [](OperatorExponent copy) -> std::unique_ptr<ExpressionBase>
    {
        // x^1 -> x, 1^x -> 1
        if (dynamic_cast<Constant*>(copy.left.get()) && dynamic_cast<Constant*>(copy.left.get())->GetConstant() == 1)
        {
            return std::make_unique<Constant>(1);
        }

        if (dynamic_cast<Constant*>(copy.right.get()) && dynamic_cast<Constant*>(copy.right.get())->GetConstant() == 1)
        {
            return copy.left->Clone();
        }

        auto evaluated = copy.EvaluateIfPossible();
        if (evaluated) return evaluated;

        return copy.Clone();

    }(OperatorExponent(left->Simplified(), right->Simplified()));
}

std::unique_ptr<ExpressionBase> OperatorUnaryMinus::Simplified() const
{
    auto copy = std::make_unique<OperatorUnaryMinus>(right->Simplified());

    auto evaluated = copy->EvaluateIfPossible();
    if (evaluated) return evaluated;

    return copy;
}

//---------------------------------

std::unique_ptr<ExpressionBase> BinaryOperator::EvaluateIfPossible() const
{
    // Evaluate to a constant if possible

    auto eval = Evaluate();
    if (eval)
        return std::make_unique<Constant>(*eval);
    else
        return { nullptr };
}

std::unique_ptr<ExpressionBase> UnaryOperator::EvaluateIfPossible() const
{
    // Evaluate to a constant if possible

    auto eval = Evaluate();
    if (eval)
        return std::make_unique<Constant>(*eval);
    else
        return { nullptr };
}

