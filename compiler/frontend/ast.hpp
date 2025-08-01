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

namespace yoi {
    class AST {
        lexer::token token;
    public:
        AST();

        AST(lexer::token token);

        const std::tuple<yoi::indexT, yoi::indexT> &getLocation();

        yoi::indexT getLine();

        yoi::indexT getColumn();
    };
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

    class leftExpr;

    class externModuleAccessExpression;

    class basicLiterals : public AST {
    public:
        lexer::token node;

        lexer::token &get();
    };

    class identifier : public AST {
    public:
        lexer::token node;

        lexer::token &get();
    };

    class identifierWithTypeSpec : public AST {
    public:
        identifier *id;
        typeSpec *spec;

        identifier &getId() const;

        typeSpec &getSpec() const;
    };

    class defTemplateArgSpec : public AST {
    public:
        identifier *id;
        externModuleAccessExpression *impl;

        identifier &getId() const;

        externModuleAccessExpression &getImpl() const;
    };

    class defTemplateArg : public AST {
    public:
        vec<defTemplateArgSpec *> spec;

        vec<defTemplateArgSpec *> &get();
    };

    class templateArgSpec : public AST {
    public:
        typeSpec *spec;

        typeSpec &get() const;
    };

    class templateArg : public AST {
    public:
        vec<templateArgSpec *> spec;

        vec<templateArgSpec *> &get();
    };

    class invocationArguments : public AST {
    public:
        vec<rExpr *> arg;

        vec<rExpr *> &get();
    };

    class definitionArguments : public AST {
    public:
        vec<identifierWithTypeSpec *> spec;

        vec<identifierWithTypeSpec *> &get();
    };

    class funcTypeSpec : public AST {
    public:
        definitionArguments *args;
        typeSpec *resultType;

        definitionArguments &getArgs() const;

        typeSpec &getResultType() const;

    };

    class typeSpec : public AST {
    public:
        int16_t kind; // 0 is member 1 is func 2 is null
        externModuleAccessExpression *member;
        funcTypeSpec *func;
        bool isNull;

        externModuleAccessExpression &getMemberExpr() const;

        funcTypeSpec &getTypeSpec() const;
    };

    class subscript : public AST {
    public:
        rExpr *expr;

        rExpr &getExpr() const;
    };

    class identifierWithTemplateArg : public AST {
    public:
        identifier *id;
        templateArg *arg;

        identifier &getId() const;

        templateArg &getArg() const;

        bool hasTemplateArg() const;
    };

    class identifierWithDefTemplateArg : public AST {
    public:
        identifier *id;
        defTemplateArg *arg;

        identifier &getId() const;

        defTemplateArg &getArg() const;

        bool hasDefTemplateArg() const;
    };

    class subscriptExpr : public AST {
    public:
        identifierWithTemplateArg *id;
        invocationArguments *args;
        subscript *subscriptVal;

        bool isInvocation() const;

        bool isSubscript() const;

        bool isIdentifier() const;

        identifierWithTemplateArg &getId() const;

        invocationArguments &getArg() const;

        yoi::subscript &getSubscript() const;
    };

    class memberExpr : public AST {
    public:
        vec<subscriptExpr *> terms;

        vec<subscriptExpr *> &getTerms();
    };

    class primary : public AST {
    public:
        int8_t kind; // 0 is memberExpr 1 is basicLiterals 2 is rExpr
        memberExpr *member;
        basicLiterals *literals;
        rExpr *expr;

        memberExpr &getMemberExpr() const;

        basicLiterals &getLiterals() const;

        rExpr &getExpr() const;
    };

    class uniqueExpr : public AST {
    public:
        lexer::token op;
        primary *lhs{};

        lexer::token &getOp();

        primary &getLhs() const;

        operator bool() const;
    };

    class leftExpr : public AST {
    public:
        lexer::token op;
        uniqueExpr *lhs;
        rExpr *rhs;

        lexer::token &getOp();

        uniqueExpr &getLhs() const;

        rExpr &getRhs() const;

        bool hasRhs() const;
    };

    class mulExpr : public AST {
    public:
        vec<leftExpr *> terms;
        vec<lexer::token> ops;

