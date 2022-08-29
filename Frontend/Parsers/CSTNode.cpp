//
// Created by theflysong on 2022/8/29
//

#include <Parsers/CSTNode.hpp>

namespace Hoshi {
    CSTNode::CSTNode(int Line, int Column)
        : Line(Line), Column(Column) {
    }
    
    CSTNode::~CSTNode() = default;
}