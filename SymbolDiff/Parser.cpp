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

