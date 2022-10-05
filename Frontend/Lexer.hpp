//
// Created by Jerry Chou on 2022/5/7.
//

#ifndef XSCRIPT2_LEXER_HPP
#define XSCRIPT2_LEXER_HPP

#include <algorithm>

#include <Config.hpp>
#include <Utils.hpp>
#include <HoshiException.hpp>

namespace Hoshi {

    /**
      * @brief Exceptions of Lexer in Hoshi 2
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
            Invert,                  // ~
            BinaryXOR,                  // ^
            BinaryOr,                   // |
            BinaryAnd,                  // &
            IncrementSign,              // ++
            DecrementSign,              // --
            BinaryLeftShift,             // <<
            BinaryRightShift,            // >>
            AdditionAssignment,         // +=
            SubtractionAssignment,      // -=
            MultiplicationAssignment,   // *=
            DivisionAssignment,         // /=
            RemainderAssignment,        // %=
            GreaterThan,                   // >
            LessThan,                   // <
            GreaterThanOrEqual,            // >=
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
            Func,
            Var,
            As,
            Return,
            Struct,
            Impl,
            Interface,
            Public,
            Template,
            If,
            Else,
            While,
            For,
            Break,
            Continue,
            Use,
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
         * @brief Default constructor of Lexer
         */
        Lexer();

        /**
         * @brief Initialize a Lexer with source codes.
         * @param String Source codes
         */
        explicit Lexer(XString String);

        XIndexType Line{}, Column{}, Position{};

        /**
         * @brief Get next character
         * @return Next character
         */
        XCharacter NextCharacter();

        /**
         * @brief Get next token
         * @return The next token
         */
        Token Scan();

        /**
         * @brief Reset Lexer to initial state.
         */
        void Reset();

    private:
        /**
         * @brief Get next token as Identifier/Keyword/Boolean
         * @return The next token
         */
        Token ScanAsIdentifier();

        /**
         * @brief Get next token as Digital
         * @return The next token
         */
        Token ScanAsDigital();

        /**
         * @brief Get next token as ReadonlyStringLiteral
         * @return The next token
         */
        Token ScanAsReadonlyStringLiteral();

        /**
         * @brief Get next token as Character
         * @return The next token
         */
        Token ScanAsCharacter();

        /**
         * @brief Get next token as Single Sign
         * @return The next token
         */
        Token ScanAsSingleSign();

        /**
         * @brief Get next token as Multiple Sign
         * @return The next token
         */
        Token ScanAsMultipleSign();

        /**
         * @brief Get next token as Logic Sign
         * @return The next token
         */
        Token ScanAsLogicSign();
    };

} // Hoshi

#endif //XSCRIPT2_LEXER_HPP
