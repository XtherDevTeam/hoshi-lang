//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_CSTNODE_HPP
#define XSCRIPT2_CSTNODE_HPP

#include <Lexer.hpp>
#include <Config.hpp>

namespace Hoshi {
    class CSTNode {
    public:
        virtual XString GetNodeType(void) = 0;
        template<typename NodeType> class Parser {
        public:
            virtual NodeType Parse(Lexer L) = 0;
        };
    };
}

#endif