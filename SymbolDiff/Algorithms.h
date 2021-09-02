#pragma once
#include "Parser.h"

std::string Differentiate(const std::string& str, char wrt);
bool ExpressionsNumericallyEqual(const ExpressionBase& lhs, const ExpressionBase& rhs);