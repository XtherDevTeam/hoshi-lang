//
// Created by XIaokang00010 on 2023/1/24.
//

#include "lexer.hpp"

namespace yoi {
    lexer::lexer(std::wstringstream ss) : stream(std::move(ss)), line(0), col(0), curCh() {
        getCh();
    }

    void lexer::getCh() {
        start:
        if (!stream) {
            throw std::runtime_error("hoshi::lexer::getCh() - eof");
        }
        if (!stream.get(curCh) || stream.fail()) {
            curCh = '\0';
        }
        if (curCh == L'\n') {
            line++, col = 0;
        } else if (curCh == L'\r') {
            goto start;
        } else {
            col++;
        }
    }

    lexer::token lexer::scan() {
        if (curCh == '\0') {
            return curToken = lexer::token {line, col, token::tokenKind::eof};
        }
        while (curCh == ' ' or curCh == '\n' or curCh == '\r' or curCh == '\t') getCh();
        if (std::isalpha(curCh) or curCh == '_') {
            return curToken = alphaStart();
        } else if (std::isdigit(curCh)) {
            return curToken = digitStart();
        } else if (curCh == L'+') {
            return curToken = plusStart();
        } else if (curCh == L'-') {
            return curToken = minusStart();
        } else if (curCh == L'*') {
            return curToken = asteriskStart();
        } else if (curCh == L'/') {
            return curToken = slashStart();
        } else if (curCh == L'%') {
            return curToken = percentSignStart();
        } else if (curCh == L'!') {
            return curToken = notStart();
        } else if (curCh == L'=') {
            return curToken = equalStart();
        } else if (curCh == L'>') {
            return curToken = greaterStart();
        } else if (curCh == L'<') {
            return curToken = lessStart();
        } else if (curCh == L'"' or curCh == L'\'') {
            return curToken = strStart();
        } else if (curCh == L';') {
            return curToken = semicolonStart();
        } else if (curCh == L':') {
            return curToken = colonStart();
        } else if (curCh == L',') {
            return curToken = commaStart();
        } else if (curCh == L'.') {
            return curToken = dotStart();
        } else if (curCh == L'(') {
            return curToken = leftParenthesesStart();
        } else if (curCh == L')') {
            return curToken = rightParenthesesStart();
        } else if (curCh == L'[') {
            return curToken = leftBracketStart();
        } else if (curCh == L']') {
            return curToken = rightBracketStart();
        } else if (curCh == L'{') {
            return curToken = leftBracesStart();
        } else if (curCh == L'}') {
            return curToken = rightBracesStart();
        } else if (curCh == L'&') {
            return curToken = andStart();
        } else if (curCh == L'|') {
            return curToken = orStart();
        } else if (curCh == L'^') {
            return curToken = xorStart();
        } else if (curCh == L'#') {
            return curToken = sharpStart();
        } else if (curCh == L'!') {
            return curToken = notStart();
        } else if (curCh == L'\0') {
            return curToken = {line, col, token::tokenKind::eof, token::vBasicValue{false}};
        } else {
            throw std::runtime_error("hoshi::lexer::scan() - undefined token");
        }
    }

