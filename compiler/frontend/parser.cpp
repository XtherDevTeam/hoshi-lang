#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-qualification"
#pragma ide diagnostic ignored "misc-no-recursion"
//
// Created by XIaokang00010 on 2023/2/11.
//

#include "parser.hpp"

namespace yoi {
    void parse(yoi::basicLiterals *&o, yoi::lexer &lex) {
        switch (lex.curToken.kind) {
            case lexer::token::tokenKind::integer:
            case lexer::token::tokenKind::decimal:
            case lexer::token::tokenKind::character:
            case lexer::token::tokenKind::string:
            case lexer::token::tokenKind::boolean:
            case lexer::token::tokenKind::kNull:
                o = new yoi::basicLiterals(lex.curToken, lex.curToken);
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
        identifier *id;
        typeSpec *spec;
        parse(id, lex);
        if (!id) {
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
        lex.dropState();
        o = new identifierWithTypeSpec{lex.curToken, id, spec};
    }

    void parse(defTemplateArgSpec *&o, lexer &lex) {
        identifier *id;
        identifier *impl;
        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::kImpl) {
            impl = id;
            parse(id, lex);
            if (!id) {
                panic(lex.line, lex.col, "expected identifier after `impl` in defTemplateArgSpec");
                return;
            }
            o = new defTemplateArgSpec{lex.curToken, id, impl};
        }
    }

    void parse(defTemplateArg *&o, lexer &lex) {
        lex.saveState();
        if (lex.curToken.kind != lexer::token::tokenKind::lessThan) {
            lex.returnState();
            o = nullptr;
            return;
        }
        lex.scan();

        vec<defTemplateArgSpec *> specs;
        defTemplateArgSpec *t;
        parse(t, lex);
        while (t) {
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
            parse(t, lex);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::greaterThan) {
            lex.scan();
            o = new defTemplateArg{lex.curToken, specs};
            lex.dropState();
        } else {
            for (auto &i: specs) finalizeAST(i);
            lex.returnState();
            o = nullptr;
            return;
        }
    }

    void parse(templateArgSpec *&o, lexer &lex) {
        typeSpec *spec;
        parse(spec, lex);
        o = spec ? new templateArgSpec{lex.curToken, spec} : nullptr;
    }

    void parse(templateArg *&o, lexer &lex) {
        lex.saveState();
        if (lex.curToken.kind != lexer::token::tokenKind::lessThan) {
            lex.dropState();
            o = nullptr;
            return;
        }
        lex.scan();

        vec<templateArgSpec *> specs;
        templateArgSpec *t;
        parse(t, lex);
        while (t) {
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
            parse(t, lex);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::greaterThan) {
            lex.scan();
            o = new templateArg{lex.curToken, specs};
            lex.dropState();
        } else {
            lex.returnState();
            for (auto &i: specs) finalizeAST(i);
            o = nullptr;
            return;
        }
    }

    void parse(invocationArguments *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            return;
        }
        lex.scan();

        vec<rExpr *> args;
        rExpr *t;
        parse(t, lex);
        while (t) {
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
            parse(t, lex);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
            o = new invocationArguments{lex.curToken, args};
        } else {
            for (auto &i: args) finalizeAST(i);
            panic(lex.line, lex.col, "expected `]` to close an arguments node");
            return;
        }
    }

    void parse(definitionArguments *&o, lexer &lex) {
        if (lex.curToken.kind != lexer::token::tokenKind::leftParentheses) {
            o = nullptr;
            return;
        }
        lex.scan();

        vec<identifierWithTypeSpec *> args;
        identifierWithTypeSpec *t;
        parse(t, lex);
        while (t) {
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
            parse(t, lex);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
            o = new definitionArguments{lex.curToken, args};
        } else {
            for (auto &i: args) finalizeAST(i);
            panic(lex.line, lex.col, "expected `]` to close an arguments node");
            return;
        }
    }

