//
// Created by XIaokang00010 on 2023/2/11.
//

#include "compiler/compilerContext.h"
#include "compiler/frontend/lexer.hpp"
#include "compiler/ir/IR.h"
#include "share/def.hpp"
#include <cstdint>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-qualification"
#pragma ide diagnostic ignored "misc-no-recursion"

#include "parser.hpp"

namespace yoi {
    // Helper to finalize children in vectors on error
    template <typename T> void finalizeAST_vec(yoi::vec<T *> &vec) {
        for (auto &i : vec) {
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
        satisfyClause *satisfyCondition = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::kSatisfy) {
            parse(satisfyCondition, lex);
            if (!satisfyCondition) {
                finalizeAST(id);
                panic(lex.line, lex.col, "expected satisfyClause after `satisfy` in defTemplateArgSpec");
                o = nullptr; // Ensure o is null on failure
                return;
            }
            o = new defTemplateArgSpec{node_start_token, id, satisfyCondition};
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
        decltypeExpr *decltypeExpression = nullptr;
        lexer::token node_start_token = lex.curToken;

        if (lex.curToken.kind == lexer::token::tokenKind::kNull) {
            lex.scan();
            o = new typeSpec{node_start_token, typeSpec::typeSpecKind::Null, nullptr, nullptr, nullptr, nullptr, true};
            return;
        } else if (lex.curToken.kind == lexer::token::tokenKind::kThreeDots) {
            lex.scan();
            typeSpec *t = nullptr;
            parse(t, lex);
            if (!t) {
                o = new typeSpec{node_start_token, typeSpec::typeSpecKind::Elipsis, nullptr, nullptr, nullptr, nullptr, false, nullptr};
            } else {
                if (t->kind == typeSpec::typeSpecKind::Elipsis) {
                    finalizeAST(t);
                    o = nullptr;
                    panic(lex.line, lex.col, "expected typeSpec after `...`");
                }
                o = new typeSpec{node_start_token, typeSpec::typeSpecKind::Elipsis, nullptr, nullptr, t, nullptr, false, nullptr};
            }
            return;
        }
        typeSpec::typeSpecKind kind;

        parse(spec, lex);
        if (spec) {
            kind = typeSpec::typeSpecKind::Func;
        } else {
            parse(expr, lex);
            if (expr) {
                kind = typeSpec::typeSpecKind::Member;
            } else {
                parse(decltypeExpression, lex);
                if (decltypeExpression) {
                    kind = typeSpec::typeSpecKind::DecltypeExpr;
                } else {
                    o = nullptr;
                    return;
                }
            }
        }

        yoi::vec<uint64_t> *arraySubscript = nullptr;
        if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
            lex.scan();
            if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                lex.scan();
                arraySubscript = new vec<uint64_t>{(uint64_t)-1};
            } else if (lex.curToken.kind == lexer::token::tokenKind::integer) {
                arraySubscript = new yoi::vec<uint64_t>{lex.curToken.basicVal.vUint};
                while (lex.scan().kind == lexer::token::tokenKind::comma) {
                    lex.scan();
                    if (lex.curToken.kind == lexer::token::tokenKind::integer) {
                        arraySubscript->push_back(lex.curToken.basicVal.vInt);
                    } else {
                        if (spec) finalizeAST(spec);
                        if (expr) finalizeAST(expr);
                        if (decltypeExpression) finalizeAST(decltypeExpression);
                        delete arraySubscript;
                        o = nullptr;
                        panic(lex.line, lex.col, "expected integer after `,` in array type specifier");
                        return;
                    }
                }
                if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                    lex.scan();
                } else {
                    if (spec) finalizeAST(spec);
                    if (expr) finalizeAST(expr);
                    if (decltypeExpression) finalizeAST(decltypeExpression);
                    delete arraySubscript;
                    o = nullptr;
                    return;
                }
            } else {
                if (spec) finalizeAST(spec);
                if (expr) finalizeAST(expr);
                if (decltypeExpression) finalizeAST(decltypeExpression);
                o = nullptr;
                return;
            }
        }

        o = new typeSpec{node_start_token, kind, expr, spec, nullptr, decltypeExpression, false, arraySubscript};
    }

    void parse(subscript *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
            lex.saveState();
            lexer::token node_start_token = lex.curToken;
            lex.scan();
            rExpr *r = nullptr;
            parse(r, lex);
            if (!r) {
                // panic(lex.line, lex.col, "expected rightValueExpr in subscript");
                lex.returnState();
                o = nullptr;
                return;
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightBracket) {
                lex.dropState();
                lex.scan();
                o = new subscript{node_start_token, r};
            } else {
                lex.dropState();
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
                break;                        // No more dots, end of expression
            } else if (a->hasTemplateArg()) { // Logic: an identifier with template arg cannot be followed by a dot.
                finalizeAST_vec(vecA);
                panic(lex.line,
                      lex.col,
                      "expected identifier (except the last term) in externModuleAccessExpression, found identifier with template arguments followed "
                      "by '.'");
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
            if (lex.curToken.kind == lexer::token::tokenKind::leftBracket && lex.scan().kind == lexer::token::tokenKind::rightBracket) {
                o->length = nullptr;
                lex.scan(); // Consume ']'
            } else {
                panic(lex.line, lex.col, "expected lengthExpr after `new` in newExpression");
                finalizeAST(o->type);
                delete o;
                o = nullptr;
            }
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
        funcExpr *h = nullptr;
        bracedInitalizerList *i = nullptr;
        rExpr *c = nullptr;

        lexer::token node_start_token = lex.curToken;

        parse(a, lex);
        if (a) {
            o = new primary{node_start_token, primary::primaryKind::memberExpr, a, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
            return;
        }
        parse(b, lex);
        if (b) {
            o = new primary{node_start_token, primary::primaryKind::basicLiterals, nullptr, b, nullptr, nullptr, nullptr, nullptr, nullptr};
            return;
        }
        parse(d, lex);
        if (d) {
            o = new primary{
                node_start_token, primary::primaryKind::typeIdExpression, nullptr, nullptr, nullptr, d, nullptr, nullptr, nullptr, nullptr};
            return;
        }
        parse(e, lex);
        if (e) {
            o = new primary{
                node_start_token, primary::primaryKind::dynCastExpression, nullptr, nullptr, nullptr, nullptr, e, nullptr, nullptr, nullptr, nullptr};
            return;
        }
        parse(f, lex);
        if (f) {
            o = new primary{
                node_start_token, primary::primaryKind::newExpression, nullptr, nullptr, nullptr, nullptr, nullptr, f, nullptr, nullptr, nullptr};
            return;
        }
        parse(g, lex);
        if (g) {
            o = new primary{
                node_start_token, primary::primaryKind::lambdaExpr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, g, nullptr, nullptr};
            return;
        }
        parse(h, lex);
        if (h) {
            o = new primary{
                node_start_token, primary::primaryKind::funcExpr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, h, nullptr};
            return;
        }
        parse(i, lex);
        if (i) {
            o = new primary{node_start_token,
                            primary::primaryKind::bracedInitalizerList,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            nullptr,
                            i};
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
                o = new primary{node_start_token, primary::primaryKind::rExpr, nullptr, nullptr, c};
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
#define PARSE_BINARY_EXPR(NODE_TYPE, CHILD_TYPE, OPERATORS, ERROR_MSG)                                                                               \
    void parse(NODE_TYPE *&o, lexer &lex) {                                                                                                          \
        yoi::vec<CHILD_TYPE *> vecA;                                                                                                                 \
        yoi::vec<lexer::token> vecB;                                                                                                                 \
        CHILD_TYPE *a = nullptr;                                                                                                                     \
        lexer::token node_start_token = lex.curToken;                                                                                                \
                                                                                                                                                     \
        parse(a, lex);                                                                                                                               \
        if (a) {                                                                                                                                     \
            vecA.push_back(a);                                                                                                                       \
            while                                                                                                                                    \
                OPERATORS {                                                                                                                          \
                    vecB.push_back(lex.curToken);                                                                                                    \
                    lex.scan();                                                                                                                      \
                    a = nullptr;                                                                                                                     \
                    parse(a, lex);                                                                                                                   \
                    if (!a) {                                                                                                                        \
                        finalizeAST_vec(vecA);                                                                                                       \
                        panic(lex.line, lex.col, ERROR_MSG);                                                                                         \
                        o = nullptr;                                                                                                                 \
                        return;                                                                                                                      \
                    }                                                                                                                                \
                    vecA.push_back(a);                                                                                                               \
                }                                                                                                                                    \
            o = new NODE_TYPE{node_start_token, vecA, vecB};                                                                                         \
        } else {                                                                                                                                     \
            o = nullptr;                                                                                                                             \
        }                                                                                                                                            \
    }

    PARSE_BINARY_EXPR(mulExpr,
                      leftExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::asterisk || lex.curToken.kind == lexer::token::tokenKind::slash ||
                       lex.curToken.kind == lexer::token::tokenKind::percentSign),
                      "expected uniqueExpr after operators while parsing mulExpr")
    PARSE_BINARY_EXPR(addExpr,
                      mulExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::plus || lex.curToken.kind == lexer::token::tokenKind::minus),
                      "expected mulExpr after operators while parsing addExpr")
    PARSE_BINARY_EXPR(shiftExpr,
                      addExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::binaryShiftLeft ||
                       lex.curToken.kind == lexer::token::tokenKind::binaryShiftRight),
                      "expected addExpr after operators while parsing shiftExpr")
    PARSE_BINARY_EXPR(relationalExpr,
                      shiftExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::lessThan || lex.curToken.kind == lexer::token::tokenKind::greaterThan ||
                       lex.curToken.kind == lexer::token::tokenKind::lessEqual || lex.curToken.kind == lexer::token::tokenKind::greaterEqual),
                      "expected shiftExpr after operators while parsing relationalExpr")
    PARSE_BINARY_EXPR(equalityExpr,
                      relationalExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::equal || lex.curToken.kind == lexer::token::tokenKind::notEqual),
                      "expected relationalExpr after operators while parsing equalityExpr")
    PARSE_BINARY_EXPR(andExpr,
                      equalityExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::binaryAnd),
                      "expected equalityExpr after operators while parsing andExpr") // Corrected from relationalExpr
    PARSE_BINARY_EXPR(exclusiveExpr,
                      andExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::binaryXor),
                      "expected andExpr after operators while parsing exclusiveExpr")
    PARSE_BINARY_EXPR(inclusiveExpr,
                      exclusiveExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::binaryOr),
                      "expected exclusiveExpr after operators while parsing inclusiveExpr")
    PARSE_BINARY_EXPR(logicalAndExpr,
                      inclusiveExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::logicAnd),
                      "expected inclusiveExpr after operators while parsing logicalAndExpr")
    PARSE_BINARY_EXPR(logicalOrExpr,
                      logicalAndExpr,
                      (lex.curToken.kind == lexer::token::tokenKind::logicOr),
                      "expected logicalAndExpr after operators while parsing logicalOrExpr")

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
                yoi::indexT endLine = lex.curToken.line;
                yoi::indexT endColumn = lex.curToken.col;
                lex.scan();
                o = new codeBlock{node_start_token, stmts};
                o->endLine = endLine;
                o->endColumn = endColumn;
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
        finalizerDecl *fin = nullptr;
        lexer::token node_start_token = lex.curToken;
        structDefInnerPair::Modifier mod{structDefInnerPair::Modifier::None};

        if (lex.curToken.kind == lexer::token::tokenKind::kWeak) {
            mod = structDefInnerPair::Modifier::Weak;
            lex.scan();
        } else if (lex.curToken.kind == lexer::token::tokenKind::kDataField) {
            mod = structDefInnerPair::Modifier::DataField;
            lex.scan();
        }

        parse(con, lex);
        if (con) {
            o = new structDefInnerPair{node_start_token, 1, mod, nullptr, con, nullptr, nullptr};
            return;
        }
        parse(method, lex);
        if (method) {
            o = new structDefInnerPair{node_start_token, 2, mod, nullptr, nullptr, method, nullptr};
            return;
        }
        parse(fin, lex);
        if (fin) {
            o = new structDefInnerPair{node_start_token, 3, mod, nullptr, nullptr, nullptr, fin};
            return;
        }
        parse(var, lex);
        if (var) {
            o = new structDefInnerPair{node_start_token, 0, mod, var, nullptr, nullptr, nullptr};
            return;
        }
        o = nullptr;
    }

    void parse(implInnerPair *&o, lexer &lex) {
        innerMethodDef *method = nullptr;
        constructorDef *con = nullptr;
        finalizerDef *fin = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(con, lex);
        if (con) {
            o = new implInnerPair{node_start_token, con, nullptr, nullptr};
            return;
        }
        parse(method, lex);
        if (method) {
            o = new implInnerPair{node_start_token, nullptr, method, nullptr};
            return;
        }
        parse(fin, lex);
        if (fin) {
            o = new implInnerPair{node_start_token, nullptr, nullptr, fin};
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

    void parse(dataStructDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kDataStruct) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        identifier *id = nullptr;
        structDefInner *inner = nullptr;

        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected datastruct name after `datastruct`");
            o = nullptr;
            return;
        }
        parse(inner, lex);
        if (!inner) {
            finalizeAST(id);
            panic(lex.line, lex.col, "expected datastruct body after identifier");
            o = nullptr;
            return;
        }

        for (auto &pair : inner->getInner()) {
            if (pair->kind != 0) {
                finalizeAST(id);
                finalizeAST(inner);
                panic(lex.line, lex.col, "datastruct can only contain fields, no methods or constructors allowed");
                o = nullptr;
                return;
            }
        }

        o = new dataStructDefStmt{node_start_token, id, inner};
    }

    void parse(implStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kImpl) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;

        externModuleAccessExpression *first = nullptr;  // Represents the interface (optional)
        externModuleAccessExpression *second = nullptr; // Represents the struct
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
            lex.scan();        // Consume the colon
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
            o = new implStmt{node_start_token, first, second, nullptr};
            return;
        }

        lex.dropState(); // Drop state on success.
        o = new implStmt{node_start_token, first, second, inner};
    }

    void parse(letAssignmentPair *&o, lexer &lex) {
        letAssignmentPairLHS *lhs = nullptr;
        typeSpec *type = nullptr;
        rExpr *rhs = nullptr;
        lexer::token node_start_token = lex.curToken;

        parse(lhs, lex);
        if (!lhs) {
            panic(lex.line, lex.col, "expected left-hand-side in letAssignmentPair");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan();
            parse(type, lex);
            if (!type) {
                panic(lex.line, lex.col, "expected typeSpec after `:` in letAssignmentPair");
                o = nullptr;
                return;
            }
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
        o = new letAssignmentPair{node_start_token, lhs, type, rhs};
    }

    void parse(letAssignmentPairLHS *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind == lexer::token::tokenKind::identifier) {
            identifier *id = new identifier{lex.curToken, lex.curToken};
            lex.scan();
            o = new letAssignmentPairLHS{node_start_token, letAssignmentPairLHS::vKind::identifier, id};
        } else if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
            lex.scan();
            yoi::vec<lexer::token> vecA;
            while (true) {
                if (lex.curToken.kind != lexer::token::tokenKind::identifier && lex.curToken.kind != lexer::token::tokenKind::kThreeDots) {
                    break;
                }
                vecA.push_back(lex.curToken);
                lex.scan();
                if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                    lex.scan();
                } else {
                    break;
                }
            }
            if (lex.curToken.kind == lexer::token::tokenKind::kThreeDots) {
                vecA.push_back(lex.curToken);
                lex.scan();
            }
            yoi_assert(vecA.empty() || vecA.size() < 2 ||
                           (vecA.front().kind != lexer::token::tokenKind::kThreeDots || vecA.back().kind != lexer::token::tokenKind::kThreeDots),
                       lex.line,
                       lex.col,
                       "structured binding cannot have `...` both in the front and in the back of the list");
            if (vecA.size() > 2)
                for (yoi::indexT i = 1; i < vecA.size() - 1; i++)
                    yoi_assert(vecA[i].kind != lexer::token::tokenKind::kThreeDots,
                               lex.line,
                               lex.col,
                               "structured binding cannot have `...` in the middle of the list");

            yoi_assert(lex.curToken.kind == lexer::token::tokenKind::rightBracket, lex.curToken.line, lex.curToken.col, "expected `]`");
            lex.scan();
            o = new letAssignmentPairLHS{node_start_token, letAssignmentPairLHS::vKind::list, nullptr, vecA};
        } else {
            panic(lex.line, lex.col, "expected identifier or `[...]` after `let`");
            o = nullptr;
        }
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
            if (!a) {               // If 'a' could not be parsed
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
        marcoDescriptor *marco = nullptr;
        useStmt *a = nullptr;
        interfaceDefStmt *b = nullptr;
        structDefStmt *c = nullptr;
        implStmt *d = nullptr;
        letStmt *e = nullptr;
        funcDefStmt *f = nullptr;
        exportDecl *g = nullptr;
        importDecl *h = nullptr;
        typeAliasStmt *i = nullptr;
        enumerationDefinition *j = nullptr;
        conceptDefinition *k = nullptr;

        lexer::token node_start_token = lex.curToken;

        parse(marco, lex);

        parse(a, lex);
        if (a) {
            o = new globalStmt{node_start_token, globalStmt::vKind::useStmt, marco, {a}};
            return;
        }

        parse(b, lex);
        if (b) {
            o = new globalStmt{node_start_token, globalStmt::vKind::interfaceDefStmt, marco, {b}};
            return;
        }

        parse(c, lex);
        if (c) {
            o = new globalStmt{node_start_token, globalStmt::vKind::structDefStmt, marco, {c}};
            return;
        }

        dataStructDefStmt *ds = nullptr;
        parse(ds, lex);
        if (ds) {
            o = new globalStmt{node_start_token, globalStmt::vKind::dataStructDefStmt, marco, {ds}};
            return;
        }

        parse(d, lex);
        if (d) {
            o = new globalStmt{node_start_token, globalStmt::vKind::implStmt, marco, {d}};
            return;
        }

        parse(e, lex);
        if (e) {
            o = new globalStmt{node_start_token, globalStmt::vKind::letStmt, marco, {e}};
            return;
        }

        parse(f, lex);
        if (f) {
            o = new globalStmt{node_start_token, globalStmt::vKind::funcDefStmt, marco, {f}};
            return;
        }

        parse(g, lex);
        if (g) {
            o = new globalStmt{node_start_token, globalStmt::vKind::exportDecl, marco, {g}};
            return;
        }

        parse(h, lex);
        if (h) {
            o = new globalStmt{node_start_token, globalStmt::vKind::importDecl, marco, {h}};
            return;
        }

        parse(i, lex);
        if (i) {
            o = new globalStmt{node_start_token, globalStmt::vKind::typeAliasStmt, marco, {i}};
            return;
        }

        parse(j, lex);
        if (j) {
            o = new globalStmt{node_start_token, globalStmt::vKind::enumerationDef, marco, {j}};
            return;
        }

        parse(k, lex);
        if (k) {
            o = new globalStmt{node_start_token, globalStmt::vKind::conceptDef, marco, {k}};
            return;
        }

        if (marco)
            finalizeAST(marco);
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
        marcoDescriptor *marco = nullptr;
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
        yieldStmt *yieldStmtVal = nullptr;
        rExpr *rExprVal = nullptr;

        lexer::token node_start_token = lex.curToken;

        parse(marco, lex);

        // Try parsing each type, and if successful, create the inCodeBlockStmt and return.
        // This avoids creating the inCodeBlockStmt node until a successful child parse.

        parse(letStmtVal, lex);
        if (letStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::letStmt, marco, {letStmtVal}};
            return;
        }
        parse(ifStmtVal, lex);
        if (ifStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::ifStmt, marco, {ifStmtVal}};
            return;
        }
        parse(breakStmtVal, lex);
        if (breakStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::breakStmt, marco, {breakStmtVal}};
            return;
        }
        parse(continueStmtVal, lex);
        if (continueStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::continueStmt, marco, {continueStmtVal}};
            return;
        }
        parse(returnStmtVal, lex);
        if (returnStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::returnStmt, marco, {returnStmtVal}};
            return;
        }
        parse(forEachStmtVal, lex); // Only parse once
        if (forEachStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::forEachStmt, marco, {forEachStmtVal}};
            return;
        }
        parse(whileStmtVal, lex);
        if (whileStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::whileStmt, marco, {whileStmtVal}};
            return;
        }

        parse(forStmtVal, lex);
        if (forStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::forStmt, marco, {forStmtVal}};
            return;
        }

        parse(tryCatchStmtVal, lex);
        if (tryCatchStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::tryCatchStmt, marco, {tryCatchStmtVal}};
            return;
        }

        parse(throwStmtVal, lex);
        if (throwStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::throwStmt, marco, {throwStmtVal}};
            return;
        }

        parse(codeBlockVal, lex);
        if (codeBlockVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::codeBlock, marco, {codeBlockVal}};
            return;
        }

        parse(yieldStmtVal, lex);
        if (yieldStmtVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::yieldStmt, marco, {yieldStmtVal}};
            return;
        }

        // rExpr should typically be last, as it's the most general expression statement.
        parse(rExprVal, lex);
        if (rExprVal) {
            o = new inCodeBlockStmt{node_start_token, inCodeBlockStmt::vKind::rExpr, marco, {rExprVal}};
            return;
        }

        if (marco)
            finalizeAST(marco);
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
        defTemplateArg *tempArg = nullptr;
        if (lex.curToken.kind == lexer::token::tokenKind::lessThan) {
            parse(tempArg, lex);
            if (!tempArg) {
                panic(lex.line, lex.col, "expected template argument after `<` in constructor declaration");
                o = nullptr;
                return;
            }
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
        o = new constructorDecl{node_start_token, tempArg, args};
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
        templateArg *tempArg = nullptr;
        definitionArguments *args = nullptr;
        codeBlock *block = nullptr;

        if (lex.curToken.kind == lexer::token::tokenKind::lessThan) {
            parse(tempArg, lex);
            if (!tempArg) {
                panic(lex.line, lex.col, "expected template argument after `<` in constructor definition");
                o = nullptr;
                return;
            }
        }
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
        o = new constructorDef{node_start_token, tempArg, args, block};
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
        while (lex.curToken.kind >= lexer::token::tokenKind::kNoFFI && lex.curToken.kind <= lexer::token::tokenKind::kAlwaysInline) {
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
            for (auto p : catchParams)
                finalizeAST(p);
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
        if (lex.curToken.kind != lexer::token::tokenKind::kInterfaceOf && lex.curToken.kind != lexer::token::tokenKind::kImpl &&
            lex.curToken.kind != lexer::token::tokenKind::kAs) {
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
        lex.saveState();
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftBracket) {
            o = nullptr;
            lex.returnState();
            return;
        }
        lex.scan();
        vec<yoi::lambdaCapture *> captures;
        while (lex.curToken.kind != lexer::token::tokenKind::rightBracket) {
            yoi::lambdaCapture *capture = nullptr;
            parse(capture, lex);
            if (!capture) {
                for (auto c : captures)
                    finalizeAST(c);
                o = nullptr;
                panic(lex.line, lex.col, "expected identifier in capture list in lambda expression");
            }
            captures.push_back(capture);
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
        lex.dropState();
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
                break;
            }
            types.push_back(type);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            }
            lex.scan();
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            for (auto t : types)
                finalizeAST(t);
            panic(lex.line, lex.col, "expected `)` after typeSpecs in unnamed definition arguments");
        }
        lex.scan();
        o = new unnamedDefinitionArguments{node_start_token, types};
    }

    void parse(marcoPair *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        lex.saveState();
        lexer::token lhs, constraint, rhs;
        if (lex.curToken.kind == lexer::token::tokenKind::identifier) {
            lhs = lex.curToken;
        } else {
            lex.returnState();
            o = nullptr;
            return;
        }
        lex.scan();
        switch (lex.curToken.kind) {
            case lexer::token::tokenKind::equal:
            case lexer::token::tokenKind::notEqual:
            case lexer::token::tokenKind::lessEqual:
            case lexer::token::tokenKind::greaterEqual:
            case lexer::token::tokenKind::greaterThan:
            case lexer::token::tokenKind::lessThan: {
                constraint = lex.curToken;
                break;
            }
            default: {
                lex.returnState();
                o = nullptr;
                return;
            }
        }
        lex.scan();
        switch (lex.curToken.kind) {
            case lexer::token::tokenKind::identifier:
            case lexer::token::tokenKind::integer:
            case lexer::token::tokenKind::decimal:
            case lexer::token::tokenKind::string: {
                rhs = lex.curToken;
                break;
            }
            default: {
                lex.returnState();
                o = nullptr;
                return;
            }
        }
        lex.scan();
        lex.dropState();
        o = new marcoPair{node_start_token, lhs, constraint, rhs};
    }

    void parse(marcoDescriptor *&o, lexer &lex) {
        lexer::token current_token = lex.curToken;
        lex.saveState();
        if (current_token.kind != lexer::token::tokenKind::leftBracket) {
            lex.returnState();
            o = new marcoDescriptor{current_token, {}};
            return;
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftBracket) {
            lex.returnState();
            o = new marcoDescriptor{current_token, {}};
            return;
        }
        lex.scan();
        vec<marcoPair *> pairs;
        marcoPair *pair = nullptr;
        parse(pair, lex);
        while (pair) {
            pairs.push_back(pair);
            pair = nullptr;
            parse(pair, lex);
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBracket) {
            o = nullptr;
            for (auto p : pairs)
                finalizeAST(p);
            return;
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::rightBracket) {
            o = nullptr;
            for (auto p : pairs)
                finalizeAST(p);
            return;
        }
        lex.scan();
        lex.dropState();
        o = new marcoDescriptor{current_token, pairs};
    }

    void parse(typeAliasStmt *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kAlias) {
            o = nullptr;
            return;
        }
        lex.scan();
        identifierWithDefTemplateArg *lhs;
        parse(lhs, lex);
        if (!lhs) {
            panic(lex.line, lex.col, "expected identifier with definition template arguments in type alias statement");
            o = nullptr;
            return;
        }
        if (lex.curToken.kind != lexer::token::tokenKind::assignSign) {
            o = nullptr;
            finalizeAST(lhs);
            panic(lex.line, lex.col, "expected `=` after identifier in type alias statement");
            return;
        }
        lex.scan();
        typeSpec *rhs = nullptr;
        parse(rhs, lex);
        if (!rhs) {
            o = nullptr;
            finalizeAST(lhs);
            panic(lex.line, lex.col, "expected typeSpec after `=` in type alias statement");
            return;
        }
        o = new typeAliasStmt{node_start_token, lhs, rhs};
    }

    void parse(finalizerDecl *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kFinalizer) {
            o = nullptr;
            return;
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `(` after `finalizer` in finalizer declaration");
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `)` after `finalizer` in finalizer declaration");
        }
        lex.scan();
        o = new finalizerDecl{node_start_token};
    }

    void parse(finalizerDef *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kFinalizer) {
            o = nullptr;
            return;
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `(` after `finalizer` in finalizer definition");
        }
        lex.scan();
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            panic(lex.line, lex.col, "expected `)` after `finalizer` in finalizer definition");
        }
        lex.scan();
        codeBlock *block = nullptr;
        parse(block, lex);
        if (!block) {
            o = nullptr;
            panic(lex.line, lex.col, "expected codeBlock after `finalizer` declaration");
            return;
        }
        o = new finalizerDef{node_start_token, block};
    }

    void parse(funcExpr *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kFunc) {
            o = nullptr;
            return;
        }
        lex.saveState();
        lex.scan();
        externModuleAccessExpression *name = nullptr;
        parse(name, lex);
        if (!name) {
            o = nullptr;
            lex.returnState();
            return;
        }
        lex.dropState();
        unnamedDefinitionArguments *args = nullptr;
        parse(args, lex);
        if (!args) {
            o = new funcExpr{node_start_token, name, nullptr};
            return;
        }
        o = new funcExpr{node_start_token, name, args};
    }

    void parse(enumerationDefinition *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        if (lex.curToken.kind != lexer::token::tokenKind::kEnum) {
            o = nullptr;
            return;
        }
        lex.scan();
        identifier *name = nullptr;
        parse(name, lex);
        if (!name) {
            o = nullptr;
            panic(lex.line, lex.col, "expected identifier after `enum` in enumeration definition");
            return;
        }
        if (lex.curToken.kind != lexer::token::tokenKind::leftBraces) {
            o = nullptr;
            finalizeAST(name);
            panic(lex.line, lex.col, "expected `{` after identifier in enumeration definition");
            return;
        }
        lex.scan();
        vec<enumerationPair *> enumerators;
        enumerationPair *enumerator = nullptr;
        parse(enumerator, lex);
        while (enumerator) {
            enumerators.push_back(enumerator);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            } else {
                enumerator = nullptr;
                lex.scan();
                parse(enumerator, lex);
            }
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBraces) {
            o = nullptr;
            for (auto e : enumerators)
                finalizeAST(e);
            finalizeAST(name);
            panic(lex.line, lex.col, "expected `}` after enumerators in enumeration definition");
            return;
        }
        lex.scan();
        o = new enumerationDefinition{node_start_token, name, enumerators};
    }

    void parse(enumerationPair *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        identifier *name = nullptr;
        parse(name, lex);
        if (!name) {
            o = nullptr;
            return;
        }
        if (lex.curToken.kind != lexer::token::tokenKind::assignSign) {
            o = new enumerationPair{node_start_token, name, {}};
            return;
        }
        lex.scan();
        lexer::token value = lex.curToken;
        if (value.kind == lexer::token::tokenKind::integer) {
            o = new enumerationPair{node_start_token, name, value};
            lex.scan();
        } else if (value.kind == lexer::token::tokenKind::minus) {
            lex.scan();
            if (lex.curToken.kind != lexer::token::tokenKind::integer) {
                o = nullptr;
                finalizeAST(name);
                panic(lex.line, lex.col, "expected integer literal after `-` in enumeration pair");
                return;
            }
            value = lex.curToken;
            value.basicVal.vInt = -value.basicVal.vInt;
            o = new enumerationPair{node_start_token, name, value};
            lex.scan();
        } else {
            o = nullptr;
            finalizeAST(name);
            panic(lex.line, lex.col, "expected integer literal after `=` in enumeration pair");
            return;
        }
    }

    void parse(bracedInitalizerList *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::leftBraces) {
            o = nullptr;
            return;
        }
        yoi::lexer::token node_start_token = lex.curToken;
        lex.saveState();
        lex.scan();
        vec<rExpr *> expressions;
        rExpr *expression = nullptr;
        parse(expression, lex);
        while (expression) {
            expressions.push_back(expression);
            if (lex.curToken.kind != lexer::token::tokenKind::comma) {
                break;
            } else {
                expression = nullptr;
                lex.scan();
                parse(expression, lex);
            }
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightBraces) {
            o = nullptr;
            for (auto e : expressions)
                finalizeAST(e);
            lex.returnState();
            return;
        }
        lex.scan();
        lex.dropState();
        o = new bracedInitalizerList{node_start_token, expressions};
    }

    void parse(yieldStmt *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kYield) {
            o = nullptr;
            return;
        }
        yoi::lexer::token node_start_token = lex.curToken;
        lex.scan();
        rExpr *expr = nullptr;
        parse(expr, lex);
        if (!expr) {
            o = nullptr;
            panic(lex.line, lex.col, "expected rExpr after `yield` in yieldStmt");
            return;
        }
        o = new yieldStmt{node_start_token, expr};
    }

    void parse(decltypeExpr *&o, lexer &lex) {
        lexer::token node_start_token{lex.curToken};
        if (lex.curToken.kind != lexer::token::tokenKind::kDecltype) {
            o = nullptr;
            return;
        }
        lex.scan();
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::leftParentheses, lex.curToken.line, lex.curToken.col, "expected `(` after `decltype` keyword");
        lex.scan();
        rExpr *expr{};
        parse(expr, lex);
        yoi_assert(expr, lex.line, lex.col, "expected expression after `(` for decltype-expression");
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            finalizeAST(expr);
            panic(lex.line, lex.col, "expected `)` after expression");
        }
        lex.scan();
        o = new decltypeExpr{node_start_token, expr};
    }

    void parse(satisfyStmt *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kSatisfy) {
            o = nullptr;
            return;
        }
        yoi::lexer::token node_start_token = lex.curToken;
        lex.scan();
        externModuleAccessExpression *emae = nullptr;
        parse(emae, lex);
        if (!emae) {
            o = nullptr;
            panic(lex.line, lex.col, "expected externModuleAccessExpression after `satisfy` in satisfyStmt");
            return;
        }
        o = new satisfyStmt{node_start_token, emae};
    }

    void parse(conceptStmt *&o, lexer &lex) {
        yoi::lexer::token node_start_token = lex.curToken;
        
        satisfyStmt *s{};
        parse(s, lex);
        if (s) {
            o = new conceptStmt{node_start_token, conceptStmt::Kind::SatisfyStmt, s};
            return;
        }

        rExpr *e{};
        parse(e, lex);
        if (e) {
            o = new conceptStmt{node_start_token, conceptStmt::Kind::Expression, e};
            return;
        }
    }

    void parse(conceptDefinition *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kConcept) {
            o = nullptr;
            return;
        }
        yoi::lexer::token node_start_token = lex.curToken;
        lex.scan();
        
        lexer::token name{};
        yoi::vec<lexer::token> typeParams;
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::identifier, lex.curToken.line, lex.curToken.col, "expected identifier after `concept` in conceptDefinition");
        name = lex.curToken;
        lex.scan();
        
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::lessThan, lex.curToken.line, lex.curToken.col, "expected `<` after concept name in conceptDefinition");
        lex.scan();
        
        while (lex.curToken.kind == lexer::token::tokenKind::identifier) {
            typeParams.push_back(lex.curToken);
            lex.scan();
            if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
            } else {
                break;
            }
        }
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::greaterThan, lex.curToken.line, lex.curToken.col, "expected `>` after type parameters in conceptDefinition");
        lex.scan();

        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::leftParentheses, lex.curToken.line, lex.curToken.col, "expected `(` after `>`");
        lex.scan();

        yoi::vec<identifierWithTypeSpec *> specs;
        identifierWithTypeSpec *spec{};
        parse(spec, lex);
        while (spec) {
            specs.push_back(spec);
            if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
                spec = nullptr;
                parse(spec, lex);
            } else {
                break;
            }
        }
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::rightParentheses, lex.curToken.line, lex.curToken.col, "expected `)` after specs");
        lex.scan();

        if (lex.curToken.kind != lexer::token::tokenKind::leftBraces) {
            o = nullptr;
            for (auto s : specs)
                finalizeAST(s);
            panic(lex.line, lex.col, "expected `{` after `)`");
            return;
        }
        lex.scan();

        yoi::vec<conceptStmt *> conceptStmts;
        conceptStmt *stmt{};
        parse(stmt, lex);
        while (stmt) {
            conceptStmts.push_back(stmt);
            stmt = nullptr;
            parse(stmt, lex);
        }
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::rightBraces, lex.curToken.line, lex.curToken.col, "expected `}` after satisfyClauses");
        lex.scan();

        o = new conceptDefinition{node_start_token, name, typeParams, specs, conceptStmts};
    }

    void parse(satisfyClause *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::kSatisfy) {
            o = nullptr;
            return;
        }
        lexer::token node_start_token = lex.curToken;
        lex.scan();
        yoi_assert(lex.curToken.kind == lexer::token::tokenKind::leftParentheses, lex.curToken.line, lex.curToken.col, "expected `(` after `satisfy`");
        lex.scan();

        yoi::vec<externModuleAccessExpression *> specs;
        externModuleAccessExpression *spec{};
        parse(spec, lex);
        while (spec) {
            specs.push_back(spec);
            if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
                spec = nullptr;
                parse(spec, lex);
            } else {
                break;
            }
        }
        if (lex.curToken.kind != lexer::token::tokenKind::rightParentheses) {
            o = nullptr;
            for (auto s : specs)
                finalizeAST(s);
            panic(lex.line, lex.col, "expected `)` after specs");
            return;
        }
        lex.scan();
        o = new satisfyClause{node_start_token, specs};
    }

    void parse(lambdaCapture *&o, lexer &lex) {
        lexer::token node_start_token = lex.curToken;
        structDefInnerPair::Modifier mod{structDefInnerPair::Modifier::None};

        if (lex.curToken.kind == lexer::token::tokenKind::kWeak) {
            mod = structDefInnerPair::Modifier::Weak;
            lex.scan();
        } else if (lex.curToken.kind == lexer::token::tokenKind::kDataField) {
            mod = structDefInnerPair::Modifier::DataField;
            lex.scan();
        }

        if (lex.curToken.kind != lexer::token::tokenKind::identifier) {
            o = nullptr;
            return;
        }
        identifier *name = nullptr;
        parse(name, lex);
        if (!name) {
            o = nullptr;
            return;
        }

        o = new lambdaCapture{node_start_token, mod, name};
    }
} // namespace yoi

#pragma clang diagnostic pop