        vec<leftExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class addExpr : public AST {
    public:
        vec<mulExpr *> terms;
        vec<lexer::token> ops;

        vec<mulExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class shiftExpr : public AST {
    public:
        vec<addExpr *> terms;
        vec<lexer::token> ops;

        vec<addExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class relationalExpr : public AST {
    public:
        vec<shiftExpr *> terms;
        vec<lexer::token> ops;

        vec<shiftExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class equalityExpr : public AST {
    public:
        vec<relationalExpr *> terms;
        vec<lexer::token> ops;

        vec<relationalExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class andExpr : public AST {
    public:
        vec<equalityExpr *> terms;
        vec<lexer::token> ops;

        vec<equalityExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class exclusiveExpr : public AST {
    public:
        vec<andExpr *> terms;
        vec<lexer::token> ops;

        vec<andExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class inclusiveExpr : public AST {
    public:
        vec<exclusiveExpr *> terms;
        vec<lexer::token> ops;

        vec<exclusiveExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class logicalAndExpr : public AST {
    public:
        vec<inclusiveExpr *> terms;
        vec<lexer::token> ops;

        vec<inclusiveExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class logicalOrExpr : public AST {
    public:
        vec<logicalAndExpr *> terms;
        vec<lexer::token> ops;

        vec<logicalAndExpr *> &getTerms();

        vec<lexer::token> &getOp();

        operator bool() const;
    };

    class rExpr : public AST {
    public:
        logicalOrExpr *expr;

        logicalOrExpr &getExpr() const;
    };

    class useStmt : public AST {
    public:
        identifier *name;
        lexer::token path;

        identifier &getName();

        lexer::token &getPath();
    };

    class funcDefStmt : public AST {
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

    class interfaceDefInnerPair : public AST {
    public:
        // member var
        identifierWithTypeSpec *var;

        identifierWithTypeSpec &getVar();

        // method
        innerMethodDecl *method;

        innerMethodDecl &getMethod();

        bool isMethod();
    };

    class interfaceDefInner : public AST {
    public:
        vec<interfaceDefInnerPair *> inner;

        vec<interfaceDefInnerPair *> &getInner();
    };

    class interfaceDefStmt : public AST {
    public:
        identifierWithDefTemplateArg *id;
        interfaceDefInner *inner;

        identifierWithDefTemplateArg &getId();

        interfaceDefInner &getInner();
    };

    class structDefInnerPair : public AST {
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

    class structDefInner : public AST {
    public:
        vec<structDefInnerPair *> inner;

        vec<structDefInnerPair *> &getInner();
    };

    class structDefStmt : public AST {
    public:
        identifierWithDefTemplateArg *id;
        structDefInner *inner;

        identifierWithDefTemplateArg &getId();

        structDefInner &getInner();
    };

    class implInnerPair : public AST {
    public:
        // constructor
        constructorDef *con;
        // method
        innerMethodDef *met;

        constructorDef &getConstructor();

        innerMethodDef &getMethod();

        bool isConstructor() const;
    };

    class implInner : public AST {
    public:
        vec<implInnerPair *> inner;

        vec<implInnerPair *> &getInner();
    };

    class implStmt : public AST {
    public:
        externModuleAccessExpression *interfaceName;
        identifierWithDefTemplateArg *structName;
        implInner *inner;

        externModuleAccessExpression &getInterfaceId();

        identifierWithDefTemplateArg &getStructId();

        implInner &getInner();

        bool isImplForStmt();
    };

    class letAssignmentPair : public AST {
    public:
        identifier *lhs;
        rExpr *rhs;

        identifier &getLhs();

        rExpr &getRhs();
    };

    class letStmt : public AST {
    public:
        vec<letAssignmentPair *> terms;

        vec<letAssignmentPair *> &getTerms();
    };

    class globalStmt : public AST {
    public:
        enum class vKind : int16_t {
            useStmt,
            funcDefStmt,
            interfaceDefStmt,
            structDefStmt,
            implStmt,
            letStmt,
        } kind;

        union vValue {
            useStmt *useStmtVal;
            interfaceDefStmt *interfaceDefStmtVal;
            structDefStmt *structDefStmtVal;
            implStmt *implStmtVal;
            letStmt *letStmtVal;
            funcDefStmt *funcDefStmtVal;
            void *ptr;

            template<typename T>
            vValue(T *t) : ptr(static_cast<void *>(t)) {}
        } value;

        vKind &getKind();

        vValue &getValue();
    };

    class ifStmt : public AST {
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

        bool hasElseBlock() const;
    };

    class whileStmt : public AST {
    public:
        rExpr *cond;
        codeBlock *block;

        rExpr &getCond();

        codeBlock &getBlock();
    };

    class forStmt : public AST {
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

    class forEachStmt : public AST {
    public:
        identifier *var;
        rExpr *container;
        codeBlock *block;

        identifier &getVar();

        rExpr &getContainer();

        codeBlock &getBlock();
    };

    class returnStmt : public AST {
    public:
        rExpr *value;

        rExpr &getValue();

        bool hasValue() const;
    };

    class continueStmt : public AST {
    };

    class breakStmt : public AST {
    };

    class inCodeBlockStmt : public AST {
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
            ifStmt *ifStmtVal;
            whileStmt *whileStmtVal;
            forEachStmt *forEachStmtVal;
            returnStmt *returnStmtVal;
            continueStmt *continueStmtVal;
            breakStmt *breakStmtVal;
            letStmt *letStmtVal;
            codeBlock *codeBlockVal;
            rExpr *rExprVal;
            forStmt *forStmtVal;
            void *ptr;

            template<typename T>
            vValue(T *t) : ptr((void *) t) {}
        } value;

        vKind &getKind();

        vValue &getValue();
    };

    class innerMethodDecl : public AST {
    public:
        identifier *name;
        definitionArguments *args;
        typeSpec *resultType;

        identifier &getName();

        definitionArguments &getArgs();

        typeSpec &getResultType();
    };

    class innerMethodDef : public AST {
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

    class constructorDecl : public AST {
    public:
        definitionArguments *args;

        definitionArguments &getArgs();
    };

    class constructorDef : public AST {
    public:
        definitionArguments *args;
        codeBlock *block;

        definitionArguments &getArgs();

        codeBlock &getBlock();
    };

    class codeBlock : public AST {
    public:
        vec<inCodeBlockStmt *> stmts;

        vec<inCodeBlockStmt *> &getStmts();
    };

    class hoshiModule : public AST {
    public:
        vec<globalStmt *> stmts;

        vec<globalStmt *> &getStmts();
    };

    class externModuleAccessExpression : public AST {
        public:
            vec<identifierWithTemplateArg *> terms;

            vec<identifierWithTemplateArg *> &getTerms();

            bool isIdentifier() const;
    };

    void finalizeAST(externModuleAccessExpression *ptr);

    void finalizeAST(hoshiModule *ptr);

    void finalizeAST(leftExpr *ptr);

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