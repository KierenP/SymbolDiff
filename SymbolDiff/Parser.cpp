#include "Parser.hpp"
#include <stack>
#include <unordered_map>
#include <assert.h>
#include <vector>
#include <numeric>
#include <typeindex>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <random>

int ExpressionBase::Priority() const
{
    static const std::unordered_map<std::type_index, int> priority =
    {
        { typeid(Constant),         10 },
        { typeid(Variable),         10 },
        { typeid(OperatorPlus),     1 },
        { typeid(OperatorMinus),    1 },
        { typeid(OperatorMultiply), 2 },
        { typeid(OperatorDivide),   2 },
        { typeid(OperatorExponent), 3 },
    };

    return priority.at(typeid(*this));
}

std::unique_ptr<ExpressionBase> BuildExpression(std::vector<Token> input)
{
    // Adapted from https://www.geeksforgeeks.org/program-to-convert-infix-notation-to-expression-tree/
    // For reasons, this algorithm only works with the entire expression is wrapped in parenthesis

    // Prioritising the operators
    static const std::unordered_map<char, int> p = {
        { ')', 0},
        { '+', 1},
        { '-', 1},
        { '/', 2},
        { '*', 2},
        { '^', 3}
    };

    input.insert(input.begin(), Token::CreateOperator('('));
    input.push_back(Token::CreateOperator(')'));

    // Stack to hold expressions
    std::stack<ExpressionBase*> nodes;

    // Stack to hold tokens
    std::stack<Token> tokens;

    auto CombineExpressions = [&]()
    {
        // Get and remove the top element
        // from the node stack

        ExpressionBase* right = nodes.top();
        nodes.pop();

        // Get and remove the currently top
        // element from the node stack

        ExpressionBase* left = nodes.top();
        nodes.pop();

        // Get and remove the top element
        // from the character stack
        ExpressionBase* t;
            
        switch (tokens.top().GetOperator())
        {
        case '+':
            t = new OperatorPlus{ *left, *right };
            break;
        case '-':
            t = new OperatorMinus{ *left, *right };
            break;
        case '/':
            t = new OperatorDivide{ *left, *right };
            break;
        case '*':
            t = new OperatorMultiply{ *left, *right };
            break;
        case '^':
            t = new OperatorExponent{ *left, *right };
            break;
        default:
            throw std::invalid_argument("Unknown operator: " + std::to_string(tokens.top().GetOperator()));
        }
            
        tokens.pop();

        return t;
    };

    for (const auto& token : input)
    {
        if (token.IsConstant())
        {
            nodes.emplace(new Constant(token.GetConstant()));
        }

        else if (token.IsVariable())
        {
            nodes.emplace(new Variable(token.GetVariable()));
        }

        else if (token.GetOperator() == '(') {

            // Push '(' in token stack
            tokens.push(token);
        }

        else if (token.GetOperator() == ')')
        {
            while (!tokens.empty() && tokens.top().GetOperator() != '(')
            {
                // From right to left keep grouping the expressions and building the tree
                nodes.push(CombineExpressions());
            }

            tokens.pop();
        }

        // Make sure its a recognised operator
        else if (p.count(token.GetOperator()))
        {
            // If an operator with lower or same associativity appears
            while (!tokens.empty() && tokens.top().GetOperator() != '('
                && ((token.GetOperator() != '^' && p.at(tokens.top().GetOperator()) >= p.at(token.GetOperator())
                    || (token.GetOperator() == '^' && p.at(tokens.top().GetOperator()) > p.at(token.GetOperator())))))
            {
                nodes.push(CombineExpressions());
            }

            // Push token to stack
            tokens.push(token);
        }
        else
            throw std::invalid_argument("Detected operator with unknown priority: " + std::to_string(tokens.top().GetOperator()));
    }

    return std::unique_ptr<ExpressionBase>(nodes.top());
}

