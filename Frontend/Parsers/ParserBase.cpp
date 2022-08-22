//
// Created by Jerry Chou on 22-8-21.
//

#include "ParserBase.hpp"
#include "../../Share/Exceptions/ParserException.hpp"

namespace Hoshi {
    namespace Parser {
        ParserBase::ParserBase(Hoshi::Lexer& L) : L(L), Line(L.Line), Col(L.Column), LT(L.LastToken) {

        }

        void ParserBase::Throw(const XString &ParserName, const XString &Reason) const {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"[" + ParserName + L"] " + Reason);
        }

        void ParserBase::Rollback() {
            L.Line = Line;
            L.Column = Col;
            L.LastToken = LT;
        }

        void ParserBase::UpdateBackup() {
            Line = L.Line;
            Col = L.Column;
            LT = L.LastToken;
        }
    } // Hoshi
} // Parser