    lexer::token lexer::alphaStart() {
        lexer::token tok{line, col, token::tokenKind::identifier, wstr()};
        wstr tempStr;
        tempStr += curCh;
        getCh();
        while (isalpha(curCh) or isdigit(curCh) or curCh == L'_') {
            tempStr += curCh;
            getCh();
        }

        // 关键词处理
        if (tempStr == L"return") {
            tok.kind = token::tokenKind::kReturn;
        } else if (tempStr == L"continue") {
            tok.kind = token::tokenKind::kContinue;
        } else if (tempStr == L"break") {
            tok.kind = token::tokenKind::kBreak;
        } else if (tempStr == L"for") {
            tok.kind = token::tokenKind::kFor;
        } else if (tempStr == L"forEach") {
            tok.kind = token::tokenKind::kForEach;
        } else if (tempStr == L"while") {
            tok.kind = token::tokenKind::kWhile;
        } else if (tempStr == L"func") {
            tok.kind = token::tokenKind::kFunc;
        } else if (tempStr == L"use") {
            tok.kind = token::tokenKind::kUse;
        } else if (tempStr == L"let") {
            tok.kind = token::tokenKind::kLet;
        } else if (tempStr == L"cast") {
            tok.kind = token::tokenKind::kCast;
        } else if (tempStr == L"in") {
            tok.kind = token::tokenKind::kIn;
        } else if (tempStr == L"if") {
            tok.kind = token::tokenKind::kIf;
        } else if (tempStr == L"else") {
            tok.kind = token::tokenKind::kElse;
        } else if (tempStr == L"elif") {
            tok.kind = token::tokenKind::kElif;
        } else if (tempStr == L"interface") {
            tok.kind = token::tokenKind::kInterface;
        } else if (tempStr == L"constructor") {
            tok.kind = token::tokenKind::kConstructor;
        } else if (tempStr == L"struct") {
            tok.kind = token::tokenKind::kStruct;
        } else if (tempStr == L"impl") {
            tok.kind = token::tokenKind::kImpl;
        } else if (tempStr == L"null") {
            tok.kind = token::tokenKind::kNull;
        } else if (tempStr == L"import") {
            tok.kind = token::tokenKind::kImport;
        } else if (tempStr == L"export") {
            tok.kind = token::tokenKind::kExport;
        } else if (tempStr == L"as") {
            tok.kind = token::tokenKind::kAs;
        } else if (tempStr == L"from") {
            tok.kind = token::tokenKind::kFrom;
        } else if (tempStr == L"type_id") {
            tok.kind = token::tokenKind::kTypeId;
        } else if (tempStr == L"dyn_cast") {
            tok.kind = token::tokenKind::kDynCast;
        } else if (tempStr == L"try") {
            tok.kind = token::tokenKind::kTry;
        } else if (tempStr == L"catch") {
            tok.kind = token::tokenKind::kCatch;
        } else if (tempStr == L"finally") {
            tok.kind = token::tokenKind::kFinally;
        } else if (tempStr == L"throw") {
            tok.kind = token::tokenKind::kThrow;
        } else if (tempStr == L"noffi") {
            tok.kind = token::tokenKind::kNoFFI;
        } else if (tempStr == L"always_inline") {
            tok.kind = token::tokenKind::kAlwaysInline;
        } else if (tempStr == L"new") {
            tok.kind = token::tokenKind::kNew;
        } else if (tempStr == L"true" or tempStr == L"false") {
            tok.kind = token::tokenKind::boolean;
            tok.basicVal.vBool = tempStr == L"true";
        } else {
            tok.strVal = tempStr;
        }

        return tok;
    }

    lexer::token lexer::strStart() {
        wchar strV = curCh;
        lexer::token tok{line, col, strV == L'"' ? token::tokenKind::string : token::tokenKind::character, wstr()};
        getCh();
        while (curCh != strV) {
            if (curCh == '\\') {
                getCh();
                tok.strVal += '\\';
            }
            tok.strVal += curCh;
            getCh();
        }
        getCh(); // skip "
        std::wistringstream ss{tok.strVal};
        tok.strVal = {};
        parseString(ss, tok.strVal);
        if (strV == L'\'' && tok.strVal.size() > 1)
            throw std::runtime_error("lexer::strStart() - character literal length > 1");
        return tok;
    }

    lexer::token lexer::digitStart() {
        lexer::token tok{line, col, token::tokenKind::integer, token::vBasicValue{(int64_t) 0}};
        wstr tempStr;
        tempStr += curCh;
        getCh();
        while (isdigit(curCh)) {
            tempStr += curCh;
            getCh();
        }
        if (curCh == '.') {
            tok.kind = token::tokenKind::decimal;
            tempStr += curCh;
            getCh();
            while (isdigit(curCh)) {
                tempStr += curCh;
                getCh();
            }
        }
        if (tok.kind == token::tokenKind::integer)
            tok.basicVal.vInt = std::stol(tempStr);
        else
            tok.basicVal.vDeci = std::stof(tempStr);
        return tok;
    }