std::string Differentiate(std::string str, Token::variable_t wrt)
{
    return BuildExpression(Tokenize(str))->Derivative(wrt)->Simplified()->Print();
}

bool ExpressionsNumericallyEqual(ExpressionBase& lhs, ExpressionBase& rhs)
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

bool ExpressionBase::operator==(const ExpressionBase& other) const
{
    return typeid(*this) == typeid(other) && isEqual(other);
}

//---------------------------------

bool Constant::isEqual(const ExpressionBase& other) const
{
    return value == static_cast<decltype(*this)>(other).value;
}

void Variable::GetSetOfAllSubVariables(std::unordered_set<Token::variable_t>& variables) const
{
    variables.insert(pronumeral);
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

std::optional<double> Constant::Evaluate(const std::unordered_map<Token::variable_t, Token::constant_t>& /*values*/) const
{
    return value;
}

std::optional<double> Variable::Evaluate(const std::unordered_map<Token::variable_t, Token::constant_t>& values) const
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

Operator::Operator(const Operator& other) : Operator(*other.left, *other.right) {}

void Operator::GetSetOfAllSubVariables(std::unordered_set<Token::variable_t>& variables) const
{
    left->GetSetOfAllSubVariables(variables);
    right->GetSetOfAllSubVariables(variables);
}

std::string Operator::Print(std::string op, bool swap, bool leftAssosiative) const
{
    auto PrintExpr = [=](const ExpressionBase& left, const ExpressionBase& right)
    {
        std::string str = "";

        if ( leftAssosiative && left.Priority() <  Priority() || 
            !leftAssosiative && left.Priority() <= Priority())
            str += "(" + left.Print() + ")";
        else
            str += left.Print();

        str += op;

        if ( leftAssosiative && right.Priority() <= Priority() ||
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

//---------------------------------

void ExpressionBase::GetConstantSubNodesFromPlus(std::vector<Constant*>& nodes)
{
    // Do Nothing
}

void ExpressionBase::GetConstantSubNodesFromMultiply(std::vector<Constant*>& nodes)
{
    // Do Nothing
}

std::unordered_set<Token::variable_t> ExpressionBase::GetSetOfAllSubVariables() const
{
    std::unordered_set<Token::variable_t> variables;
    GetSetOfAllSubVariables(variables);
    return variables;
}

void ExpressionBase::GetSetOfAllSubVariables(std::unordered_set<Token::variable_t>&) const
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

std::unique_ptr<ExpressionBase> Constant::Derivative(Token::variable_t wrt) const
{
    return std::make_unique<Constant>(0);
}

std::unique_ptr<ExpressionBase> Variable::Derivative(Token::variable_t wrt) const
{
    return std::make_unique<Constant>(wrt == pronumeral ? 1 : 0);
}

std::unique_ptr<ExpressionBase> OperatorPlus::Derivative(Token::variable_t wrt) const
{
    return std::make_unique<OperatorPlus>(*left->Derivative(wrt), *right->Derivative(wrt));
}

std::unique_ptr<ExpressionBase> OperatorMinus::Derivative(Token::variable_t wrt) const
{
    return std::make_unique<OperatorMinus>(*left->Derivative(wrt), *right->Derivative(wrt));
}

std::unique_ptr<ExpressionBase> OperatorMultiply::Derivative(Token::variable_t wrt) const
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

std::unique_ptr<ExpressionBase> OperatorDivide::Derivative(Token::variable_t wrt) const
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

std::unique_ptr<ExpressionBase> OperatorExponent::Derivative(Token::variable_t wrt) const
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

std::unique_ptr<ExpressionBase> ExpressionBase::Simplified() const
{
    // Do nothing
    return Clone();
}

std::unique_ptr<ExpressionBase> Operator::EvaluateIfPossible() const
{
    // Evaluate to a constant if possible

    auto eval = Evaluate();
    if (eval)
        return std::make_unique<Constant>(*eval);
    else
        return { nullptr };
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
