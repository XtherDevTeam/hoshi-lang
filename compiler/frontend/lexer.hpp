//
// Created by XIaokang00010 on 2023/1/24.
//

#ifndef HOSHI_LANG_LEXER_HPP
#define HOSHI_LANG_LEXER_HPP

#include <cstdint>
#include <istream>
#include <share/def.hpp>

namespace yoi {

    class lexer {
        std::wstringstream stream;
    public:
        struct Comment {
            uint64_t line, col;
            wstr text;
            bool isMultiLine;
        };

        vec<Comment> comments;

        struct token {
            uint64_t line, col;
            enum class tokenKind {
                unknown = 0,
                identifier,
                character,
                string,
                integer,
                unsignedInt,
                shortInt,
                decimal,
                boolean,
                toSign,
                plus,
                minus,
                asterisk,
                slash,
                percentSign,
                binaryXor,
                binaryOr,
                binaryAnd,
                binaryNot,
                logicNot,
                incrementSign,
                decrementSign,
                binaryShiftLeft,
                binaryShiftRight,
                additionAssignment,
                subtractionAssignment,
                multiplicationAssignment,
                divisionAssignment,
                reminderAssignment,
                greaterThan,
                lessThan,
                greaterEqual,
                lessEqual,
                equal,
                notEqual,
                logicAnd,
                logicOr,
                assignSign,
                directAssignSign, // :=
                leftParentheses,
                rightParentheses,
                leftBracket,
                rightBracket,
                leftBraces,
                rightBraces,
                semicolon,  // ;
                colon,      // :
                comma,      // ,
                dot,        // .
                sharp,      // #
                kUse,
                kFunc,
                kInterface,
                kConstructor,
                kFinalizer,
                kStruct,
                kImpl,
                kLet,
                kIn,
                kFor,
                kForEach,
                kWhile,
                kIf,
                kElif,
                kElse,
                kReturn,
                kContinue,
                kBreak,
                kCast,
                kNull,
                kImport,
                kExport,
                kAs,
                kFrom,
                kTry,
                kCatch,
                kFinally,
                kThrow,
                kTypeId,
                kDynCast,
                kNoFFI,
                kStatic,
                kIntrinsic,
                kGenerator,
                kAlwaysInline,
                kNew,
                kCallable,
                kThreeDots,
                kInterfaceOf,
                kAlias,
                kEnum,
                kDataStruct,
                kDataField,
                kYield,
                kDecltype,
                eof,
            } kind;

            union vBasicValue {
                int64_t vInt;
                double vDeci;
                bool vBool;
                uint64_t vUint;
                int16_t vShort;

                vBasicValue(int64_t v);

                vBasicValue(double v);

                vBasicValue(bool v);

                vBasicValue(uint64_t v);

                vBasicValue(int16_t v);

                vBasicValue();
            } basicVal;

            wstr strVal;

            token();

            token(int64_t line, int64_t col, tokenKind kind);

            token(int64_t line, int64_t col, tokenKind kind, vBasicValue basicVal);

            token(int64_t line, int64_t col, tokenKind kind, wstr strVal);
        };

        struct lexerState {
            int64_t line, col;
            std::istream::pos_type pos;
            wchar curCh;
            token curToken;

            lexerState();

            lexerState(int64_t line, int64_t col, std::istream::pos_type pos, wchar curCh, lexer::token curToken);
        };

        vec<lexerState> states;

        token curToken;
        wchar curCh;

        int64_t line, col;

        void getCh();

        explicit lexer(std::wstringstream ss);

        void saveState();

        void returnState();

        void dropState();

        token scan();

        token alphaStart();

        token operatorStart();

        token strStart();

        token digitStart();

        token minusStart();

        token plusStart();

        token asteriskStart();

        token slashStart();

        token percentSignStart();

        token equalStart();

        token notStart();

        token lessStart();

        token greaterStart();

        token semicolonStart();

        token colonStart();

        token commaStart();

        token dotStart();

        token sharpStart();

        token leftParenthesesStart();

        token rightParenthesesStart();

        token leftBracketStart();

        token rightBracketStart();

        token leftBracesStart();

        token rightBracesStart();

        token andStart();

        token orStart();

        token xorStart();

        token binaryNotStart();
    };

} // rex

#endif //HOSHI_LANG_LEXER_HPP
