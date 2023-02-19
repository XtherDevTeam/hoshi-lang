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
    class hoshiModule;

    class innerMethodDef;

    class constructorDef;

    class innerMethodDecl;

    class constructorDecl;

    class subscript;

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
        vec<templateArgSpec *> spec;

        vec<templateArgSpec *> &get();
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

        definitionArguments &getArgs() const;

        typeSpec &getResultType() const;

    };

    class typeSpec {
    public:
        int16_t kind; // 0 is member 1 is func 2 is null
        memberExpr *member;
        funcTypeSpec *func;
        bool isNull;

        memberExpr &getMemberExpr() const;

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
        subscript *subscript;

        bool isInvocation() const;

        identifierWithTemplateArg &getId() const;

        invocationArguments &getArg() const;

        hoshi::subscript &getSubscript() const;
    };

    class memberExpr {
    public:
        vec<subscriptExpr *> terms;

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
        primary *lhs{};

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
    public:
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
        innerMethodDecl *method;

        innerMethodDecl &getMethod();

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
        constructorDecl *con;
        // method
        innerMethodDecl *method;

        identifierWithTypeSpec &getVar();

        constructorDecl &getConstructor();

        innerMethodDecl &getMethod();
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
        // constructor
        constructorDef *con;
        // method
        innerMethodDef *met;

        constructorDef &getConstructor();

        innerMethodDef &getMethod();

        bool isConstructor() const;
    };

    class implInner {
    public:
        vec<implInnerPair *> inner;

        vec<implInnerPair *> &getInner();
    };

    class implStmt {
    public:
        identifier *interfaceName;
        identifier *structName;
        implInner *inner;

        identifier &getInterfaceId();

        identifier &getStructId();

        implInner &getInner();

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
            funcDefStmt,
            interfaceDefStmt,
            structDefStmt,
            implStmt,
            letStmt,
            hoshiModule,
        } kind;

        union vValue {
            useStmt *useStmt;
            interfaceDefStmt *interfaceDefStmt;
            structDefStmt *structDefStmt;
            implStmt *implStmt;
            letStmt *letStmt;
            hoshiModule *hoshiModule;
            funcDefStmt *funcDefStmt;
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
        codeBlock *block;

        inCodeBlockStmt &getInitStmt();

        rExpr &getCond();

        inCodeBlockStmt &getAfterStmt();

        codeBlock &getBlock();
    };

    class forEachStmt {
    public:
        identifier *var;
        rExpr *container;
        codeBlock *block;

        identifier &getVar();

        rExpr &getContainer();

        codeBlock &getBlock();
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
            forStmt,
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
            forStmt *forStmt;
            void *ptr;

            template<typename T>
            vValue(T *t) : ptr((void *) t) {}
        } value;

        vKind &getKind();

        vValue &getValue();
    };

    class innerMethodDecl {
    public:
        identifier *name;
        definitionArguments *args;
        typeSpec *resultType;

        identifier &getName();

        definitionArguments &getArgs();

        typeSpec &getResultType();
    };

    class innerMethodDef {
    public:
        identifier *name;
        definitionArguments *args;
        typeSpec *resultType;
        codeBlock *block;

        identifier &getName();

        definitionArguments &getArgs();

        typeSpec &getResultType();

        codeBlock &getBlock();
    };

    class constructorDecl {
    public:
        definitionArguments *args;

        definitionArguments &getArgs();
    };

    class constructorDef {
    public:
        definitionArguments *args;
        codeBlock *block;

        definitionArguments &getArgs();

        codeBlock &getBlock();
    };

    class codeBlock {
    public:
        vec<inCodeBlockStmt *> stmts;

        vec<inCodeBlockStmt *> &getStmts();
    };

    class hoshiModule {
    public:
        vec<globalStmt *> stmts;

        vec<globalStmt *> &getStmts();
    };

    void finalizeAST(basicLiterals *ptr);

    void finalizeAST(identifier *ptr);

    void finalizeAST(typeSpec *ptr);

    void finalizeAST(identifierWithTypeSpec *ptr);

    void finalizeAST(defTemplateArgSpec *ptr);

    void finalizeAST(defTemplateArg *ptr);

    void finalizeAST(templateArgSpec *ptr);

    void finalizeAST(templateArg *ptr);

    void finalizeAST(invocationArguments *ptr);

    void finalizeAST(definitionArguments *ptr);

    void finalizeAST(funcTypeSpec *ptr);

    void finalizeAST(subscript *ptr);

    void finalizeAST(identifierWithDefTemplateArg *ptr);

    void finalizeAST(identifierWithTemplateArg *ptr);

    void finalizeAST(subscriptExpr *ptr);

    void finalizeAST(memberExpr *ptr);

    void finalizeAST(primary *ptr);

    void finalizeAST(uniqueExpr *ptr);

    void finalizeAST(mulExpr *ptr);

    void finalizeAST(addExpr *ptr);

    void finalizeAST(shiftExpr *ptr);

    void finalizeAST(relationalExpr *ptr);

    void finalizeAST(equalityExpr *ptr);

    void finalizeAST(andExpr *ptr);

    void finalizeAST(exclusiveExpr *ptr);

    void finalizeAST(inclusiveExpr *ptr);

    void finalizeAST(logicalAndExpr *ptr);

    void finalizeAST(logicalOrExpr *ptr);

    void finalizeAST(rExpr *ptr);

    void finalizeAST(codeBlock *ptr);

    void finalizeAST(useStmt *ptr);

    void finalizeAST(funcDefStmt *ptr);

    void finalizeAST(interfaceDefInner *ptr);

    void finalizeAST(interfaceDefInnerPair *ptr);

    void finalizeAST(interfaceDefStmt *ptr);

    void finalizeAST(structDefInnerPair *ptr);

    void finalizeAST(structDefInner *ptr);

    void finalizeAST(structDefStmt *ptr);

    void finalizeAST(implInnerPair *ptr);

    void finalizeAST(implInner *ptr);

    void finalizeAST(implStmt *ptr);

    void finalizeAST(letAssignmentPair *ptr);

    void finalizeAST(letStmt *ptr);

    void finalizeAST(globalStmt *ptr);

    void finalizeAST(ifStmt *ptr);

    void finalizeAST(whileStmt *ptr);

    void finalizeAST(forStmt *ptr);

    void finalizeAST(forEachStmt *ptr);

    void finalizeAST(returnStmt *ptr);

    void finalizeAST(continueStmt *ptr);

    void finalizeAST(breakStmt *ptr);

    void finalizeAST(inCodeBlockStmt *ptr);

    void finalizeAST(innerMethodDecl *ptr);

    void finalizeAST(innerMethodDef *ptr);

    void finalizeAST(constructorDecl *ptr);

    void finalizeAST(constructorDef *ptr);
} // hoshi
#endif //HOSHI_LANG_AST_HPP
#pragma clang diagnostic pop