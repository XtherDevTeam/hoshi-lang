//
// Created by Jerry Chou on 2022/5/7.
//

#include <Lexer.hpp>

namespace Hoshi {
    LexerException::LexerException() = default;

    LexerException::LexerException(XIndexType Line, XIndexType Column, const XString &Reason) {
        message = "[Position " + std::to_string(Line) + ":" + std::to_string(Column) + "] Lexer Error: " +
                  wstring2string(Reason);
    }

    const char *LexerException::what() const noexcept {
        return message.c_str();
    }

    Lexer::Lexer() : Line(0), Column(0), Position(0) {}

    Lexer::Lexer(XString String) : Line(0), Column(0), Position(0), String(std::move(String)) {
    }

    XCharacter Lexer::NextCharacter() {
        if (Position >= String.length()) {
            return L'\0';
        }
        XCharacter Result = String[Position];
        if (Result == L'\n') {
            Line++;
            Column = 0;
        } else {
            Column++;
        }
        Position++;
        return Result;
    }

#pragma region Scan
    Lexer::Token Lexer::Scan() {
        /* White spaces */
        while (String[Position] == L' ' || String[Position] == L'\t' ||
               String[Position] == L'\n' || String[Position] == L'\r')
            NextCharacter();
        /* Literals */
        /* Identifiers or reserved words */
        if (IsAlpha(String[Position])) {
            return LastToken = ScanAsIdentifier();
        }
            /* Digital literals */
        else if (IsDigit(String[Position])) {
            return LastToken = ScanAsDigital();
        }
            /* String literals */
        else if (String[Position] == L'"') {
            return LastToken = ScanAsReadonlyStringLiteral();
        }
            /* Character literals */
        else if (String[Position] == L'\'') {
            return ScanAsCharacter();
        }
        
        switch (String[Position]) {
        case L'^': case L')': case L'(': case L'[': case L']': case L'{':
        case L'}': case L';': case L',': case L'.': case L'~': case L'\0':
            return LastToken = ScanAsSingleSign();
        case L'+': case L'-': case L'*': case L'/': case L'%': case L':':
            return LastToken = ScanAsMultipleSign();
        case L'=': case L'!': case L'>': case L'<': case L'|': case L'&': 
            return LastToken = ScanAsLogicSign();
        }
        /* Unknown tokens */
        throw LexerException(Line, Column, L"Unknown token : " + std::to_wstring(String[Position]));
    }

    Lexer::Token Lexer::ScanAsIdentifier(void) {
        XString TempStr;
        XIndexType Start = Position;
        while (String[Position] and (IsAlpha(String[Position]) or IsDigit(String[Position]))) NextCharacter();
        TempStr = String.substr(Start, Position - Start);
        /* Identify boolean literals */
        if (TempStr == L"True" or TempStr == L"False") {
            return {TokenKind::Boolean, TempStr, Line, Column};
        }
        else {
            bool IsKeywords = std::find(LexerReservedWords.begin(), LexerReservedWords.end(), TempStr) != LexerReservedWords.end();
            return {IsKeywords ? TokenKind::Keywords : TokenKind::Identifier, TempStr, Line, Column};
        }
    } 

    Lexer::Token Lexer::ScanAsDigital(void) {
        XString TempStr{};
        XIndexType Start = Position;
        while (String[Position] && IsDigit(String[Position])) NextCharacter();
        TempStr = String.substr(Start, Position - Start);

        if (String[Position] == L'.') { // Is Decimal
            NextCharacter();
            Start = Position;
            while (String[Position] && IsDigit(String[Position])) NextCharacter();
            TempStr += L'.';
            TempStr.append(String.substr(Start, Position - Start));

            return {TokenKind::Decimal, TempStr, Line, Column};
        } else {
            return {TokenKind::Integer, TempStr, Line, Column};
        }
    }

