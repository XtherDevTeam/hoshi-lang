//
// Created by Jerry Chou on 22-8-21.
//

#ifndef HOSHI_LANG_PARSERBASE_HPP
#define HOSHI_LANG_PARSERBASE_HPP

#include "../Lexer.hpp"
#include "../AST.hpp"

namespace Hoshi {
    namespace Parser {

        class ParserBase {
        protected:
            Hoshi::Lexer &L;
            Hoshi::Lexer::Token LT;
            XIndexType Line, Col;

            void Throw(const XString &ParserName, const XString &Reason) const;

            void UpdateBackup();

            void Rollback();
        public:
            explicit ParserBase(Hoshi::Lexer& L);

            virtual Hoshi::AST Parse() = 0;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_PARSERBASE_HPP
