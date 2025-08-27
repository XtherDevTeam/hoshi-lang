//
// Created by XIaokang00010 on 2023/2/11.
//

#include "compiler/compilerContext.h"
#include "compiler/frontend/lexer.hpp"
#include "compiler/ir/IR.h"
#include "share/def.hpp" 
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-qualification"
#pragma ide diagnostic ignored "misc-no-recursion"

#include "parser.hpp"

namespace yoi {
    // Helper to finalize children in vectors on error
    template<typename T>
    void finalizeAST_vec(yoi::vec<T*>& vec) {
        for (auto& i : vec) {
            finalizeAST(i);
        }
        vec.clear(); // Clear the pointers from the vector
    }

    void parse(yoi::basicLiterals *&o, yoi::lexer &lex) {
        switch (lex.curToken.kind) {
            case lexer::token::tokenKind::integer:
            case lexer::token::tokenKind::decimal:
            case lexer::token::tokenKind::character:
            case lexer::token::tokenKind::string:
            case lexer::token::tokenKind::boolean:
            case lexer::token::tokenKind::kNull:
            case lexer::token::tokenKind::unsignedInt:
            case lexer::token::tokenKind::shortInt:
                o = new yoi::basicLiterals{lex.curToken, lex.curToken};
                lex.scan();
                break;
            default:
                o = nullptr;
                break;
        }
    }

