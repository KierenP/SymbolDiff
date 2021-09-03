#pragma once
#include "Lexer.h"
#include "Expression.h"

std::unique_ptr<ExpressionBase> BuildExpression(std::vector<Token> input);