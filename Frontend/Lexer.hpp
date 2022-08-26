//
// Created by Jerry Chou on 2022/5/7.
//

#ifndef XSCRIPT2_LEXER_HPP
#define XSCRIPT2_LEXER_HPP

#include <algorithm>

#include "Config.hpp"
#include "Utils.hpp"
#include "HoshiException.hpp"

namespace Hoshi {

    /**
      * Keywords in Hoshi 2
      */
    static XArray<XString> LexerReservedWords{
            L"func", L"var", L"as", L"return",
            L"class", L"extends", L"impl", L"interface", L"public", L"template", L"virtual",
            L"if", L"else", L"while", L"for", L"break", L"continue",
            L"use"
    };

    /**
      * @biref Exceptions of Lexer in Hoshi 2
      */
    class LexerException : HoshiException {
        std::string message;
    public:
        LexerException();

        LexerException(XIndexType Line, XIndexType Column, const XString &Reason);

        [[nodiscard]] const char *what() const noexcept override;
    };

    class Lexer {
    private:
        /**
         * Source codes.
         */
        XString String;
    public:
        /**
         * Kinds of tokens.
         */
        enum class TokenKind : int {
            Identifier,
            String,
            Character,
            Keywords,
            Integer,
            Decimal,
            Boolean,
            ToSign,                     // ->
            Plus,                       // +
            Minus,                      // -
            Asterisk,                   // *
            Slash,                      // /
            PercentSign,                // %
            BinaryNot,                  // ~
            BinaryXOR,                  // ^
            BinaryOr,                   // |
            BinaryAnd,                  // &
            IncrementSign,              // ++
            DecrementSign,              // --
            BinaryLeftMove,             // <<
            BinaryRightMove,            // >>
            AdditionAssignment,         // +=
            SubtractionAssignment,      // -=
            MultiplicationAssignment,   // *=
            DivisionAssignment,         // /=
            RemainderAssignment,        // %=
            MoreThan,                   // >
            LessThan,                   // <
            MoreThanOrEqual,            // >=
            LessThanOrEqual,            // <=
            Equal,                      // ==
            NotEqual,                   // !=
            LogicAnd,                   // &&
            LogicOr,                    // ||
            TypeCastingSign,            // =>
            AssignSign,                 // =
            LeftParentheses,            // (
            RightParentheses,           // )
            LeftBracket,                // [
            RightBracket,               // ]
            LeftBraces,                 // {
            RightBraces,                // }
            Semicolon,                  // ;
            Colon,                      // ,
            Dot,                        // .
            TypeDescriptorSign,         // :
            StaticMemberAccessSign,     // ::
            EoF,
        };

        /**
         * Token type
         */
        class Token {
        public:
            TokenKind Kind;
            XIndexType Line, Column;
            XString Value;

            Token();

            Token(TokenKind Kind, XString Value, XIndexType Line, XIndexType Column);

            bool operator==(const Token &rhs) const;

            bool operator!=(const Token &rhs) const;
        };

        /**
         * The last token has been got
         */
        Token LastToken;

        /**
         * @biref Default constructor of Lexer
         */
        Lexer();

        /**
         * @biref Initialize a Lexer with source codes.
         * @param String Source codes
         */
        explicit Lexer(XString String);

        XIndexType Line{}, Column{}, Position{};

        /**
         * @biref Get next character
         * @return Next character
         */
        XCharacter NextCharacter();

        /**
         * @biref Get next token
         * @return The next token
         */
        Token Scan();

        /**
         * @biref Reset Lexer to initial state.
         */
        void Reset();
    };

} // Hoshi

#endif //XSCRIPT2_LEXER_HPP
