#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"
#pragma ide diagnostic ignored "google-explicit-constructor"
//
// Created by XIaokang00010 on 2023/1/24.
//

#ifndef HOSHI_LANG_AST_HPP
#define HOSHI_LANG_AST_HPP

#include <share/def.hpp>

#include "lexer.hpp"

namespace hoshi {
    class basicLiterals;

    class identifier;

    class identifierWithTypeSpec;

    class defTemplateArgSpec;

    class defTemplateArg;

    class templateArgSpec;

    class templateArg;

    class invocationArguments;

    class definitionArguments;

    class funcTypeSpec;

    class typeSpec;

    class subscript;

    class identifierWithTemplateArg;

    class identifierWithDefTemplateArg;

    class subscriptExpr;

    class accessExpr;

    class memberExpr;

    class primary;

    class uniqueExpr;

    class mulExpr;

    class addExpr;

    class shiftExpr;

    class relationalExpr;

    class equalityExpr;

    class andExpr;

    class exclusiveExpr;

    class inclusiveExpr;

    class logicalAndExpr;

    class logicalOrExpr;

    class rExpr;

    class codeBlock;

    class useStmt;

    class funcDefStmt;

    class interfaceDefInnerPair;

    class interfaceDefInner;

    class interfaceDefStmt;

    class structDefInnerPair;

    class structDefInner;

    class structDefStmt;

    class implInnerPair;

    class implInner;

    class implStmt;

    class letAssignmentPair;

    class letStmt;

    class globalStmt;

    class ifStmt;

    class whileStmt;

    class forStmt;

    class forEachStmt;

    class returnStmt;

    class continueStmt;

    class breakStmt;

    class inCodeBlockStmt;

    class codeBlock;

    class basicLiterals {
    public:
        lexer::token node;

        lexer::token &get();
    };

    class identifier {
    public:
        lexer::token node;

        lexer::token &get();
    };

    class identifierWithTypeSpec {
    public:
        identifier *id;
        typeSpec *spec;

        identifier &getId() const;

        typeSpec &getSpec() const;
    };

    class defTemplateArgSpec {
    public:
        identifier *id;
        identifier *impl;

        identifier &getId() const;

        identifier &getImpl() const;
    };

    class defTemplateArg {
    public:
        vec<defTemplateArgSpec *> spec;

        vec<defTemplateArgSpec *> &get();
    };

    class templateArgSpec {
    public:
        typeSpec *spec;

        typeSpec &get() const;
    };

    class templateArg {
    public:
        vec<typeSpec *> spec;

        vec<typeSpec *> &get();
    };

    class invocationArguments {
    public:
        vec<rExpr *> arg;

        vec<rExpr *> &get();
    };

    class definitionArguments {
    public:
        vec<identifierWithTypeSpec *> spec;

        vec<identifierWithTypeSpec *> &get();
    };

    class funcTypeSpec {
    public:
        definitionArguments *args;
        typeSpec *resultType;
        codeBlock *block;

        definitionArguments &getArgs() const;

        typeSpec &getResultType() const;

        codeBlock &getBlock() const;
    };

    class typeSpec {
    public:
        accessExpr *access;
        funcTypeSpec *func;

        bool isFuncTypeSpec() const;

        accessExpr &getAccessExpr() const;

        funcTypeSpec &getTypeSpec() const;
    };

    class subscript {
    public:
        rExpr *expr;

        rExpr &getExpr() const;
    };

    class identifierWithTemplateArg {
    public:
        identifier *id;
        templateArg *arg;

        identifier &getId() const;

        templateArg &getArg() const;
    };

    class identifierWithDefTemplateArg {
    public:
        identifier *id;
        defTemplateArg *arg;

        identifier &getId() const;

        defTemplateArg &getArg() const;
    };

    class subscriptExpr {
    public:
        identifierWithTemplateArg *id;
        invocationArguments *arg;
        subscriptExpr *subscript;

        bool isInvocation() const;

        identifierWithTemplateArg &getId() const;

        invocationArguments &getArg() const;

        subscriptExpr &getSubscript() const;
    };

    class accessExpr {
    public:
        vec<identifier *> prefix;
        identifierWithTemplateArg *term;

        vec<identifier *> &getPrefix();