    /**
     * @brief Get next token as ReadonlyStringLiteral
     * @return The next token
     */
    Lexer::Token Lexer::ScanAsReadonlyStringLiteral(void) {
        XString TempStr{};
        XCharacter Type = String[Position];
        NextCharacter();
        XIndexType Start = Position;
        while (String[Position] && String[Position] != Type) {
            if (String[Position] == L'\\') {
                NextCharacter();
            }
            NextCharacter();
        }
        TempStr = String.substr(Start, Position - Start);
        NextCharacter(); // Skip
        return {TokenKind::String, unescape_string(TempStr), Line, Column};
    }
    /**
     * @brief Get next token as Character
     * @return The next token
     */
    Lexer::Token Lexer::ScanAsCharacter(void) {
        XString TempStr{};
        XCharacter Type = String[Position];
        NextCharacter();
        XIndexType Start = Position;
        while (String[Position] and String[Position] != Type) {
            if (String[Position] == L'\\') {
                NextCharacter();
            }
            NextCharacter();
        }
        TempStr = String.substr(Start, Position - Start);
        NextCharacter(); // Skip
        return {TokenKind::Character, unescape_string(TempStr), Line, Column};
    }
    /**
     * @brief Get next token as Single Sign
     * @return The next token
     */
    Lexer::Token Lexer::ScanAsSingleSign(void) {
            /* XOR */
        XCharacter Sign = String[Position];
        NextCharacter();;
        switch (Sign) {
        case L'^': return {TokenKind::BinaryXOR, L"^", Line, Column};
        case L'(': return {TokenKind::LeftParentheses, L"(", Line, Column};
        case L')': return {TokenKind::RightParentheses, L")", Line, Column};
        case L'[': return {TokenKind::LeftBracket, L"[", Line, Column};
        case L']': return {TokenKind::RightBracket, L"]", Line, Column};
        case L'{': return {TokenKind::LeftBraces, L"{", Line, Column};
        case L'}': return {TokenKind::RightBraces, L"}", Line, Column};
        case L';': return {TokenKind::Semicolon, L";", Line, Column};
        case L',': return {TokenKind::Colon, L",", Line, Column};
        case L'.': return {TokenKind::Dot, L".", Line, Column};
        case L'~': return {TokenKind::Invert, L"~", Line, Column};
        case L'\0': return {TokenKind::EoF, L"", Line, Column};
        default: return {};
        }
    }
    /**
     * @brief Get next token as Multiple Sign
     * @return The next token
     */
    Lexer::Token Lexer::ScanAsMultipleSign(void) {
            /* Expression symbols */
            /* Starts with :*/
        if (String[Position] == L':') {
            NextCharacter();
            if (String[Position] == L':') {
                NextCharacter();
                return {TokenKind::StaticMemberAccessSign, L"::", Line, Column};
            }
            return {TokenKind::TypeDescriptorSign, L":", Line, Column};
        }
            /* Starts with + */
        if (String[Position] == L'+') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::AdditionAssignment, L"+=", Line, Column};
            } 
            if (String[Position] == L'+') {
                NextCharacter();   // Read after '+'
                return {TokenKind::IncrementSign, L"++", Line, Column};
            } 
            return {TokenKind::Plus, L"+", Line, Column};
        }
            /* Starts with - */
        if (String[Position] == L'-') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::SubtractionAssignment, L"-=", Line, Column};
            }
            if (String[Position] == L'-') {
                NextCharacter();   // Read after '+'
                return {TokenKind::DecrementSign, L"--", Line, Column};
            }
            if (String[Position] == L'>') {
                NextCharacter();   // Read after '>'
                return {TokenKind::ToSign, L"->", Line, Column};
            }
            return {TokenKind::Minus, L"-", Line, Column};
        }
            /* Starts with * */
        if (String[Position] == L'*') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::MultiplicationAssignment, L"*=", Line, Column};
            }
            return {TokenKind::Asterisk, L"*", Line, Column};
        }
            /* Starts with / */
        if (String[Position] == L'/') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::DivisionAssignment, L"/=", Line, Column};
            } else if (String[Position] == L'/') {
                while (NextCharacter() != L'\n' and String[Position] != L'\0');
                return Scan();
            } else if (String[Position] == L'*') {
                NextCharacter();
                while ((String[Position] != L'*' or String[Position+1] != L'/') and String[Position] != '\0')
                    NextCharacter();
                NextCharacter();
                NextCharacter();
                return Scan();
            }
            return {TokenKind::Slash, (std::wstring){std::filesystem::path::preferred_separator}, Line, Column};
        }
            /* Starts with % */
        if (String[Position] == L'%') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::RemainderAssignment, L"%=", Line, Column};
            }
            return {TokenKind::PercentSign, L"%", Line, Column};
        }
        return {};
    }

    /**
     * @brief Get next token as Logic Sign
     * @return The next token
     */
    Lexer::Token Lexer::ScanAsLogicSign(void) {
        /* Logic symbols */
        /* Starts with = */
        if (String[Position] == L'=') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::Equal, L"==", Line, Column};
            }
            if (String[Position] == L'>') {
                NextCharacter();   // Read after '>'
                return {TokenKind::TypeCastingSign, L"=>", Line, Column};
            }
            return {TokenKind::AssignSign, L"=", Line, Column};
        }
            /* Starts with ! */
        if (String[Position] == L'!') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::NotEqual, L"!=", Line, Column};
            } 
            throw LexerException(Line, Column, L"Unknown token : " + std::to_wstring(String[Position]));
        }
            /* Starts with < */
        if (String[Position] == L'<') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::LessThanOrEqual, L"<=", Line, Column};
            }
            if (String[Position] == '<') {
                NextCharacter();   // Read after '>'
                return {TokenKind::BinaryLeftMove, L"<<", Line, Column};
            }
            return {TokenKind::LessThan, L"<", Line, Column};
        }
            /* Starts with > */
        if (String[Position] == L'>') {
            NextCharacter();
            if (String[Position] == L'=') {
                NextCharacter();   // Read after '='
                return {TokenKind::MoreThanOrEqual, L">=", Line, Column};
            } 
            if (String[Position] == '>') {
                NextCharacter();   // Read after '>'
                return {TokenKind::BinaryRightMove, L">>", Line, Column};
            }
            return {TokenKind::MoreThan, L">", Line, Column};
        }
            /* Binary symbols */
            /* Or */
        if (String[Position] == L'|') {
            NextCharacter();
            if (String[Position] == L'|') {
                NextCharacter();
                return {TokenKind::LogicOr, L"||", Line, Column};
            }
            return {TokenKind::BinaryOr, L"|", Line, Column};
        }
            /* And */
        if (String[Position] == L'&') {
            NextCharacter();
            if (String[Position] == L'&') {
                NextCharacter();
                return {TokenKind::LogicAnd, L"&&", Line, Column};
            }
            return {TokenKind::BinaryAnd, L"&", Line, Column};
        }
        return {};
    }
#pragma endregion

    void Lexer::Reset() {
        Line = Column = Position = 0;
    }

    Lexer::Token::Token() : Kind(TokenKind::EoF), Line(0), Column(0), Value() {}

    Lexer::Token::Token(Lexer::TokenKind Kind, XString Value, XIndexType Line, XIndexType Column) 
        : Kind(Kind), Line(Line), Column(Column), Value(std::move(Value)) {
    }

    bool Lexer::Token::operator==(const Lexer::Token &rhs) const {
        return (Kind == rhs.Kind && Value == rhs.Value);
    }

    bool Lexer::Token::operator!=(const Lexer::Token &rhs) const {
        return !(Kind == rhs.Kind && Value == rhs.Value);
    }
} // Hoshi