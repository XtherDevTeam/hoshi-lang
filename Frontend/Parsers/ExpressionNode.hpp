//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_EXPRESSIONNODE_HPP
#define XSCRIPT2_EXPRESSIONNODE_HPP

#include <Parsers/BinaryShiftExpressionNode.hpp>

namespace Hoshi {
    using ExpressionNode = BinaryShiftExpressionNode;
    #define EXPRESSION_FIRST BINARYSHIFT_FIRST
}

#endif