        identifierWithTemplateArg &getTerm() const;
    };

    class memberExpr {
    public:
        vec<identifier *> prefix;
        vec<subscriptExpr *> terms;

        vec<identifier *> &getPrefix();

        vec<subscriptExpr *> &getTerms();
    };

    class primary {
    public:
        int8_t kind; // 0 is memberExpr 1 is basicLiterals 2 is rExpr
        memberExpr *member;
        basicLiterals *literals;
        rExpr *expr;

        memberExpr &getMemberExpr() const;

        basicLiterals &getLiterals() const;

        rExpr &getExpr() const;
    };

    class uniqueExpr {
    public:
        lexer::token op;
        primary *lhs;

        lexer::token &getOp();

        primary &getLhs() const;

        operator bool() const;
    };

    class mulExpr {
    public:
        vec<uniqueExpr *> terms;
        vec<lexer::token> ops;

        vec<uniqueExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class addExpr {
    public:
        vec<mulExpr *> terms;
        vec<lexer::token> ops;

        vec<mulExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class shiftExpr {
    public:
        vec<addExpr *> terms;
        vec<lexer::token> ops;

        vec<addExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class relationalExpr {
    public:
        vec<shiftExpr *> terms;
        vec<lexer::token> ops;

        vec<shiftExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class equalityExpr {
    public:
        vec<relationalExpr *> terms;
        vec<lexer::token> ops;

        vec<relationalExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class andExpr {
    public:
        vec<equalityExpr *> terms;
        vec<lexer::token> ops;

        vec<equalityExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class exclusiveExpr {
    public:
        vec<andExpr *> terms;
        vec<lexer::token> ops;

        vec<andExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class inclusiveExpr {
    public:
        vec<exclusiveExpr *> terms;
        vec<lexer::token> ops;

        vec<exclusiveExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class logicalAndExpr {
    public:
        vec<inclusiveExpr *> terms;
        vec<lexer::token> ops;

        vec<inclusiveExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class logicalOrExpr {
    public:
        vec<logicalAndExpr *> terms;
        vec<lexer::token> ops;

        vec<logicalAndExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class rExpr {
    public:
        logicalOrExpr *expr;

        logicalOrExpr &getExpr() const;
    };

    class useStmt {
    public:
        identifier *name;
        lexer::token &path;

        identifier &getName();

        lexer::token &getPath();
    };

    class funcDefStmt {
        identifierWithDefTemplateArg *id;
        definitionArguments *args;
        typeSpec *resultType;
        codeBlock *block;

        identifierWithDefTemplateArg &getId();

        definitionArguments &getArgs();

        typeSpec &getResultType();

        codeBlock &getBlock();
    };

    class interfaceDefInnerPair {
    public:
        // member var
        identifierWithTypeSpec *var;

        identifierWithTypeSpec &getVar();

        // method
        identifierWithDefTemplateArg *id;
        definitionArguments *args;
        typeSpec *resultType;

        identifierWithDefTemplateArg &getMethodId();

        definitionArguments &getMethodArgs();

        typeSpec &getMethodResultType();

        bool isMethod();
    };

    class interfaceDefInner {
    public:
        vec<interfaceDefInnerPair *> inner;

        vec<interfaceDefInnerPair *> &getInner();
    };

    class interfaceDefStmt {
    public:
        identifier *id;
        interfaceDefInner *inner;

        identifier &getId();

        interfaceDefInner &getInner();
    };

    class structDefInnerPair {
    public:
        // 0 is member 1 is constructor 2 is method
        int8_t kind;
        // member var
        identifierWithTypeSpec *var;
        // constructor
        definitionArguments *conArgs;
        // method
        identifierWithDefTemplateArg *methodId;
        definitionArguments *methodArgs;
        typeSpec *methodResultType;

        identifierWithTypeSpec &getVar();

        definitionArguments &getConArgs();

        identifierWithDefTemplateArg &getMethodId();

        definitionArguments &getMethodArgs();

        typeSpec &getMethodResultType();
    };

    class structDefInner {
    public:
        vec<structDefInnerPair *> inner;

        vec<structDefInnerPair *> &getInner();
    };

    class structDefStmt {
    public:
        identifierWithDefTemplateArg *id;

        structDefInner *inner;

        identifierWithDefTemplateArg &getId();

        structDefInner &getInner();
    };

    class implInnerPair {
    public:
        // 0 is member 1 is constructor 2 is method
        int8_t kind;
        // member var
        identifierWithTypeSpec *var;
        // constructor
        definitionArguments *conArgs;
        codeBlock *conBlock;
        // method
        identifierWithDefTemplateArg *methodId;
        definitionArguments *methodArgs;
        typeSpec *methodResultType;
        codeBlock *methodBlock;

        identifierWithTypeSpec &getVar();

        definitionArguments &getConArgs();

        codeBlock &getConBlock();

        identifierWithDefTemplateArg &getMethodId();

        definitionArguments &getMethodArgs();

        typeSpec &getMethodResultType();

        codeBlock &getMethodBlock();
    };

    class implInner {
    public:
        vec<implInnerPair *> inner;

        vec<implInnerPair *> &getInner();
    };

    class implStmt {
    public:
        identifierWithDefTemplateArg *interfaceName;
        identifierWithDefTemplateArg *structName;

        identifierWithDefTemplateArg &getInterfaceId();

        identifierWithDefTemplateArg &getStructId();

        bool isImplForStmt();
    };

    class letAssignmentPair {
    public:
        identifier *lhs;
        rExpr *rhs;

        identifier &getLhs();

        rExpr &getRhs();
    };

    class letStmt {
    public:
        vec<letAssignmentPair *> terms;

        vec<letAssignmentPair *> &getTerms();
    };

    class globalStmt {
    public:
        enum class vKind : int16_t {
            useStmt,
            interfaceDefStmt,
            structDefStmt,
            implStmt,
            letStmt
        } kind;

        union vValue {
            useStmt *useStmt;
            interfaceDefStmt *interfaceDefStmt;
            structDefStmt *structDefStmt;
            implStmt *implStmt;
            letStmt *letStmt;
            void *ptr;

            template<typename T>
            vValue(T *t) : ptr((void *) t) {}
        } value;

        vKind &getKind();

        vValue &getValue();
    };

    class ifStmt {
    public:
        struct ifBlock {
            rExpr *cond;
            codeBlock *block;

            rExpr &getCond();

            codeBlock &getBlock();
        };

        ifBlock ifB;
        vec<ifBlock> elifB;
        codeBlock *elseB;

        ifBlock &getIfBlock();

        vec<ifBlock> &getElifBlock();

        codeBlock &getElseBlock();
    };

    class whileStmt {
    public:
        rExpr *cond;
        codeBlock *block;

        rExpr &getCond();

        codeBlock &getBlock();
    };

    class forStmt {
    public:
        inCodeBlockStmt *initStmt;
        rExpr *cond;
        inCodeBlockStmt *afterStmt;

        inCodeBlockStmt &getInitStmt();

        rExpr &getCond();

        inCodeBlockStmt &getAfterStmt();
    };

    class forEachStmt {
    public:
        identifier *var;
        rExpr *container;

        identifier &getVar();

        rExpr &getContainer();
    };

    class returnStmt {
    public:
        rExpr *value;

        rExpr &getValue();
    };

    class continueStmt {

    };

    class breakStmt {

    };

    class inCodeBlockStmt {
    public:
        enum class vKind : int16_t {
            ifStmt,
            whileStmt,
            forEachStmt,
            returnStmt,
            continueStmt,
            breakStmt,
            letStmt,
            codeBlock,
            rExpr,
        } kind;

        union vValue {
            ifStmt *ifStmt;
            whileStmt *whileStmt;
            forEachStmt *forEachStmt;
            returnStmt *returnStmt;
            continueStmt *continueStmt;
            breakStmt *breakStmt;
            letStmt *letStmt;
            codeBlock *codeBlock;
            rExpr *rExpr;
            void *ptr;

            template<typename T>
            vValue(T *t) : ptr((void *) t) {}
        } value;

        vKind &getKind();

        vValue &getValue();
    };

    class codeBlock {
    public:
        vec<inCodeBlockStmt *> stmts;

        vec<inCodeBlockStmt *> &getStmts();
    };
} // hoshi

#endif //HOSHI_LANG_AST_HPP

#pragma clang diagnostic pop