    lexer::token lexer::minusStart() {
        lexer::token tok{line, col, token::tokenKind::minus};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::subtractionAssignment;
            getCh();
        } else if (curCh == '-') {
            tok.kind = token::tokenKind::decrementSign;
            getCh();
        } else if (curCh == '>') {
            tok.kind = token::tokenKind::toSign;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::plusStart() {
        lexer::token tok{line, col, token::tokenKind::plus};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::additionAssignment;
            getCh();
        } else if (curCh == '+') {
            tok.kind = token::tokenKind::incrementSign;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::asteriskStart() {
        lexer::token tok{line, col, token::tokenKind::asterisk};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::multiplicationAssignment;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::slashStart() {
        lexer::token tok{line, col, token::tokenKind::slash};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::divisionAssignment;
            getCh();
        } else if (curCh == '/') {
            getCh();
            while (curCh and curCh != '\n')
                getCh();
            return scan(); // 单行注释解析
        } else if (curCh == '*') {
            getCh();
            while (curCh) {
                if (curCh == '*') {
                    getCh();
                    if (curCh == '/') {
                        getCh();
                        break;
                    }
                }
                getCh();
            }
            return scan(); // 多行注释解析
        }
        return tok;
    }

    lexer::token lexer::percentSignStart() {
        lexer::token tok{line, col, token::tokenKind::percentSign};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::reminderAssignment;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::equalStart() {
        lexer::token tok{line, col, token::tokenKind::assignSign};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::equal;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::notStart() {
        lexer::token tok{line, col, token::tokenKind::logicNot};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::notEqual;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::lessStart() {
        lexer::token tok{line, col, token::tokenKind::lessThan};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::lessEqual;
            getCh();
        } else if (curCh == '<') {
            tok.kind = token::tokenKind::binaryShiftLeft;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::greaterStart() {
        lexer::token tok{line, col, token::tokenKind::greaterThan};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::greaterEqual;
            getCh();
        } else if (curCh == '>') {
            tok.kind = token::tokenKind::binaryShiftRight;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::semicolonStart() {
        lexer::token tok{line, col, token::tokenKind::semicolon};
        getCh();
        return tok;
    }

    lexer::token lexer::colonStart() {
        lexer::token tok{line, col, token::tokenKind::colon};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::directAssignSign;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::commaStart() {
        lexer::token tok{line, col, token::tokenKind::comma};
        getCh();
        return tok;
    }

    lexer::token lexer::dotStart() {
        lexer::token tok{line, col, token::tokenKind::dot};
        getCh();
        return tok;
    }

    lexer::token lexer::leftParenthesesStart() {
        lexer::token tok{line, col, token::tokenKind::leftParentheses};
        getCh();
        return tok;
    }

    lexer::token lexer::rightParenthesesStart() {
        lexer::token tok{line, col, token::tokenKind::rightParentheses};
        getCh();
        return tok;
    }

    lexer::token lexer::leftBracketStart() {
        lexer::token tok{line, col, token::tokenKind::leftBracket};
        getCh();
        return tok;
    }

    lexer::token lexer::rightBracketStart() {
        lexer::token tok{line, col, token::tokenKind::rightBracket};
        getCh();
        return tok;
    }

    lexer::token lexer::leftBracesStart() {
        lexer::token tok{line, col, token::tokenKind::leftBraces};
        getCh();
        return tok;
    }

    lexer::token lexer::rightBracesStart() {
        lexer::token tok{line, col, token::tokenKind::rightBraces};
        getCh();
        return tok;
    }

    void lexer::saveState() {
        states.emplace_back(line, col, (int64_t) stream.tellg(), curCh, curToken);
    }

    void lexer::returnState() {
        stream.clear();
        lexerState &state = states.back();
        line = state.line, col = state.col, curCh = state.curCh, curToken = state.curToken;
        stream.seekg(state.pos);
        dropState();
    }

    void lexer::dropState() {
        if (!states.empty())
            states.pop_back();
    }

    lexer::token lexer::andStart() {
        lexer::token tok{line, col, token::tokenKind::binaryAnd};
        getCh();
        if (curCh == '&') {
            tok.kind = token::tokenKind::logicAnd;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::orStart() {
        lexer::token tok{line, col, token::tokenKind::binaryOr};
        getCh();
        if (curCh == '|') {
            tok.kind = token::tokenKind::logicOr;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::xorStart() {
        lexer::token tok{line, col, token::tokenKind::binaryXor};
        getCh();
        return tok;
    }

    lexer::token lexer::sharpStart() {
        lexer::token tok{line, col, token::tokenKind::sharp};
        getCh();
        return tok;
    }

    lexer::token lexer::binaryNotStart() {
        lexer::token tok{line, col, token::tokenKind::binaryNot};
        getCh();
        return tok;
    }

    lexer::token::vBasicValue::vBasicValue(int64_t v) : vInt(v) {

    }

    lexer::token::vBasicValue::vBasicValue(double v) : vDeci(v) {

    }

    lexer::token::vBasicValue::vBasicValue(bool v) : vBool(v) {

    }

    lexer::token::vBasicValue::vBasicValue() : vInt(0) {

    }


    lexer::token::token() : line(), col(), kind(), basicVal(), strVal() {

    }

    lexer::token::token(int64_t line, int64_t col, lexer::token::tokenKind kind) :
            line(line), col(col), kind(kind), basicVal(), strVal() {

    }

    lexer::token::token(int64_t line, int64_t col, lexer::token::tokenKind kind, lexer::token::vBasicValue basicVal) :
            line(line), col(col), kind(kind), basicVal(basicVal), strVal() {

    }

    lexer::token::token(int64_t line, int64_t col, lexer::token::tokenKind kind, wstr strVal) :
            line(line), col(col), kind(kind), basicVal(), strVal(std::move(strVal)) {

    }


    lexer::lexerState::lexerState() : line(), col(), pos(), curCh(), curToken() {

    }

    lexer::lexerState::lexerState(int64_t line, int64_t col, std::istream::pos_type pos, wchar curCh,
                                  lexer::token curToken)
            :
            line(line), col(col), pos(pos), curCh(curCh), curToken(std::move(curToken)) {

    }
} // hoshi