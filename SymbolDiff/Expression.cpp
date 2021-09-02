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

bool Operator::isEqual(const ExpressionBase& other) const
{
    const auto& converted_other = static_cast<decltype(*this)>(other);

    return (left == converted_other.left || (left && converted_other.left && *left == *converted_other.left)) &&
        (right == converted_other.right || (right && converted_other.right && *right == *converted_other.right));
}

//---------------------------------

Operator::Operator(const Operator& other) : Operator(*other.left, *other.right) {}

//---------------------------------

std::unordered_set<char> ExpressionBase::GetSetOfAllSubVariables() const
{
    std::unordered_set<char> variables;
    GetSetOfAllSubVariables(variables);
    return variables;
}

void Operator::GetSetOfAllSubVariables(std::unordered_set<char>& variables) const
{
    left->GetSetOfAllSubVariables(variables);
    right->GetSetOfAllSubVariables(variables);
}

void Variable::GetSetOfAllSubVariables(std::unordered_set<char>& variables) const
{
    variables.insert(pronumeral);
}

void ExpressionBase::GetSetOfAllSubVariables(std::unordered_set<char>&) const
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
    // A little weird, but we don't want to accidentally use *this and only want to use copy.
    // By wrapping in a lambda, *this is unavailable

    return [](OperatorPlus copy) -> std::unique_ptr<ExpressionBase>
    {
        copy.left = copy.left->Simplified();
        copy.right = copy.right->Simplified();

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
        copy.GetConstantSubNodesFromPlus(constantLeafs);

        double total = GetSum(constantLeafs);

        for (size_t i = 0; i < constantLeafs.size(); i++)
            constantLeafs[i]->SetConstant(i == 0 ? total : 0);

        copy.left = copy.left->Simplified();
        copy.right = copy.right->Simplified();

        // x+0 -> x
        if (dynamic_cast<Constant*>(copy.left.get()) && dynamic_cast<Constant*>(copy.left.get())->GetConstant() == 0)
        {
            return copy.right->Clone();
        }

        if (dynamic_cast<Constant*>(copy.right.get()) && dynamic_cast<Constant*>(copy.right.get())->GetConstant() == 0)
        {
            return copy.left->Clone();
        }

        auto evaluated = copy.EvaluateIfPossible();
        if (evaluated) return evaluated;

        return copy.Clone();

    }(*this);
}

std::unique_ptr<ExpressionBase> OperatorMinus::Simplified() const
{
    auto copy = std::make_unique<OperatorMinus>(*left->Simplified(), *right->Simplified());

    auto evaluated = copy->EvaluateIfPossible();
    if (evaluated) return evaluated;

    return copy;
}

std::unique_ptr<ExpressionBase> OperatorDivide::Simplified() const
{
    auto copy = std::make_unique<OperatorDivide>(*left->Simplified(), *right->Simplified());

    auto evaluated = copy->EvaluateIfPossible();
    if (evaluated) return evaluated;

    return copy;
}

std::unique_ptr<ExpressionBase> OperatorMultiply::Simplified() const
{
    return [](OperatorMultiply copy) -> std::unique_ptr<ExpressionBase>
    {
        copy.left = copy.left->Simplified();
        copy.right = copy.right->Simplified();

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
        copy.GetConstantSubNodesFromMultiply(constantLeafs);

        double total = GetProduct(constantLeafs);

        for (size_t i = 0; i < constantLeafs.size(); i++)
            constantLeafs[i]->SetConstant(i == 0 ? total : 1);

        copy.left = copy.left->Simplified();
        copy.right = copy.right->Simplified();

        // x*0 -> 0
        if ((dynamic_cast<Constant*>(copy.left.get()) && dynamic_cast<Constant*>(copy.left.get())->GetConstant() == 0) ||
            (dynamic_cast<Constant*>(copy.right.get()) && dynamic_cast<Constant*>(copy.right.get())->GetConstant() == 0))
        {
            return std::make_unique<Constant>(0);
        }

        // x*1 -> x
        if (dynamic_cast<Constant*>(copy.left.get()) && dynamic_cast<Constant*>(copy.left.get())->GetConstant() == 1)
        {
            return copy.right->Clone();
        }

        if (dynamic_cast<Constant*>(copy.right.get()) && dynamic_cast<Constant*>(copy.right.get())->GetConstant() == 1)
        {
            return copy.left->Clone();
        }

        auto evaluated = copy.EvaluateIfPossible();
        if (evaluated) return evaluated;

        return copy.Clone();

    }(*this);
}

std::unique_ptr<ExpressionBase> OperatorExponent::Simplified() const
{
    return [](OperatorExponent copy) -> std::unique_ptr<ExpressionBase>
    {
        copy.left = copy.left->Simplified();
        copy.right = copy.right->Simplified();

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

    }(*this);
}

//---------------------------------

std::unique_ptr<ExpressionBase> Operator::EvaluateIfPossible() const
{
    // Evaluate to a constant if possible

    auto eval = Evaluate();
    if (eval)
        return std::make_unique<Constant>(*eval);
    else
        return { nullptr };
}
