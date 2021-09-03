#include "Parser.h"
#include <stack>
#include <unordered_map>
#include <assert.h>
#include <vector>
#include <numeric>
#include <typeindex>
#include <stdexcept>
#include <algorithm>
#include <iterator>

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

std::unique_ptr<ExpressionBase> BuildExpression(std::vector<Token> input)
{
    // Reference: CS3901 - Introduction to Data Structures How to Parse Arithmetic Expressions

    if (input.empty()) throw std::invalid_argument("Input cannot be empty");

    input.insert(input.begin(), Token::CreateOperator('('));
    input.emplace_back(Token::CreateOperator(')'));

    std::stack<std::string> operators;
    std::stack<std::unique_ptr<ExpressionBase>> expressions;

    bool nextIsUnary = true;

    static const std::unordered_map<std::string, int> prio =
    {
        { "^", 5 },
        { "exponent", 4 },
        { "unary", 3 },
        { "*", 2 },
        { "/", 2 },
        { "+", 1 },
        { "-", 1 },
        { "(", 0 },
    };

    for (auto token = input.begin(); token != input.end(); ++token)
    {
        // Open parenthesis
        if (token->IsOperator() && token->GetOperator() == '(')
        {
            if (!nextIsUnary)
                throw std::invalid_argument("Invalid expression: '(' directly after term");

            operators.emplace("(");

            nextIsUnary = true;
        }

        // Close parenthesis
        else if (token->IsOperator() && token->GetOperator() == ')')
        {
            if (nextIsUnary)
                throw std::invalid_argument("Invalid expression: '()' is invalid");

            while (operators.top() != "(")
            {
                if (operators.top() == "unary")
                {
                    // assemble_unary_expression
                }
                else
                {
                    expressions.emplace(BuildBinaryExpression(operators, expressions));
                }
            }

            operators.pop();

            if (operators.empty() && std::next(token) != input.end())
            {
                throw std::invalid_argument("Invalid expression: unbalanced parenthesis");
            }

            nextIsUnary = false;
        }

        // Other Operator
        else if (token->IsOperator())
        {
            if (nextIsUnary)
            {
                if (token->GetOperator() == '-')
                {
                    //push "- ?" onto expressions
                    operators.emplace(std::string{ token->GetOperator() });
                    operators.emplace("unary");
                }
                else
                {
                    throw std::invalid_argument("Invalid expression: only '-' can be unary, not '" + std::string{ token->GetVariable() } + "')");
                }
            }
            else
            {
                while (prio.at(operators.top()) >= prio.at({ token->GetOperator() }))
                {
                    if (operators.top() == "unary")
                    {
                        // assemble_unary_expression
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

        // Number
        else if (token->IsConstant())
        {
            if (!nextIsUnary)
                throw std::invalid_argument("Invalid expression: constant ('" + std::to_string(token->GetConstant()) + "') directly after term");

            expressions.push(std::make_unique<Constant>(token->GetConstant()));

            nextIsUnary = false;
        }

        // Variable
        else if (token->IsVariable())
        {
            if (!nextIsUnary)
                throw std::invalid_argument("Invalid expression: variable ('" + std::string{ token->GetVariable() } + "') directly after term");

            expressions.push(std::make_unique<Variable>(token->GetVariable()));

            nextIsUnary = false;
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