    void parse(identifier *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::identifier) {
            o = new identifier{lex.curToken, lex.curToken};
            lex.scan();
        } else {
            o = nullptr;
        }
    }

    void parse(identifierWithTypeSpec *&o, lexer &lex) {
        lex.saveState();
        identifier *id = nullptr;
        typeSpec *spec = nullptr;
        
        lexer::token node_start_token = lex.curToken;

        parse(id, lex);
        if (!id) {
            lex.dropState(); // No identifier parsed, so no state to return. Drop it.
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan();
        } else {
            lex.returnState();
            finalizeAST(id);
            o = nullptr;
            return;
        }

        parse(spec, lex);
        if (!spec) { // If typeSpec parsing fails
            finalizeAST(id);
            lex.returnState(); // Backtrack because we couldn't complete the rule
            o = nullptr;
            return;
        }

        lex.dropState();
        o = new identifierWithTypeSpec{node_start_token, id, spec};
    }

    void parse(defTemplateArgSpec *&o, lexer &lex) {
        identifier *id = nullptr;
        externModuleAccessExpression *impl = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::kImpl) {
            lex.scan();
            parse(impl, lex);
            if (!impl) {
                finalizeAST(id);
                panic(lex.line, lex.col, "expected externModuleAccessExpression after `impl` in defTemplateArgSpec");
                o = nullptr; // Ensure o is null on failure
                return;
            }
            o = new defTemplateArgSpec{node_start_token, id, impl};
        } else {
            o = new defTemplateArgSpec{node_start_token, id, nullptr};
        }
    }

    void parse(defTemplateArg *&o, lexer &lex) {
        lex.saveState();
        if (lex.curToken.kind != lexer::token::tokenKind::lessThan) {
            lex.dropState();
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();

        yoi::vec<defTemplateArgSpec *> specs;
        defTemplateArgSpec *t = nullptr; // Initialize t

        parse(t, lex);
        if (t) { // Parse first argument if present
            specs.push_back(t);
            while (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
                t = nullptr; // Reset for next parse
                parse(t, lex);
                if (!t) { // Comma must be followed by an argument
                    finalizeAST_vec(specs);
                    lex.returnState();
                    panic(lex.line, lex.col, "expected defTemplateArgSpec after comma in defTemplateArg");
                    o = nullptr;
                    return;
                }
                specs.push_back(t);
            }
        }
        
        if (lex.curToken.kind == lexer::token::tokenKind::greaterThan) {
            lex.scan();
            o = new defTemplateArg{node_start_token, specs};
            lex.dropState();
        } else {
            finalizeAST_vec(specs);
            lex.returnState();
            panic(lex.line, lex.col, "expected `>` to close a defTemplateArg node"); // More specific panic message
            o = nullptr;
            return;
        }
    }

    void parse(templateArgSpec *&o, lexer &lex) {
        typeSpec *spec = nullptr;
        lexer::token node_start_token = lex.curToken;
        parse(spec, lex);
        o = spec ? new templateArgSpec{node_start_token, spec} : nullptr;
    }

    void parse(templateArg *&o, lexer &lex) {
        lex.saveState();
        if (lex.curToken.kind != lexer::token::tokenKind::lessThan) {
            lex.dropState();
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();

        yoi::vec<templateArgSpec *> specs;
        templateArgSpec *t = nullptr; // Initialize t

        parse(t, lex);
        if (t) { // Parse first argument if present
            specs.push_back(t);
            while (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
                t = nullptr; // Reset for next parse
                parse(t, lex);
                if (!t) { // Comma must be followed by an argument
                    finalizeAST_vec(specs);
                    lex.returnState();
                    panic(lex.line, lex.col, "expected templateArgSpec after comma in templateArg");
                    o = nullptr;
                    return;
                }
                specs.push_back(t);
            }
        }

        if (lex.curToken.kind == lexer::token::tokenKind::greaterThan) {
            lex.scan();
            o = new templateArg{node_start_token, specs};
            lex.dropState();
        } else {
            lex.returnState();
            finalizeAST_vec(specs);
            o = nullptr;
            return;
        }
    }

    void parse(invocationArguments *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();

        yoi::vec<rExpr *> args;
        rExpr *t = nullptr; // Initialize t

        parse(t, lex);
        if (t) { // Parse first argument if present
            args.emplace_back(t);
            while (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
                t = nullptr; // Reset for next parse
                parse(t, lex);
                if (!t) { // Comma must be followed by an argument
                    finalizeAST_vec(args);
                    panic(lex.line, lex.col, "expected rightValueExpr after comma in invocationArguments");
                    o = nullptr;
                    return;
                }
                args.emplace_back(t);
            }
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
            o = new invocationArguments{node_start_token, args};
        } else {
            finalizeAST_vec(args);
            panic(lex.line, lex.col, "expected `)` to close an invocationArguments node");
            o = nullptr;
            return;
        }
    }

    void parse(definitionArguments *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();

        yoi::vec<identifierWithTypeSpec *> args;
        identifierWithTypeSpec *t = nullptr; // Initialize t

        parse(t, lex);
        if (t) { // Parse first argument if present
            args.push_back(t);
            while (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
                t = nullptr; // Reset for next parse
                parse(t, lex);
                if (!t) { // Comma must be followed by an argument
                    finalizeAST_vec(args);
                    panic(lex.line, lex.col, "expected identifierWithTypeSpec after comma in definitionArguments");
                    o = nullptr;
                    return;
                }
                args.push_back(t);
            }
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
            o = new definitionArguments{node_start_token, args};
        } else {
            finalizeAST_vec(args);
            panic(lex.line, lex.col, "expected `)` to close a definitionArguments node");
            o = nullptr;
            return;
        }
    }

    void parse(funcTypeSpec *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kFunc) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        unnamedDefinitionArguments *args = nullptr;
        typeSpec *spec = nullptr;

        parse(args, lex);
        if (!args) {
            panic(lex.line, lex.col, "expected definitionArguments after `func`");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon)
            lex.scan();
        else {
            finalizeAST(args);
            panic(lex.line, lex.col, "expected `:` after definitionArguments");
            o = nullptr;
            return;
        }
        parse(spec, lex);
        if (!spec) {
            finalizeAST(args);
            panic(lex.line, lex.col, "expected typeSpec after `:`");
            o = nullptr;
            return;
        }
        o = new funcTypeSpec{node_start_token, args, spec};
    }

    void parse(typeSpec *&o, lexer &lex) {
        externModuleAccessExpression *expr = nullptr;
        funcTypeSpec *spec = nullptr;
        lexer::token node_start_token = lex.curToken;

        if (lex.curToken.kind == lexer::token::tokenKind::kNull) {
            lex.scan();
            o = new typeSpec{node_start_token, 2, nullptr, nullptr, true};
            return;
        } else if (lex.curToken.kind == lexer::token::tokenKind::kThreeDots) {
            lex.scan();
            o = new typeSpec{node_start_token, 3, nullptr, nullptr, false, false};
            return;
        }
        parse(spec, lex);
        if (spec) {
            if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
                lex.scan();
                if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                    lex.scan();
                    o = new typeSpec{node_start_token, 1, nullptr, spec, false, true};
                    return;
                } else {
                    panic(lex.line, lex.col, "expected `]` to close a array type specifier node");
                    finalizeAST(spec);
                    o = nullptr;
                    return;
                }
            }
            o = new typeSpec{node_start_token, 1, nullptr, spec, false, true};
            return;
        }
        parse(expr, lex);
        if (expr) {
            if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
                lex.scan();
                if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                    lex.scan();
                    o = new typeSpec{node_start_token, 0, expr, nullptr, false, true};
                    return;
                } else {
                    panic(lex.line, lex.col, "expected `]` to close a array type specifier node");
                    finalizeAST(expr);
                    o = nullptr;
                    return;
                }
            }
            o = new typeSpec{node_start_token, 0, expr, nullptr, false};
            return;
        }
        o = nullptr;
    }

    void parse(subscript *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            rExpr *r = nullptr;
            parse(r, lex);
            if (!r) {
                panic(lex.line, lex.col, "expected rightValueExpr in subscript");
                o = nullptr;
                return;
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                lex.scan();
                o = new subscript{node_start_token, r};
            } else {
                finalizeAST(r);
                panic(lex.line, lex.col, "expected `]` to close a subscript");
                o = nullptr;
                return;
            }
        } else if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lexer::token node_start_token = lex.curToken;
            invocationArguments *args = nullptr;
            parse(args, lex);
            if (!args) {
                panic(lex.line, lex.col, "expected invocationArguments in subscript");
                o = nullptr;
                return;
            }
            o = new subscript{node_start_token, nullptr, args};
        } else {
            o = nullptr;
        }
    }

    void parse(identifierWithTemplateArg *&o, lexer &lex) {
        identifier *id = nullptr;
        templateArg *arg = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }
        o = new identifierWithTemplateArg{node_start_token, id, nullptr};

        parse(arg, lex);
        if (!arg) {
            // No template arg, 'o' is already created with 'nullptr' for arg.
            return;
        }
        o->arg = arg;
    }

    void parse(identifierWithDefTemplateArg *&o, lexer &lex) {
        identifier *id = nullptr;
        defTemplateArg *arg = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }
        o = new identifierWithDefTemplateArg{node_start_token, id, nullptr};

        parse(arg, lex);
        if (!arg) {
            // No def template arg, 'o' is already created with 'nullptr' for arg.
            return;
        }
        o->arg = arg;
    }

    void parse(externModuleAccessExpression *&o, lexer &lex) {
        yoi::vec<identifierWithTemplateArg *> vecA;
        identifierWithTemplateArg *a = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (!a) {
            o = nullptr;
            return;
        }

        while (true) {
            vecA.push_back(a); // Add current 'a' to vector
            if (lex.curToken.kind != lexer::token::tokenKind::dot) {
                break; // No more dots, end of expression
            } else if (a->hasTemplateArg()) { // Logic: an identifier with template arg cannot be followed by a dot.
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected identifier (except the last term) in externModuleAccessExpression, found identifier with template arguments followed by '.'");
                o = nullptr;
                return;
            } else {
                lex.scan(); // Consume the dot
            }
            a = nullptr; // Reset 'a' for the next parse call
            parse(a, lex);
            if (!a) { // A dot must be followed by another identifier.
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected identifier after `.` in externModuleAccessExpression");
                o = nullptr;
                return;
            }
        }
        o = new externModuleAccessExpression{node_start_token, vecA};
    }

    void parse(subscriptExpr *&o, lexer &lex) {
        identifierWithTemplateArg *a = nullptr;
        vec<subscript *> b;
        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (!a) {
            o = nullptr;
            return;
        }
        
        subscript *s = nullptr;
        for (parse(s, lex); s; parse(s, lex)) {
            b.push_back(s);
        }

        o = new subscriptExpr{node_start_token, a, b};
    }

    void parse(memberExpr *&o, lexer &lex) {
        yoi::vec<subscriptExpr *> vecA;
        subscriptExpr *a = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (!a) {
            o = nullptr;
            return;
        }
        while (true) {
            vecA.push_back(a);
            if (lex.curToken.kind != lexer::token::tokenKind::dot) {
                break;
            } else {
                lex.scan();
            }
            a = nullptr; // Reset for next parse
            parse(a, lex);
            if (!a) { // Dot must be followed by another subscriptExpr
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected subscriptExpr after `.` in memberExpr");
                o = nullptr;
                return;
            }
        }
        o = new memberExpr{node_start_token, vecA};
    }

    void parse(newExpression *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kNew) {
            o = nullptr;
            return;
        }
        lex.scan();

        externModuleAccessExpression *expr = nullptr;
        parse(expr, lex);
        if (!expr) {
            panic(lex.line, lex.col, "expected externModuleAccessExpression after `new` in newExpression");
        }

        o = new newExpression{node_start_token, expr, nullptr, nullptr};
        parse(o->length, lex);
        if (!o->length) {
            panic(lex.line, lex.col, "expected lengthExpr after `new` in newExpression");
            finalizeAST(o->type);
            delete o;
            o = nullptr;
        }

        parse(o->args, lex);
        if (!o->args) {
            panic(lex.line, lex.col, "expected invocationArguments after `new` in newExpression");
            finalizeAST(o->type);
            finalizeAST(o->length);
            delete o;
            o = nullptr;
        }
    }

    void parse(primary *&o, lexer &lex) {
        memberExpr *a = nullptr;
        basicLiterals *b = nullptr;
        typeIdExpression *d = nullptr;
        dynCastExpression *e = nullptr;
        newExpression *f = nullptr;
        lambdaExpr *g = nullptr;
        callableExpression *h = nullptr;
        rExpr *c = nullptr;

        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (a) {
            o = new primary{node_start_token, 0, a, nullptr, nullptr, nullptr, nullptr, nullptr};
            return;
        }
        parse(b, lex);
        if (b) {
            o = new primary{node_start_token, 1, nullptr, b, nullptr, nullptr, nullptr, nullptr};
            return;
        }
        parse(d, lex);
        if (d) {
            o = new primary{node_start_token, 3, nullptr, nullptr, nullptr, d, nullptr, nullptr, nullptr};
            return;
        }
        parse(e, lex);
        if (e) {
            o = new primary{node_start_token, 4, nullptr, nullptr, nullptr, nullptr, e, nullptr, nullptr, nullptr};
            return;
        }
        parse(f, lex);
        if (f) {
            o = new primary{node_start_token, 5, nullptr, nullptr, nullptr, nullptr, nullptr, f, nullptr, nullptr};
            return;
        }
        parse(g, lex);
        if (g) {
            o = new primary{node_start_token, 6, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, g, nullptr};
            return;
        }
        parse(h, lex);
        if (h) {
            o = new primary{node_start_token, 7, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, h};
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan(); // Consume '('
            parse(c, lex);
            if (!c) {
                panic(lex.line, lex.col, "expected rightValueExpr after `(` while parsing primary");
                o = nullptr; // Ensure o is null on failure
                return;
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
                lex.scan(); // Consume ')'
                o = new primary{node_start_token, 2, nullptr, nullptr, c};
                return;
            } else {
                finalizeAST(c);
                panic(lex.line, lex.col, "expected `)` after rightValueExpr while parsing primary");
                o = nullptr; // Ensure o is null on failure
                return;
            }
        }
        o = nullptr;
    }

    void parse(uniqueExpr *&o, lexer &lex) {
        lex.saveState();
        lexer::token t{};
        switch (lex.curToken.kind) {
            case lexer::token::tokenKind::incrementSign:
            case lexer::token::tokenKind::decrementSign:
            case lexer::token::tokenKind::minus:
            case lexer::token::tokenKind::binaryNot: {
                t = lex.curToken;
                lex.scan(); // Consume the operator
                break;
            }
            default: {
                t.kind = lexer::token::tokenKind::unknown;
                break;
            }
        }
        abstractExpr *expr = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(expr, lex);
        if (!expr) {
            lex.returnState();
            o = nullptr;
            return;
        }
        o = new uniqueExpr{node_start_token, t, expr};
        lex.dropState();
    }

    void parse(leftExpr *&o, lexer &lex) {
        uniqueExpr *a = nullptr;
        lexer::token t{};
        rExpr *expr = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (!a) {
            o = nullptr;
            return;
        }

        switch (lex.curToken.kind) {
            case lexer::token::tokenKind::assignSign:
            case lexer::token::tokenKind::additionAssignment:
            case lexer::token::tokenKind::subtractionAssignment:
            case lexer::token::tokenKind::multiplicationAssignment:
            case lexer::token::tokenKind::divisionAssignment: 
            case lexer::token::tokenKind::directAssignSign: {
                t = lex.curToken;
                lex.scan();
                parse(expr, lex);
                if (!expr) {
                    finalizeAST(a);
                    panic(lex.line, lex.col, "expected rightValueExpr after assignment operator");
                    o = nullptr;
                    return;
                }
                o = new leftExpr{node_start_token, t, a, expr};
                break;
            }
            default: {
                o = new leftExpr{node_start_token, t, a, nullptr}; // 't' will be an empty token, 'expr' will be nullptr.
                break;
            }
        }
    }

    // Common pattern for binary expressions (mulExpr, addExpr, shiftExpr, etc.)
    // Applied to all binary expression parsers below.
    #define PARSE_BINARY_EXPR(NODE_TYPE, CHILD_TYPE, OPERATORS, ERROR_MSG) \
    void parse(NODE_TYPE *&o, lexer &lex) { \
        yoi::vec<CHILD_TYPE *> vecA; \
        yoi::vec<lexer::token> vecB; \
        CHILD_TYPE *a = nullptr; \
        lexer::token node_start_token = lex.curToken; \
\
        parse(a, lex); \
        if (a) { \
            vecA.push_back(a); \
            while OPERATORS { \
                vecB.push_back(lex.curToken); \
                lex.scan(); \
                a = nullptr; \
                parse(a, lex); \
                if (!a) { \
                    finalizeAST_vec(vecA); \
                    panic(lex.line, lex.col, ERROR_MSG); \
                    o = nullptr; \
                    return; \
                } \
                vecA.push_back(a); \
            } \
            o = new NODE_TYPE{node_start_token, vecA, vecB}; \
        } else { \
            o = nullptr; \
        } \
    }

    PARSE_BINARY_EXPR(mulExpr, leftExpr, (lex.curToken.kind == lexer::token::tokenKind::asterisk || lex.curToken.kind == lexer::token::tokenKind::slash || lex.curToken.kind == lexer::token::tokenKind::percentSign), "expected uniqueExpr after operators while parsing mulExpr")
    PARSE_BINARY_EXPR(addExpr, mulExpr, (lex.curToken.kind == lexer::token::tokenKind::plus || lex.curToken.kind == lexer::token::tokenKind::minus), "expected mulExpr after operators while parsing addExpr")
    PARSE_BINARY_EXPR(shiftExpr, addExpr, (lex.curToken.kind == lexer::token::tokenKind::binaryShiftLeft || lex.curToken.kind == lexer::token::tokenKind::binaryShiftRight), "expected addExpr after operators while parsing shiftExpr")
    PARSE_BINARY_EXPR(relationalExpr, shiftExpr, (lex.curToken.kind == lexer::token::tokenKind::lessThan || lex.curToken.kind == lexer::token::tokenKind::greaterThan || lex.curToken.kind == lexer::token::tokenKind::lessEqual || lex.curToken.kind == lexer::token::tokenKind::greaterEqual), "expected shiftExpr after operators while parsing relationalExpr")
    PARSE_BINARY_EXPR(equalityExpr, relationalExpr, (lex.curToken.kind == lexer::token::tokenKind::equal || lex.curToken.kind == lexer::token::tokenKind::notEqual), "expected relationalExpr after operators while parsing equalityExpr")
    PARSE_BINARY_EXPR(andExpr, equalityExpr, (lex.curToken.kind == lexer::token::tokenKind::binaryAnd), "expected equalityExpr after operators while parsing andExpr") // Corrected from relationalExpr
    PARSE_BINARY_EXPR(exclusiveExpr, andExpr, (lex.curToken.kind == lexer::token::tokenKind::binaryXor), "expected andExpr after operators while parsing exclusiveExpr")
    PARSE_BINARY_EXPR(inclusiveExpr, exclusiveExpr, (lex.curToken.kind == lexer::token::tokenKind::binaryOr), "expected exclusiveExpr after operators while parsing inclusiveExpr")
    PARSE_BINARY_EXPR(logicalAndExpr, inclusiveExpr, (lex.curToken.kind == lexer::token::tokenKind::logicAnd), "expected inclusiveExpr after operators while parsing logicalAndExpr")
    PARSE_BINARY_EXPR(logicalOrExpr, logicalAndExpr, (lex.curToken.kind == lexer::token::tokenKind::logicOr), "expected logicalAndExpr after operators while parsing logicalOrExpr")

    #undef PARSE_BINARY_EXPR

    void parse(rExpr *&o, lexer &lex) {
        logicalOrExpr *expr = nullptr;
        lexer::token node_start_token = lex.curToken;
        parse(expr, lex);
        if (expr) {
            o = new rExpr{node_start_token, expr};
        } else {
            o = nullptr;
        }
    }

    void parse(codeBlock *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            yoi::vec<inCodeBlockStmt *> stmts;
            inCodeBlockStmt *stmt = nullptr; // Initialize stmt

            while (true) {
                parse(stmt, lex);
                if (!stmt)
                    break;
                stmts.push_back(stmt);
                stmt = nullptr; // Reset stmt for the next parse call
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
                lex.scan();
                o = new codeBlock{node_start_token, stmts};
            } else {
                finalizeAST_vec(stmts);
                panic(lex.line, lex.col, "expected `}` to close codeBlock");
                o = nullptr;
                return;
            }
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(useStmt *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kUse) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        identifier *id = nullptr;
        lexer::token str_token{}; // Not a pointer, directly stored

        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected identifier after `use`");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind != lexer::token::tokenKind::string) {
            finalizeAST(id);
            panic(lex.line, lex.col, "expected string token after identifier while parsing useStmt");
            o = nullptr;
            return;
        }
        str_token = lex.curToken; // Store the string token by value
        lex.scan();
        o = new useStmt{node_start_token, id, str_token};
    }

    void parse(funcDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kFunc) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        yoi::vec<lexer::token> attrs;
        identifierWithDefTemplateArg *name = nullptr;
        definitionArguments *args = nullptr;
        typeSpec *spec = nullptr;
        codeBlock *block = nullptr;

        while (lex.curToken.kind >= lexer::token::tokenKind::kNoFFI && lex.curToken.kind <= lexer::token::tokenKind::kAlwaysInline) {
            attrs.push_back(lex.curToken);
            lex.scan();
        }

        parse(name, lex);
        if (!name) {
            panic(lex.line, lex.col, "expected function name");
            o = nullptr;
            return;
        }
        parse(args, lex);
        if (!args) {
            finalizeAST(name);
            panic(lex.line, lex.col, "expected definitionArguments after identifierWithDefTemplateArg");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon)
            lex.scan();
        else {
            finalizeAST(name);
            finalizeAST(args);
            panic(lex.line, lex.col, "expected `:` after definitionArguments");
            o = nullptr;
            return;
        }
        parse(spec, lex);
        if (!spec) {
            finalizeAST(name);
            finalizeAST(args);
            panic(lex.line, lex.col, "expected typeSpec after `:`");
            o = nullptr;
            return;
        }
        parse(block, lex);
        if (!block) {
            finalizeAST(name);
            finalizeAST(args);
            finalizeAST(spec);
            panic(lex.line, lex.col, "expected codeBlock after typeSpec");
            o = nullptr;
            return;
        }
        o = new funcDefStmt{node_start_token, attrs, name, args, spec, block};
    }

    void parse(interfaceDefInnerPair *&o, lexer &lex) {
        identifierWithTypeSpec *var = nullptr;
        innerMethodDecl *method = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(method, lex);
        if (method) {
            o = new interfaceDefInnerPair{node_start_token, nullptr, method};
            return;
        }
        parse(var, lex);
        if (var) {
            o = new interfaceDefInnerPair{node_start_token, var, nullptr};
            return;
        }
        o = nullptr;
    }

    void parse(structDefInnerPair *&o, lexer &lex) {
        identifierWithTypeSpec *var = nullptr;
        innerMethodDecl *method = nullptr;
        constructorDecl *con = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(con, lex);
        if (con) {
            o = new structDefInnerPair{node_start_token, 1, nullptr, con, nullptr};
            return;
        }
        parse(method, lex);
        if (method) {
            o = new structDefInnerPair{node_start_token, 2, nullptr, nullptr, method};
            return;
        }
        parse(var, lex);
        if (var) {
            o = new structDefInnerPair{node_start_token, 0, var, nullptr, nullptr};
            return;
        }
        o = nullptr;
    }

    void parse(implInnerPair *&o, lexer &lex) {
        innerMethodDef *method = nullptr;
        constructorDef *con = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(con, lex);
        if (con) {
            o = new implInnerPair{node_start_token, con, nullptr};
            return;
        }
        parse(method, lex);
        if (method) {
            o = new implInnerPair{node_start_token, nullptr, method};
            return;
        }
        o = nullptr;
    }

    void parse(interfaceDefInner *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            yoi::vec<interfaceDefInnerPair *> vecA;
            interfaceDefInnerPair *a = nullptr; // Initialize a

            while (true) {
                parse(a, lex);
                if (!a) {
                    break;
                }
                vecA.push_back(a);
                a = nullptr; // Reset 'a' for next parse

                if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                    lex.scan();
                } else {
                    break;
                }
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
                lex.scan();
                o = new interfaceDefInner{node_start_token, vecA};
            } else {
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected `}` to close interfaceDefInner");
                o = nullptr;
                return;
            }
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(structDefInner *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            yoi::vec<structDefInnerPair *> vecA;
            structDefInnerPair *a = nullptr; // Initialize a

            while (true) {
                parse(a, lex);
                if (!a) {
                    break;
                }
                vecA.push_back(a);
                a = nullptr; // Reset 'a' for next parse

                if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                    lex.scan();
                } else {
                    break;
                }
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
                lex.scan();
                o = new structDefInner{node_start_token, vecA};
            } else {
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected `}` to close structDefInner");
                o = nullptr;
                return;
            }
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(implInner *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            yoi::vec<implInnerPair *> vecA;
            implInnerPair *a = nullptr; // Initialize a

            while (true) {
                parse(a, lex);
                if (!a) {
                    break;
                }
                vecA.push_back(a);
                a = nullptr; // Reset 'a' for next parse

                if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                    lex.scan();
                } else {
                    break;
                }
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
                lex.scan();
                o = new implInner{node_start_token, vecA};
            } else {
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected `}` to close implInner");
                o = nullptr;
                return;
            }
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(interfaceDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kInterface) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        identifierWithDefTemplateArg *id = nullptr;
        interfaceDefInner *inner = nullptr;

        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected interface name after `interface`");
            o = nullptr;
            return;
        }
        parse(inner, lex);
        if (!inner) {
            finalizeAST(id);
            panic(lex.line, lex.col, "expected interfaceDefInner after identifier");
            o = nullptr;
            return;
        }
        o = new interfaceDefStmt{node_start_token, id, inner};
    }

    void parse(structDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kStruct) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        identifierWithDefTemplateArg *id = nullptr;
        structDefInner *inner = nullptr;

        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected struct name after `struct`");
            o = nullptr;
            return;
        }
        parse(inner, lex);
        if (!inner) {
            finalizeAST(id);
            panic(lex.line, lex.col, "expected structDefInner after identifier");
            o = nullptr;
            return;
        }
        o = new structDefStmt{node_start_token, id, inner};
    }

    void parse(implStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kImpl) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        externModuleAccessExpression *first = nullptr; // Represents the interface (optional)
        externModuleAccessExpression *second = nullptr;   // Represents the struct
        implInner *inner = nullptr;

        // Save state for potential backtracking related to the optional ':' interface
        lex.saveState();

        parse(second, lex); // Try to parse the struct name first
        if (!second) {
            lex.dropState(); // Drop the saved state as we failed at the very beginning of the rule
            panic(lex.line, lex.col, "expected struct name after `impl`");
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan(); // Consume the colon
            parse(first, lex); // Now try to parse the interface name
            if (!first) {
                finalizeAST(second);
                lex.returnState(); // Backtrack because we tried to parse an interface but failed
                panic(lex.line, lex.col, "expected interface name after `:`");
                o = nullptr;
                return;
            }
        } else {
            // No colon means no interface specified, 'first' remains nullptr.
            // No returnState() here, as this is a successful path for the "impl struct" variant.
        }

        // Now parse the inner block
        parse(inner, lex);
        if (!inner) {
           
            if (first) finalizeAST(first);
            finalizeAST(second);
            // Drop the state from the beginning as parsing failed to complete the 'impl' rule
            lex.dropState();
            panic(lex.line, lex.col, "expected implInner after interface or struct name");
            o = nullptr;
            return;
        }

        lex.dropState(); // Drop state on success.
        o = new implStmt{node_start_token, first, second, inner};
    }

    void parse(letAssignmentPair *&o, lexer &lex) {
        identifier *lhs = nullptr;
        rExpr *rhs = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(lhs, lex);
        if (!lhs) {
            panic(lex.line, lex.col, "expected left-hand-side in letAssignmentPair");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::assignSign) {
            lex.scan();
        } else {
            finalizeAST(lhs);
            panic(lex.line, lex.col, "expected `=` after left-hand-side in letAssignmentPair");
            o = nullptr;
            return;
        }
        parse(rhs, lex);
        if (!rhs) {
            finalizeAST(lhs);
            panic(lex.line, lex.col, "expected right-hand-side in letAssignmentPair");
            o = nullptr;
            return;
        }
        o = new letAssignmentPair{node_start_token, lhs, rhs};
    }

    void parse(letStmt *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kLet) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        yoi::vec<letAssignmentPair *> vecA;
        letAssignmentPair *a = nullptr; // Initialize a

        while (true) {
            parse(a, lex);
            if (!a) { // If 'a' could not be parsed
                if (vecA.empty()) { // If it's the first assignment and it failed
                    panic(lex.line, lex.col, "expected letAssignmentPair after `let`");
                } else { // If it's a subsequent assignment after a comma and it failed
                    panic(lex.line, lex.col, "expected letAssignmentPair after comma in let statement");
                }
                finalizeAST_vec(vecA);
                o = nullptr;
                return;
            }
            vecA.push_back(a);
            a = nullptr; // Reset 'a' for the next parse

            if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
            } else {
                break;
            }
        }
        o = new letStmt{node_start_token, vecA};
    }

    void parse(globalStmt *&o, lexer &lex) {
        // Initialize all pointers to nullptr to avoid uninitialized checks
        useStmt *a = nullptr;
        interfaceDefStmt *b = nullptr;
        structDefStmt *c = nullptr;
        implStmt *d = nullptr;
        letStmt *e = nullptr;
        funcDefStmt *f = nullptr;
        exportDecl *g = nullptr;
        importDecl *h = nullptr;
        
        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (a) {
            o = new globalStmt{node_start_token, globalStmt::vKind::useStmt, {a}};
            return;
        }

        parse(b, lex);
        if (b) {
            o = new globalStmt{node_start_token, globalStmt::vKind::interfaceDefStmt, {b}};
            return;
        }

        parse(c, lex);
        if (c) {
            o = new globalStmt{node_start_token, globalStmt::vKind::structDefStmt, {c}};
            return;
        }

        parse(d, lex);
        if (d) {
            o = new globalStmt{node_start_token, globalStmt::vKind::implStmt, {d}};
            return;
        }

        parse(e, lex);
        if (e) {
            o = new globalStmt{node_start_token, globalStmt::vKind::letStmt, {e}};
            return;
        }

        parse(f, lex);
        if (f) {
            o = new globalStmt{node_start_token, globalStmt::vKind::funcDefStmt, {f}};
            return;
        }

        parse(g, lex);
        if (g) {
            o = new globalStmt{node_start_token, globalStmt::vKind::exportDecl, {g}};
            return;
        }

        parse(h, lex);
        if (h) {
            o = new globalStmt{node_start_token, globalStmt::vKind::importDecl, {h}};
            return;
        }
        o = nullptr; // No global statement matched
    }

    // ifBlock parse for ifStmt
    void parse(ifStmt::ifBlock &o, lexer &lex) {
        // Initialize members to nullptr to be safe in case of early return
        o.cond = nullptr;
        o.block = nullptr;

        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(`");
            return;
        }

        parse(o.cond, lex);
        if (!o.cond) {
            panic(lex.line, lex.col, "expected rExpr after `(`");
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            finalizeAST(o.cond);
            panic(lex.line, lex.col, "expected `)` after rExpr");
            return;
        }

        parse(o.block, lex);
        if (!o.block) {
            finalizeAST(o.cond);
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            return;
        }
    }

    void parse(ifStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kIf) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            o = new ifStmt{node_start_token, {}, {}, nullptr}; // Create ifStmt node early for cleanup
        } else {
            o = nullptr;
            return;
        }

        // Parse the initial ifBlock
        ifStmt::ifBlock temp_if_block{}; // Use a stack variable for parsing
        parse(temp_if_block, lex);
        if (!temp_if_block.cond || !temp_if_block.block) { // Check if parsing failed (parse(ifBlock) panics and returns)
            finalizeAST(o);
            o = nullptr;
            return;
        }
        o->ifB = temp_if_block; // Assign the struct by value (copying the pointers)

        // Parse elif blocks
        while (lex.curToken.kind == lexer::token::tokenKind::kElif) {
            lex.scan();
            temp_if_block = {}; // Reset stack variable for the next elif block
            parse(temp_if_block, lex);
            if (!temp_if_block.cond || !temp_if_block.block) { // If elif block parsing failed
                finalizeAST(o);
                panic(lex.line, lex.col, "expected ifBlock after `elif`");
                o = nullptr;
                return;
            }
            o->elifB.push_back(temp_if_block); // Push a *copy* of the struct
        }

        // Parse else block
        if (lex.curToken.kind == lexer::token::tokenKind::kElse) {
            lex.scan();
            parse(o->elseB, lex);
            if (!o->elseB) {
                finalizeAST(o);
                panic(lex.line, lex.col, "expected codeBlock after `else`");
                o = nullptr;
                return;
            }
        }
    }

    void parse(whileStmt *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kWhile) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }

        rExpr *expr = nullptr;
        codeBlock *block = nullptr;

        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(` after `while`");
            o = nullptr;
            return;
        }

        parse(expr, lex);
        if (!expr) {
            panic(lex.line, lex.col, "expected rExpr after `(`");
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            finalizeAST(expr);
            panic(lex.line, lex.col, "expected `)` after rExpr");
            o = nullptr;
            return;
        }

        parse(block, lex);
        if (!block) {
            finalizeAST(expr);
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            o = nullptr;
            return;
        }

        o = new whileStmt{node_start_token, expr, block};
    }

    void parse(forStmt *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kFor) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }

        inCodeBlockStmt *initStmt = nullptr;
        rExpr *cond = nullptr;
        inCodeBlockStmt *afterStmt = nullptr;
        codeBlock *block = nullptr;

        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(` after `for`");
            o = nullptr;
            return;
        }
        
        parse(initStmt, lex);
        // Original code implies initStmt can be empty. "expected initStmt" panic removed if this is allowed.
        
        // Reconciling with original panic: it seems intended to be mandatory.
        if (!initStmt) {
            panic(lex.line, lex.col, "expected initStmt after `(`");
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::semicolon) {
            lex.scan();
        } else {
            finalizeAST(initStmt);
            panic(lex.line, lex.col, "expected `;` after initStmt");
            o = nullptr;
            return;
        }
        
        parse(cond, lex);
        
        if (!cond) {
            finalizeAST(initStmt);
            panic(lex.line, lex.col, "expected condition after `;`");
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::semicolon) {
            lex.scan();
        } else {
            finalizeAST(initStmt);
            finalizeAST(cond);
            panic(lex.line, lex.col, "expected `;` after condition");
            o = nullptr;
            return;
        }
        
        parse(afterStmt, lex);
        if (!afterStmt) {
            finalizeAST(initStmt);
            finalizeAST(cond);
            panic(lex.line, lex.col, "expected afterStmt after `;`");
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            finalizeAST(initStmt);
            finalizeAST(cond);
            finalizeAST(afterStmt);
            panic(lex.line, lex.col, "expected `)` after afterStmt");
            o = nullptr;
            return;
        }
        parse(block, lex);
        if (!block) {
            finalizeAST(initStmt);
            finalizeAST(cond);
            finalizeAST(afterStmt);
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            o = nullptr;
            return;
        }
        o = new forStmt{node_start_token, initStmt, cond, afterStmt, block};
    }

    void parse(forEachStmt *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kForEach) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        // Initialize children to nullptr
        identifier *var = nullptr;
        rExpr *container = nullptr;
        codeBlock *block = nullptr;

        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(` after `forEach`");
            o = nullptr;
            return;
        }
        parse(var, lex);
        if (!var) {
            panic(lex.line, lex.col, "expected variable name after `(`");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan();
        } else {
            finalizeAST(var);
            panic(lex.line, lex.col, "expected `:` after variable name");
            o = nullptr;
            return;
        }
        parse(container, lex);
        if (!container) {
            finalizeAST(var);
            panic(lex.line, lex.col, "expected container after `:`");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            finalizeAST(var);
            finalizeAST(container);
            panic(lex.line, lex.col, "expected `)` after container");
            o = nullptr;
            return;
        }
        parse(block, lex);
        if (!block) {
            finalizeAST(var);
            finalizeAST(container);
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            o = nullptr;
            return;
        }
        o = new forEachStmt{node_start_token, var, container, block};
    }

    void parse(returnStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kReturn) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            rExpr *expr = nullptr;
            parse(expr, lex); // expr is optional
            o = new returnStmt{node_start_token, expr};
        } else {
            o = nullptr;
        }
    }

    void parse(continueStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kContinue) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            o = new continueStmt{node_start_token}; // Add token for consistency
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(breakStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kBreak) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            o = new breakStmt{node_start_token}; // Add token for consistency
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(inCodeBlockStmt *&o, lexer &lex) {
        // Initialize all potential children to nullptr before trying to parse
        letStmt *letStmtVal = nullptr;
        ifStmt *ifStmtVal = nullptr;
        breakStmt *breakStmtVal = nullptr;
        continueStmt *continueStmtVal = nullptr;
        returnStmt *returnStmtVal = nullptr;
        forEachStmt *forEachStmtVal = nullptr; 
        whileStmt *whileStmtVal = nullptr;
        forStmt *forStmtVal = nullptr;
        codeBlock *codeBlockVal = nullptr;
        tryCatchStmt *tryCatchStmtVal = nullptr;
        throwStmt *throwStmtVal = nullptr;
        rExpr *rExprVal = nullptr;
        
        lexer::token node_start_token = lex.curToken;

        // Try parsing each type, and if successful, create the inCodeBlockStmt and return.
        // This avoids creating the inCodeBlockStmt node until a successful child parse.

        parse(letStmtVal, lex);
        if (letStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::letStmt, {letStmtVal}};
            return;
        }
        parse(ifStmtVal, lex);
        if (ifStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::ifStmt, {ifStmtVal}};
            return;
        }
        parse(breakStmtVal, lex);
        if (breakStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::breakStmt, {breakStmtVal}};
            return;
        }
        parse(continueStmtVal, lex);
        if (continueStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::continueStmt, {continueStmtVal}};
            return;
        }
        parse(returnStmtVal, lex);
        if (returnStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::returnStmt, {returnStmtVal}};
            return;
        }
        parse(forEachStmtVal, lex); // Only parse once
        if (forEachStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::forEachStmt, {forEachStmtVal}};
            return;
        }
        parse(whileStmtVal, lex);
        if (whileStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::whileStmt, {whileStmtVal}};
            return;
        }

        parse(forStmtVal, lex);
        if (forStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::forStmt, {forStmtVal}};
            return;
        }

        parse(tryCatchStmtVal, lex);
        if (tryCatchStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::tryCatchStmt, {tryCatchStmtVal}};
            return;
        }

        parse(throwStmtVal, lex);
        if (throwStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::throwStmt, {throwStmtVal}};
            return;
        }
        
        parse(codeBlockVal, lex);
        if (codeBlockVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::codeBlock, {codeBlockVal}};
            return;
        }

        // rExpr should typically be last, as it's the most general expression statement.
        parse(rExprVal, lex);
        if (rExprVal) { 
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::rExpr, {rExprVal}};
            return;
        }

        o = nullptr; // If no statement type matched
    }

    void parse(innerMethodDecl *&o, lexer &lex) {
        lex.saveState();
        lexer::token node_start_token = lex.curToken;

        o = new innerMethodDecl{node_start_token, {}, nullptr, nullptr, nullptr};

        while (lex.curToken.kind >= lexer::token::tokenKind::kNoFFI && lex.curToken.kind <= lexer::token::tokenKind::kAlwaysInline) {
            o->attrs.push_back(lex.curToken);
            lex.scan();
        }

        parse(o->name, lex);
        if (!o->name) {
            lex.returnState();
            delete o; // Delete the partially constructed node
            o = nullptr;
            return;
        }
        parse(o->args, lex);
        if (!o->args) {
            finalizeAST(o->name);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan();
        } else {
            finalizeAST(o->name);
            finalizeAST(o->args);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        parse(o->resultType, lex);
        if (!o->resultType) {
            finalizeAST(o->name);
            finalizeAST(o->args);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        lex.dropState();
    }

    void parse(innerMethodDef *&o, lexer &lex) {
        lex.saveState();
        lexer::token node_start_token = lex.curToken;

        o = new innerMethodDef{node_start_token, {}, nullptr, nullptr, nullptr, nullptr};

        while (lex.curToken.kind >= lexer::token::tokenKind::kNoFFI && lex.curToken.kind <= lexer::token::tokenKind::kAlwaysInline) {
            o->attrs.push_back(lex.curToken);
            lex.scan();
        }

        parse(o->name, lex);
        if (!o->name) {
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        parse(o->args, lex);
        if (!o->args) {
            finalizeAST(o->name);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan();
        } else {
            finalizeAST(o->name);
            finalizeAST(o->args);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        parse(o->resultType, lex);
        if (!o->resultType) {
            finalizeAST(o->name);
            finalizeAST(o->args);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        parse(o->block, lex);
        if (!o->block) {
            finalizeAST(o->name);
            finalizeAST(o->args);
            finalizeAST(o->resultType);
            lex.returnState();
            delete o;
            o = nullptr;
            return;
        }
        lex.dropState();
    }

    void parse(constructorDecl *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kConstructor) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        definitionArguments *args = nullptr;
        parse(args, lex);
        if (!args) {
            panic(lex.line, lex.col, "expected arguments after `constructor`");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            finalizeAST(args);
            panic(lex.line, lex.col, "constructor declaration cannot have a return type");
            o = nullptr;
        }
        o = new constructorDecl{node_start_token, args};
    }

    void parse(constructorDef *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kConstructor) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        definitionArguments *args = nullptr;
        codeBlock *block = nullptr;

        parse(args, lex);
        if (!args) {
            panic(lex.line, lex.col, "expected arguments after `constructor`");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            finalizeAST(args);
            panic(lex.line, lex.col, "constructor declaration cannot have a return type");
            o = nullptr;
        }
        parse(block, lex);
        if (!block) {
            finalizeAST(args);
            panic(lex.line, lex.col, "expected codeBlock after arguments");
            o = nullptr;
            return;
        }
        o = new constructorDef{node_start_token, args, block};
    }

    void parse(hoshiModule *&o, lexer &lex) {
        yoi::vec<globalStmt *> vecA;
        globalStmt *a = nullptr; // Initialize a
        
        lexer::token node_start_token = lex.curToken;

        while (true) {
            if (lex.curToken.kind == lexer::token::tokenKind::eof)
                break;
            parse(a, lex);
            if (!a) { // If parsing a global statement fails, it's an error.
                finalizeAST_vec(vecA);
                panic(lex.line, lex.col, "expected globalStmt");
                o = nullptr;
                return;
            }
            vecA.push_back(a);
            a = nullptr; // Reset 'a' for the next parse call
        }
        o = new hoshiModule{node_start_token, vecA};
    }

    void parse(importDecl *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kImport) {
            lexer::token node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr; // Consistent with other functions
            return;
        }

        innerMethodDecl *a = nullptr; 
        parse(a, lex);
        if (!a) {
            panic(lex.line, lex.col, "expected innerMethodDecl after `import`"); // Corrected panic message
            o = nullptr;
            return;
        }

        o = new importDecl{node_start_token, a}; // Create the node here, now that 'a' is successfully parsed

        if (lex.curToken.kind == lexer::token::tokenKind::kFrom) {
            lex.scan(); // Consume 'from'
            if (lex.curToken.kind == lexer::token::tokenKind::string) {
                o->from_path = lex.curToken;
                lex.scan(); // Consume string
            } else {
                finalizeAST(o);
                panic(lex.line, lex.col, "expected string literal after `from` in import declaration");
                o = nullptr;
                return;
            }
        } else {
            finalizeAST(o);
            panic(lex.line, lex.col, "expected `from` after import declaration");
            o = nullptr;
            return;
        }
    }

    void parse(exportDecl *&o, lexer &lex) {
        lexer::token node_start_token;
        if (lex.curToken.kind == lexer::token::tokenKind::kExport) {
            node_start_token = lex.curToken;
            lex.scan();
        } else {
            o = nullptr; // Consistent
            return;
        }

        yoi::vec<lexer::token> attrs;
        while(lex.curToken.kind >= lexer::token::tokenKind::kNoFFI && lex.curToken.kind <= lexer::token::tokenKind::kAlwaysInline) {
            attrs.push_back(lex.curToken);
            lex.scan();
        }

        typeSpec *a = nullptr; 
        parse(a, lex);
        if (!a) {
            panic(lex.line, lex.col, "expected typeSpec after `export`"); // Corrected panic message
            o = nullptr;
            return;
        }

        o = new exportDecl{node_start_token, attrs, a, nullptr}; // Create the node here, now that 'a' is parsed.

        if (lex.curToken.kind == lexer::token::tokenKind::kAs) {
            lex.scan(); // Consume 'as'
            identifier *b = nullptr;
            parse(b, lex);
            if (!b) {
                finalizeAST(o);
                panic(lex.line, lex.col, "expected identifier after `as` in export declaration");
                o = nullptr;
                return;
            }
            o->as = b;
        } else {
            finalizeAST(o);
            panic(lex.line, lex.col, "expected `as` after export declaration");
            o = nullptr;
            return;
        }
    }

    void parse(tryCatchStmt *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kTry) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        codeBlock *tryBlock = nullptr;
        parse(tryBlock, lex);
        yoi_assert(tryBlock, lex.line, lex.col, "expected codeBlock after `try`");
        yoi::vec<catchParam *> catchParams;
        while (lex.curToken.kind == lexer::token::tokenKind::kCatch) {
            catchParam *param;
            parse(param, lex);
            if (!param) {
                finalizeAST(tryBlock);
                o = nullptr;
                panic(lex.line, lex.col, "expected catchParam after `catch`");
            }
            catchParams.push_back(param);
        }
        codeBlock *finallyBlock = nullptr;
        parse(finallyBlock, lex);
        if (!finallyBlock) {
            finalizeAST(tryBlock);
            for (auto p : catchParams) finalizeAST(p);
            o = nullptr;
            panic(lex.line, lex.col, "expected codeBlock after `catch`");
        }
        o = new tryCatchStmt{node_start_token, tryBlock, catchParams, finallyBlock};
    }

    void parse(throwStmt *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kThrow) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        rExpr *expr = nullptr;
        parse(expr, lex);
        if (!expr) {
            o = nullptr;
            panic(lex.line, lex.col, "expected expression after `throw`");
        }
        o = new throwStmt{node_start_token, expr};
    }

    void parse(catchParam *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kCatch) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `(` after `catch`");
        }
        lex.scan();
        identifier *name = nullptr;
        parse(name, lex);
        if (!name) {
            o = nullptr;
            panic(lex.line, lex.col, "expected identifier after typeSpec in catchParam");
        }
        if (lex.curToken.kind != lexer::token::tokenKind::colon) {
            o = nullptr;
            finalizeAST(name);
            panic(lex.line, lex.col, "expected `)` after identifier in catchParam");
        }
        lex.scan();
        typeSpec *type = nullptr;
        parse(type, lex);
        if (!type) {
            o = nullptr;
            finalizeAST(name);
            panic(lex.line, lex.col, "expected typeSpec after `(` in catchParam");
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            finalizeAST(name);
            finalizeAST(type);
            panic(lex.line, lex.col, "expected `)` after typeSpec in catchParam");
        }
        lex.scan();
        codeBlock *block = nullptr;
        parse(block, lex);
        if (!block) {
            o = nullptr;
            finalizeAST(type);
            finalizeAST(name);
            panic(lex.line, lex.col, "expected codeBlock after identifier in catchParam");
        }
        o = new catchParam{node_start_token, type, name, block};
    }

    void parse(typeIdExpression *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kTypeId) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
            rExpr *expr = nullptr;
            parse(expr, lex);
            if (!expr) {
                o = nullptr;
                panic(lex.line, lex.col, "expected expression after `(` in `type_id` expression");
            }
            if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
                o = nullptr;
                finalizeAST(expr);
                panic(lex.line, lex.col, "expected `)` after expression in `type_id` expression");
            }
            lex.scan();
            o = new typeIdExpression{node_start_token, nullptr, expr};
        } else if (lex.curToken.kind == lexer::token::tokenKind::lessThan) {
            lex.scan();
            typeSpec *type = nullptr;
            parse(type, lex);
            if (!type) {
                o = nullptr;
                panic(lex.line, lex.col, "expected typeSpec after `<` in `type_id` expression");
            }
            if (lex.curToken.kind != lexer::token::tokenKind::greaterThan) {
                o = nullptr;
                finalizeAST(type);
                panic(lex.line, lex.col, "expected `>` after typeSpec in `type_id` expression");
            }
            lex.scan();
            o = new typeIdExpression{node_start_token, type, nullptr};
        } else {
            o = nullptr;
            panic(lex.line, lex.col, "expected `(` or `<` after `type_id` in `type_id` expression");
        }
    }

    void parse(dynCastExpression *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kDynCast) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::lessThan) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `<` after `dyn_cast`");
        }
        lex.scan();
        typeSpec *type = nullptr;
        parse(type, lex);
        if (!type) {
            o = nullptr;
            panic(lex.line, lex.col, "expected typeSpec after `<` in `dyn_cast` expression");
        }
        if (lex.curToken.kind != lexer::token::tokenKind::greaterThan) {
            o = nullptr;
            finalizeAST(type);
            panic(lex.line, lex.col, "expected `>` after typeSpec in `dyn_cast` expression");
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            finalizeAST(type);
            panic(lex.line, lex.col, "expected `(` after `>` in `dyn_cast` expression");
        }
        lex.scan();
        rExpr *expr = nullptr;
        parse(expr, lex);
        if (!expr) {
            o = nullptr;
            finalizeAST(type);
            panic(lex.line, lex.col, "expected expression after `(` in `dyn_cast` expression");
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            finalizeAST(type);
            finalizeAST(expr);
            panic(lex.line, lex.col, "expected `)` after expression in `dyn_cast` expression");
        }
        lex.scan();
        o = new dynCastExpression{node_start_token, type, expr};
    }

    void parse(abstractExpr *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        primary *lhs{};
        parse(lhs, lex);
        if (!lhs) {
            o = nullptr;
            return;
        }
        if (lex.curToken.kind != lexer::token::tokenKind::kInterfaceOf && lex.curToken.kind != lexer::token::tokenKind::kImpl && lex.curToken.kind != lexer::token::tokenKind::kAs) {
            o = new abstractExpr{node_start_token, lhs, {}, nullptr};
            return;
        }
        o = new abstractExpr{node_start_token, lhs, lex.curToken, nullptr};
        lex.scan();

        parse(o->rhs, lex);
        if (!o->rhs) {
            finalizeAST(o);
            o = nullptr;
            panic(lex.line, lex.col, "expected extern module access expression after `interfaceof` or `impl`");
        }
    }

    void parse(lambdaExpr *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kFunc) {
            o = nullptr;
            return;
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftBracket) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `[` after `func` in lambda expression");
        }
        lex.scan();
        vec<yoi::identifier *> captures;
        while (lex.curToken.kind == lexer::token::tokenKind::identifier) {
            identifier *capture = nullptr;
            parse(capture, lex);
            captures.push_back(capture);
            if (!capture) {
                for (auto c : captures) finalizeAST(c);
                o = nullptr;
                panic(lex.line, lex.col, "expected identifier in capture list in lambda expression");
            }
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBracket) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `]` after capture list in lambda expression");
        }
        lex.scan();
        definitionArguments *args = nullptr;
        parse(args, lex);
        if (!args) {
            o = nullptr;
            panic(lex.line, lex.col, "expected arguments after capture list in lambda expression");
        }
        if (lex.curToken.kind != lexer::token::tokenKind::colon) {
            o = nullptr;
            finalizeAST(args);
            panic(lex.line, lex.col, "expected `:` after arguments in lambda expression");
        }
        lex.scan();
        typeSpec *resultType = nullptr;
        parse(resultType, lex);
        if (!resultType) {
            o = nullptr;
            finalizeAST(args);
            panic(lex.line, lex.col, "expected typeSpec after `:` in lambda expression");
        }
        codeBlock *block = nullptr;
        parse(block, lex);
        if (!block) {
            o = nullptr;
            finalizeAST(args);
            finalizeAST(resultType);
            panic(lex.line, lex.col, "expected codeBlock after `:` in lambda expression");
        }
        o = new lambdaExpr{node_start_token, captures, args, resultType, block};
    }

    void parse(unnamedDefinitionArguments *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        vec<typeSpec *> types;
        while (true) {
            typeSpec *type = nullptr;
            parse(type, lex);
            if (!type) {
                o = nullptr;
                panic(lex.line, lex.col, "expected typeSpec in unnamed definition arguments");
            }
            types.push_back(type);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            for (auto t : types) finalizeAST(t);
            panic(lex.line, lex.col, "expected `)` after typeSpecs in unnamed definition arguments");
        }
        lex.scan();
        o = new unnamedDefinitionArguments{node_start_token, types};
    }

    void parse(callableExpression *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kCallable) {
            o = nullptr;
            return;
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            return;
        }
        lex.scan();
        rExpr *expr = nullptr;
        parse(expr, lex);
        if (!expr) {
            o = nullptr;
            panic(lex.line, lex.col, "expected expression in callable expression");
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            finalizeAST(expr);
            panic(lex.line, lex.col, "expected `)` after expression in callable expression");
        }
        lex.scan();
        o = new callableExpression{node_start_token, expr};
    }
} // namespace yoi

#pragma clang diagnostic pop