#include "Parser.h"
#include <stack>
#include <unordered_map>
#include <assert.h>
#include <vector>
#include <numeric>
#include <stdexcept>
#include <algorithm>
#include <iterator>

void ParseOpenParenthesis(bool& nextIsUnary, std::stack<std::string>& operators);
void ParseVariable(bool& nextIsUnary, const std::vector<Token>::iterator& token, std::stack<std::unique_ptr<ExpressionBase>>& expressions);
void ParseConstant(bool& nextIsUnary, const std::vector<Token>::iterator& token, std::stack<std::unique_ptr<ExpressionBase>>& expressions);
void ParseOperator(bool& nextIsUnary, const std::vector<Token>::iterator& token, std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions);
void ParseCloseParenthesis(bool& nextIsUnary, std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions, bool lastToken);

std::unique_ptr<ExpressionBase> BuildBinaryExpression(std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions);
std::unique_ptr<ExpressionBase> BuildUnaryExpression(std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions);


std::unique_ptr<ExpressionBase> BuildExpression(std::vector<Token> input)
{
    // Reference: CS3901 - Introduction to Data Structures How to Parse Arithmetic Expressions

    if (input.empty()) throw std::invalid_argument("Input cannot be empty");

    input.insert(input.begin(), Token::CreateOperator('('));
    input.emplace_back(Token::CreateOperator(')'));

    std::stack<std::string> operators;
    std::stack<std::unique_ptr<ExpressionBase>> expressions;

    bool nextIsUnary = true;

    for (auto token = input.begin(); token != input.end(); ++token)
    {
        // Open parenthesis
        if (token->IsOperator() && token->GetOperator() == '(')
        {
            ParseOpenParenthesis(nextIsUnary, operators);
        }

        // Close parenthesis
        else if (token->IsOperator() && token->GetOperator() == ')')
        {
            ParseCloseParenthesis(nextIsUnary, operators, expressions, std::next(token) == input.end());
        }

        // Other Operator
        else if (token->IsOperator())
        {
            ParseOperator(nextIsUnary, token, operators, expressions);
        }

        // Number
        else if (token->IsConstant())
        {
            ParseConstant(nextIsUnary, token, expressions);
        }

        // Variable
        else if (token->IsVariable())
        {
            ParseVariable(nextIsUnary, token, expressions);
        }

        else
        {
            // This should never happen
            assert(!"You have made a new type of token without editing the parsing code");
        }
    }

    if (!operators.empty() || expressions.size() != 1) 
        throw std::invalid_argument("Invalid expression: unbalanced parenthesis");

    return std::move(expressions.top());
}

void ParseCloseParenthesis(bool& nextIsUnary, std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions, bool lastToken)
{
    if (nextIsUnary)
        throw std::invalid_argument("Invalid expression: '()' is invalid");

    while (operators.top() != "(")
    {
        if (operators.top() == "unary")
        {
            expressions.emplace(BuildUnaryExpression(operators, expressions));
        }
        else
        {
            expressions.emplace(BuildBinaryExpression(operators, expressions));
        }
    }

    operators.pop();

    if (operators.empty() && !lastToken)
    {
        throw std::invalid_argument("Invalid expression: unbalanced parenthesis");
    }

    nextIsUnary = false;
}

void ParseOperator(bool& nextIsUnary, const std::vector<Token>::iterator& token, std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions)
{
    static const std::unordered_map<std::string, int> prio =
    {
        { "^", 5 },         // 'current' exponent
        { "exponent", 4 },  // 'old' exponent which is further left
        { "unary", 3 },
        { "*", 2 },
        { "/", 2 },
        { "+", 1 },
        { "-", 1 },
        { "(", 0 },
    };

    if (nextIsUnary)
    {
        if (token->GetOperator() == '-')
        {
            operators.emplace(std::string{ token->GetOperator() });
            operators.emplace("unary");
        }
        else
        {
            throw std::invalid_argument("Invalid expression: only '-' can be unary, not '" + std::string{ token->GetOperator() } + "')");
        }
    }
    else
    {
        while (prio.at(operators.top()) >= prio.at({ token->GetOperator() }))
        {
            if (operators.top() == "unary")
            {
                expressions.emplace(BuildUnaryExpression(operators, expressions));
            }
            else
            {
                expressions.emplace(BuildBinaryExpression(operators, expressions));
            }
        }

        if (token->GetOperator() == '^')
        {
            operators.emplace("exponent");
        }
        else
        {
            operators.emplace(std::string{ token->GetOperator() });
        }
    }

    nextIsUnary = true;
}

void ParseVariable(bool& nextIsUnary, const std::vector<Token>::iterator& token, std::stack<std::unique_ptr<ExpressionBase>>& expressions)
{
    if (!nextIsUnary)
        throw std::invalid_argument("Invalid expression: variable ('" + std::string{ token->GetVariable() } + "') directly after term");

    expressions.push(std::make_unique<Variable>(token->GetVariable()));

    nextIsUnary = false;
}

void ParseConstant(bool& nextIsUnary, const std::vector<Token>::iterator& token, std::stack<std::unique_ptr<ExpressionBase>>& expressions)
{
    if (!nextIsUnary)
        throw std::invalid_argument("Invalid expression: constant ('" + std::to_string(token->GetConstant()) + "') directly after term");

    expressions.push(std::make_unique<Constant>(token->GetConstant()));

    nextIsUnary = false;
}

std::unique_ptr<ExpressionBase> BuildBinaryExpression(std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions)
{
    if (operators.size() < 1) throw std::invalid_argument("Invalid expression: trying to build binary expression with empty operator stack");
    if (expressions.size() < 2) throw std::invalid_argument("Invalid expression: binary expression '" + operators.top() + "' without two operands");

    auto op = operators.top();
    operators.pop();

    auto rhs = std::move(expressions.top());
    expressions.pop();

    auto lhs = std::move(expressions.top());
    expressions.pop();

    if (op == "+")
        return std::make_unique<OperatorPlus>(lhs.release(), rhs.release());
    if (op == "-")
        return std::make_unique<OperatorMinus>(lhs.release(), rhs.release());
    if (op == "/")
        return std::make_unique<OperatorDivide>(lhs.release(), rhs.release());
    if (op == "*")
        return std::make_unique<OperatorMultiply>(lhs.release(), rhs.release());
    if (op == "exponent")
        return std::make_unique<OperatorExponent>(lhs.release(), rhs.release());

    throw std::invalid_argument("Invalid expression: could not build binary expression with operator '" + op + "'");
}

std::unique_ptr<ExpressionBase> BuildUnaryExpression(std::stack<std::string>& operators, std::stack<std::unique_ptr<ExpressionBase>>& expressions)
{
    if (operators.size() < 2) throw std::invalid_argument("Invalid expression: trying to build unary expression with empty operator stack");

    operators.pop();    // "unary"

    if (expressions.size() < 1) throw std::invalid_argument("Invalid expression: unary expression '" + operators.top() + "' without operand");

    auto op = operators.top();
    operators.pop();

    auto rhs = std::move(expressions.top());
    expressions.pop();

    if (op == "-")
        return std::make_unique<OperatorUnaryMinus>(rhs.release());

    throw std::invalid_argument("Invalid expression: could not build unary expression with operator '" + op + "'");
}

void ParseOpenParenthesis(bool& nextIsUnary, std::stack<std::string>& operators)
{
    if (!nextIsUnary)
        throw std::invalid_argument("Invalid expression: '(' directly after term");

    operators.emplace("(");

    nextIsUnary = true;
}

