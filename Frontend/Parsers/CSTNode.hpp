//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_CSTNODE_HPP
#define XSCRIPT2_CSTNODE_HPP

#include <Lexer.hpp>
#include <Config.hpp>
#include <Exceptions/ParserException.hpp>

namespace Hoshi {
    class CSTNode {
    public:
        XInteger Line;
        XInteger Column;

        CSTNode(XInteger Line, XInteger Column);

        virtual ~CSTNode();

        virtual XString GetNodeType() = 0;

        template<typename NodeType>
        class Parser {
        protected:
            std::set<Lexer::TokenKind> FIRST;
        public:
            Parser(std::initializer_list<Lexer::TokenKind> FIRST)
                    : FIRST(FIRST) {
            }

            bool IsFirstToken(Lexer::Token token) {
                return FIRST.find(token.Kind) != FIRST.end();
            }

            virtual NodeType *Parse(Lexer &L) = 0;
        };
    };
}

#endif