    void parse(funcTypeSpec *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kFunc)
            lex.scan();
        else {
            o = nullptr;
            return;
        }
        definitionArguments *args;
        typeSpec *spec;
        parse(args, lex);
        if (!args) {
            panic(lex.line, lex.col, "expected definitionArguments after `func`");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon)
            lex.scan();
        else {
            panic(lex.line, lex.col, "expected `:` after definitionArguments");
            return;
        }
        parse(spec, lex);
        if (!spec) {
            panic(lex.line, lex.col, "expected typeSpec after `:`");
            return;
        }
        o = new funcTypeSpec{lex.curToken, args, spec};
    }

    void parse(typeSpec *&o, lexer &lex) {
        memberExpr *expr;
        funcTypeSpec *spec;
        if (lex.curToken.kind == lexer::token::tokenKind::kNull) {
            lex.scan();
            o = new typeSpec{lex.curToken, 2, nullptr, nullptr, true};
            return;
        }
        parse(spec, lex);
        if (spec) {
            o = new typeSpec{lex.curToken, 1, nullptr, spec, false};
            return;
        }
        parse(expr, lex);
        if (expr) {
            o = new typeSpec{lex.curToken, 0, expr, nullptr, false};
            return;
        }
        o = nullptr;
    }

    void parse(subscript *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        rExpr *r;
        parse(r, lex);
        if (!r) {
            panic(lex.line, lex.col, "expected rightValueExpr in subscript");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::leftBracket) {
            lex.scan();
            o = new subscript{lex.curToken, r};
        } else {
            panic(lex.line, lex.col, "expected `]` to close a subscript");
            return;
        }
    }

    void parse(identifierWithTemplateArg *&o, lexer &lex) {
        identifierWithTemplateArg *node;
        identifier *id;
        templateArg *arg;
        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }
        node = new identifierWithTemplateArg{lex.curToken, id, nullptr};
        parse(arg, lex);
        if (!arg) {
            o = node;
            return;
        }
        o->arg = arg;
    }

    void parse(identifierWithDefTemplateArg *&o, lexer &lex) {
        identifierWithDefTemplateArg *node;
        identifier *id;
        defTemplateArg *arg;
        parse(id, lex);
        if (!id) {
            o = nullptr;
            return;
        }
        node = new identifierWithDefTemplateArg{lex.curToken, id, nullptr};
        parse(arg, lex);
        if (!arg) {
            o = node;
            return;
        }
        o->arg = arg;
    }

    void parse(subscriptExpr *&o, lexer &lex) {
        subscriptExpr *expr;
        identifierWithTemplateArg *a;
        invocationArguments *b;
        subscript *c;
        parse(a, lex);
        if (!a) {
            o = nullptr;
            return;
        }
        expr = new subscriptExpr{lex.curToken, a, nullptr, nullptr};
        parse(b, lex);
        if (b) {
            expr->arg = b;
            o = expr;
            return;
        }
        parse(c, lex);
        if (c) {
            expr->subscript = c;
            o = expr;
            return;
        }
        o = expr;
    }

    void parse(memberExpr *&o, lexer &lex) {
        vec<subscriptExpr *> vecA;
        subscriptExpr *a;
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
            parse(a, lex);
        }
        o = new memberExpr{lex.curToken, vecA};
    }

    void parse(primary *&o, lexer &lex) {
        memberExpr *a;
        basicLiterals *b;
        rExpr *c;
        parse(a, lex);
        if (a) {
            o = new primary{lex.curToken, 0, a, nullptr, nullptr};
            return;
        }
        parse(b, lex);
        if (b) {
            o = new primary{lex.curToken, 1, nullptr, b, nullptr};
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
            parse(c, lex);
            if (c) {
                o = new primary{lex.curToken, 2, nullptr, nullptr, c};
            } else {
                panic(lex.line, lex.col, "expected rightValueExpr after `(` while parsing primary");
            }
            if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
                lex.scan();
                return;
            } else {
                panic(lex.line, lex.col, "expected `)` after rightValueExpr while parsing primary");
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
                break;
            }
            default: {
                break;
            }
        }
        primary *expr;
        parse(expr, lex);
        if (!expr) {
            lex.returnState();
            o = nullptr;
            return;
        }
        o = new uniqueExpr{lex.curToken, t, expr};
        lex.dropState();
    }

    void parse(mulExpr *&o, lexer &lex) {
        vec<uniqueExpr *> vecA;
        vec<lexer::token> vecB;
        lexer::token b;
        uniqueExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (b.kind == lexer::token::tokenKind::asterisk || b.kind == lexer::token::tokenKind::slash
                   || b.kind == lexer::token::tokenKind::percentSign) {
                vecB.push_back(b);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected uniqueExpr after operators while parsing mulExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new mulExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(addExpr *&o, lexer &lex) {
        vec<mulExpr *> vecA;
        vec<lexer::token> vecB;
        mulExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::plus || lex.curToken.kind == lexer::token::tokenKind::minus) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected mulExpr after operators while parsing addExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new addExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(shiftExpr *&o, lexer &lex) {
        vec<addExpr *> vecA;
        vec<lexer::token> vecB;
        addExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::binaryShiftLeft ||
                    lex.curToken.kind == lexer::token::tokenKind::binaryShiftRight) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected addExpr after operators while parsing shiftExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new shiftExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(relationalExpr *&o, lexer &lex) {
        vec<shiftExpr *> vecA;
        vec<lexer::token> vecB;
        shiftExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::lessThan || lex.curToken.kind == lexer::token::tokenKind::greaterThan ||
                    lex.curToken.kind == lexer::token::tokenKind::lessEqual || lex.curToken.kind == lexer::token::tokenKind::greaterEqual) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected shiftExpr after operators while parsing relationalExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new relationalExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(equalityExpr *&o, lexer &lex) {
        vec<relationalExpr *> vecA;
        vec<lexer::token> vecB;
        relationalExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::equal || lex.curToken.kind == lexer::token::tokenKind::notEqual) {
                vecB.push_back( lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected relationalExpr after operators while parsing equalityExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new equalityExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(andExpr *&o, lexer &lex) {
        vec<equalityExpr *> vecA;
        vec<lexer::token> vecB;
        equalityExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::binaryAnd) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected relationalExpr after operators while parsing equalityExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new andExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(exclusiveExpr *&o, lexer &lex) {
        vec<andExpr *> vecA;
        vec<lexer::token> vecB;
        andExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::binaryXor) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected andExpr after operators while parsing exclusiveExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new exclusiveExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(inclusiveExpr *&o, lexer &lex) {
        vec<exclusiveExpr *> vecA;
        vec<lexer::token> vecB;
        exclusiveExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::binaryOr) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected exclusiveExpr after operators while parsing inclusiveExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new inclusiveExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(logicalAndExpr *&o, lexer &lex) {
        vec<inclusiveExpr *> vecA;
        vec<lexer::token> vecB;
        inclusiveExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::logicAnd) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected inclusiveExpr after operators while parsing logicalAndExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new logicalAndExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(logicalOrExpr *&o, lexer &lex) {
        vec<logicalAndExpr *> vecA;
        vec<lexer::token> vecB;
        logicalAndExpr *a;
        parse(a, lex);
        if (a) {
            vecA.push_back(a);
            while (lex.curToken.kind == lexer::token::tokenKind::logicOr) {
                vecB.push_back(lex.curToken);
                lex.scan();
                parse(a, lex);
                if (!a) {
                    panic(lex.line, lex.col, "expected logicalAndExpr after operators while parsing logicalOrExpr");
                    return;
                }
                vecA.push_back(a);
            }
            o = new logicalOrExpr{lex.curToken, vecA, vecB};
        } else {
            o = nullptr;
        }
    }

    void parse(rExpr *&o, lexer &lex) {
        logicalOrExpr *expr;
        parse(expr, lex);
        if (expr) {
            o = new rExpr{lex.curToken, expr};
            return;
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(codeBlock *&o, lexer &lex) {
        vec<inCodeBlockStmt *> stmts;
        inCodeBlockStmt *stmt;
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        while (true) {
            parse(stmt, lex);
            if (!stmt)
                break;
            stmts.push_back(stmt);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `}` to close codeBlock");
            return;
        }
        o = new codeBlock{lex.curToken, stmts};
    }

    void parse(useStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kUse) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        identifier *id;
        lexer::token str;
        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected identifier after `use`");
            return;
        }
        if (lex.curToken.kind != lexer::token::tokenKind::string) {
            panic(lex.line, lex.col, "expected string token after identifier while parsing useStmt");
            return;
        }
        str = lex.curToken;
        lex.scan();
        o = new useStmt{lex.curToken, id, str};
    }

    void parse(funcDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kFunc)
            lex.scan();
        else {
            o = nullptr;
            return;
        }
        identifierWithDefTemplateArg *name;
        definitionArguments *args;
        typeSpec *spec;
        codeBlock *block;
        parse(name, lex);
        if (!name) {
            panic(lex.line, lex.col, "expected function name");
            return;
        }
        parse(args, lex);
        if (!args) {
            panic(lex.line, lex.col, "expected definitionArguments after identifierWithDefTemplateArg");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon)
            lex.scan();
        else {
            panic(lex.line, lex.col, "expected `:` after definitionArguments");
            return;
        }
        parse(spec, lex);
        if (!spec) {
            panic(lex.line, lex.col, "expected typeSpec after `:`");
            return;
        }
        parse(block, lex);
        if (!block) {
            panic(lex.line, lex.col, "expected codeBlock after typeSpec");
            return;
        }
        o = new funcDefStmt{lex.curToken, name, args, spec, block};
    }

    void parse(interfaceDefInnerPair *&o, lexer &lex) {
        identifierWithTypeSpec *var;
        innerMethodDecl *method;
        parse(method, lex);
        if (method) {
            o = new interfaceDefInnerPair{lex.curToken, nullptr, method};
            return;
        }
        parse(var, lex);
        if (var) {
            o = new interfaceDefInnerPair{lex.curToken, var, nullptr};
            return;
        }
        o = nullptr;
    }

    void parse(structDefInnerPair *&o, lexer &lex) {
        identifierWithTypeSpec *var;
        innerMethodDecl *method;
        constructorDecl *con;
        parse(con, lex);
        if (con) {
            o = new structDefInnerPair{lex.curToken, 1, nullptr, con, nullptr};
            return;
        }
        parse(method, lex);
        if (method) {
            o = new structDefInnerPair{lex.curToken, 2, nullptr, nullptr, method};
            return;
        }
        parse(var, lex);
        if (var) {
            o = new structDefInnerPair{lex.curToken, 0, nullptr, nullptr, method};
            return;
        }
        o = nullptr;
    }

    void parse(implInnerPair *&o, lexer &lex) {
        innerMethodDef *method;
        constructorDef *con;
        parse(con, lex);
        if (con) {
            o = new implInnerPair{lex.curToken, con, nullptr};
            return;
        }
        parse(method, lex);
        if (method) {
            o = new implInnerPair{lex.curToken, nullptr, method};
            return;
        }
        o = nullptr;
    }

    void parse(interfaceDefInner *&o, lexer &lex) {
        vec<interfaceDefInnerPair *> vecA;
        interfaceDefInnerPair *a;
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        while (true) {
            parse(a, lex);
            if (!a)
                break;
            vecA.push_back(a);
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `}` to close interfaceDefInner");
            return;
        }
        o = new interfaceDefInner{lex.curToken, vecA};
    }

    void parse(structDefInner *&o, lexer &lex) {
        vec<structDefInnerPair *> vecA;
        structDefInnerPair *a;
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        while (true) {
            parse(a, lex);
            if (!a)
                break;
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
            vecA.push_back(a);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `}` to close structDefInner");
            return;
        }
        o = new structDefInner{lex.curToken, vecA};
    }

    void parse(implInner *&o, lexer &lex) {
        vec<implInnerPair *> vecA;
        implInnerPair *a;
        if (lex.curToken.kind == lexer::token::tokenKind::leftBraces) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        while (true) {
            parse(a, lex);
            if (!a)
                break;
            if (lex.curToken.kind == lexer::token::tokenKind::comma)
                lex.scan();
            else
                break;
            vecA.push_back(a);
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightBraces) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `}` to close implInner");
            return;
        }
        o = new implInner{lex.curToken, vecA};
    }

    void parse(interfaceDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kInterface)
            lex.scan();
        else {
            o = nullptr;
            return;
        }
        identifier *id;
        interfaceDefInner *inner;
        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected interface name after `interface`");
            return;
        }
        parse(inner, lex);
        if (!inner) {
            panic(lex.line, lex.col, "expected interfaceDefInner after identifier");
            return;
        }
        o = new interfaceDefStmt{lex.curToken, id, inner};
    }

    void parse(structDefStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kStruct)
            lex.scan();
        else {
            o = nullptr;
            return;
        }
        identifierWithDefTemplateArg *id;
        structDefInner *inner;
        parse(id, lex);
        if (!id) {
            panic(lex.line, lex.col, "expected struct name after `struct`");
            return;
        }
        parse(inner, lex);
        if (!inner) {
            panic(lex.line, lex.col, "expected structDefInner after identifier");
            return;
        }
        o = new structDefStmt{lex.curToken, id, inner};
    }

    void parse(implStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kImpl)
            lex.scan();
        else {
            o = nullptr;
            return;
        }
        identifier *first{}, *second{};
        implInner *inner;
        parse(first, lex);
        if (!first) {
            panic(lex.line, lex.col, "expected interface or struct name after `impl`");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::kFor) {
            second = first;
            parse(first, lex);
            if (!first) {
                panic(lex.line, lex.col, "expected struct name after `for`");
                return;
            }
        }
        parse(inner, lex);
        if (!inner) {
            panic(lex.line, lex.col, "expected implInner after interface or struct name");
            return;
        }

        o = new implStmt{lex.curToken, first, second, inner};
    }

    void parse(letAssignmentPair *&o, lexer &lex) {
        identifier *lhs;
        rExpr *rhs;
        parse(lhs, lex);
        if (!lhs) {
            panic(lex.line, lex.col, "expected left-hand-side in letAssignmentPair");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::assignSign) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `=` after left-hand-side in letAssignmentPair");
            return;
        }
        parse(rhs, lex);
        if (!rhs) {
            panic(lex.line, lex.col, "expected right-hand-side in letAssignmentPair");
            return;
        }
        o = new letAssignmentPair{lex.curToken, lhs, rhs};
    }

    void parse(letStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kLet) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        vec<letAssignmentPair *> vecA;
        letAssignmentPair *a;
        while (true) {
            parse(a, lex);
            vecA.push_back(a);
            if (lex.curToken.kind == lexer::token::tokenKind::comma) {
                lex.scan();
            } else {
                break;
            }
        }
        o = new letStmt{lex.curToken, vecA};
    }

    void parse(globalStmt *&o, lexer &lex) {
        useStmt *a;
        interfaceDefStmt *b;
        structDefStmt *c;
        implStmt *d;
        letStmt *e;
        funcDefStmt *f;

        parse(a, lex);
        if (a) {
            o = new globalStmt{lex.curToken, globalStmt::vKind::useStmt, a};
            return;
        }

        parse(b, lex);
        if (b) {
            o = new globalStmt{lex.curToken, globalStmt::vKind::interfaceDefStmt, b};
            return;
        }

        parse(c, lex);
        if (c) {
            o = new globalStmt{lex.curToken, globalStmt::vKind::structDefStmt, c};
            return;
        }

        parse(d, lex);
        if (d) {
            o = new globalStmt{lex.curToken, globalStmt::vKind::implStmt, d};
            return;
        }

        parse(e, lex);
        if (e) {
            o = new globalStmt{lex.curToken, globalStmt::vKind::letStmt, e};
            return;
        }

        parse(f, lex);
        if (f) {
            o = new globalStmt{lex.curToken, globalStmt::vKind::funcDefStmt, f};
            return;
        }

        o = nullptr;
    }

    void parse(ifStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kIf) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }

        o = new ifStmt{lex.curToken, {}, {}, nullptr};
        ifStmt::ifBlock i{};
        codeBlock *block;

        parse(i, lex);
        o->ifB = i;

        while (lex.curToken.kind == lexer::token::tokenKind::kElif) {
            lex.scan();
            parse(i, lex);
            o->elifB.push_back(i);
        }

        if (lex.curToken.kind == lexer::token::tokenKind::kElse) {
            lex.scan();
            parse(o->elseB, lex);
            if (!o->elseB) {
                panic(lex.line, lex.col, "expected codeBlock after `else`");
            }
        }
    }

    void parse(ifStmt::ifBlock &o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(`");
            return;
        }

        parse(o.cond, lex);
        if (!o.cond) {
            panic(lex.line, lex.col, "expected rExpr after `)`");
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `)` after rExpr");
            return;
        }

        parse(o.block, lex);
        if (!o.block) {
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            return;
        }
    }

    void parse(whileStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kWhile) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }

        rExpr *expr;
        codeBlock *block;

        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(` after `while`");
            return;
        }

        parse(expr, lex);
        if (!expr) {
            panic(lex.line, lex.col, "expected rExpr after `)`");
            return;
        }

        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `)` after rExpr");
            return;
        }

        parse(block, lex);
        if (!block) {
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            return;
        }

        o = new whileStmt{lex.curToken, expr, block};
    }

    void parse(forStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kFor) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        inCodeBlockStmt *initStmt;
        rExpr *cond;
        inCodeBlockStmt *afterStmt;
        codeBlock *block;
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(` after `for`");
            return;
        }
        parse(initStmt, lex);
        if (!initStmt) {
            panic(lex.line, lex.col, "expected initStmt after `(`");
        }
        if (lex.curToken.kind == lexer::token::tokenKind::semicolon) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `;` after initStmt");
            return;
        }
        parse(cond, lex);
        if (!cond) {
            panic(lex.line, lex.col, "expected condition after `;`");
        }
        if (lex.curToken.kind == lexer::token::tokenKind::semicolon) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `;` after condition");
            return;
        }
        parse(afterStmt, lex);
        if (!cond) {
            panic(lex.line, lex.col, "expected afterStmt after `;`");
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `)` after afterStmt");
            return;
        }
        parse(block, lex);
        if (!block) {
            panic(lex.line, lex.col, "expected codeBlock after `)`");
        }
        o = new forStmt{lex.curToken, initStmt, cond, afterStmt, block};
    }

    void parse(forEachStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kForEach) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        o = new forEachStmt{lex.curToken, nullptr, nullptr, nullptr};
        if (lex.curToken.kind == lexer::token::tokenKind::leftParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `(` after `forEach`");
            return;
        }
        parse(o->var, lex);
        if (!o->var) {
            panic(lex.line, lex.col, "expected variable name after `(`");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::colon) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `:` after variable name");
            return;
        }
        parse(o->container, lex);
        if (!o->container) {
            panic(lex.line, lex.col, "expected container after `:`");
            return;
        }
        if (lex.curToken.kind == lexer::token::tokenKind::rightParentheses) {
            lex.scan();
        } else {
            panic(lex.line, lex.col, "expected `)` after container");
            return;
        }
        parse(o->block, lex);
        if (!o->block) {
            panic(lex.line, lex.col, "expected codeBlock after `)`");
            return;
        }
    }

    void parse(returnStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kReturn) {
            lex.scan();
            rExpr *expr;
            parse(expr, lex);
            if (expr) {
                o = new returnStmt{lex.curToken, expr};
            } else {
                o = new returnStmt{lex.curToken, nullptr};
            }
        } else {
            o = nullptr;
        }
    }

    void parse(continueStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kContinue) {
            lex.scan();
            o = new continueStmt{};
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(breakStmt *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kBreak) {
            lex.scan();
            o = new breakStmt{};
        } else {
            o = nullptr;
            return;
        }
    }

    void parse(inCodeBlockStmt *&o, lexer &lex) {
        o = new inCodeBlockStmt{lex.curToken, inCodeBlockStmt::vKind::ifStmt, {(void *) nullptr}};
        parse(o->value.letStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::letStmt;
            return;
        }
        parse(o->value.ifStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::ifStmt;
            return;
        }
        parse(o->value.breakStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::breakStmt;
            return;
        }
        parse(o->value.continueStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::continueStmt;
            return;
        }
        parse(o->value.returnStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::returnStmt;
            return;
        }
        parse(o->value.forEachStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::forEachStmt;
            return;
        }
        parse(o->value.whileStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::whileStmt;
            return;
        }
        parse(o->value.forStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::forStmt;
            return;
        }
        parse(o->value.forEachStmt, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::forEachStmt;
            return;
        }
        parse(o->value.codeBlock, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::codeBlock;
            return;
        }
        parse(o->value.rExpr, lex);
        if (o->value.ptr) {
            o->kind = inCodeBlockStmt::vKind::rExpr;
            return;
        }

        delete o;
        o = nullptr;
    }

    void parse(innerMethodDecl *&o, lexer &lex) {
        lex.saveState();
        o = new innerMethodDecl{lex.curToken, nullptr, nullptr, nullptr};
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
        lex.dropState();
    }

    void parse(innerMethodDef *&o, lexer &lex) {
        lex.saveState();
        o = new innerMethodDef{lex.curToken, nullptr, nullptr, nullptr, nullptr};
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
        if (lex.curToken.kind == lexer::token::tokenKind::kConstructor) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        o = new constructorDecl{lex.curToken, nullptr};
        parse(o->args, lex);
        if (!o->args) {
            panic(lex.line, lex.col, "expected arguments after `constructor`");
            return;
        }
    }

    void parse(constructorDef *&o, lexer &lex) {
        if (lex.curToken.kind == lexer::token::tokenKind::kConstructor) {
            lex.scan();
        } else {
            o = nullptr;
            return;
        }
        o = new constructorDef{lex.curToken, nullptr, nullptr};
        parse(o->args, lex);
        if (!o->args) {
            panic(lex.line, lex.col, "expected arguments after `constructor`");
            return;
        }
        parse(o->block, lex);
        if (!o->block) {
            panic(lex.line, lex.col, "expected codeBlock after arguments");
            return;
        }
    }

    void parse(hoshiModule *&o, lexer &lex) {
        vec<globalStmt *> vecA;
        globalStmt *a;
        while (true) {
            if (lex.curToken.kind == lexer::token::tokenKind::eof)
                break;
            parse(a, lex);
            if (!a) {
                panic(lex.line, lex.col, "expected globalStmt");
                return;
            }
            vecA.push_back(a);
        }
        o = new hoshiModule{lex.curToken, vecA};
    }
}

#pragma clang diagnostic pop