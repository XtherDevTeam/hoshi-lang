//
// Created by XIaokang00010 on 2023/1/24.
//

#include "ast.hpp"

namespace yoi {
    lexer::token &basicLiterals::get() {
        return node;
    }

    lexer::token &identifier::get() {
        return node;
    }

    identifier &identifierWithTypeSpec::getId() const {
        return *id;
    }

    typeSpec &identifierWithTypeSpec::getSpec() const {
        return *spec;
    }

    identifier &defTemplateArgSpec::getId() const {
        return *id;
    }

    externModuleAccessExpression &defTemplateArgSpec::getImpl() const {
        return *impl;
    }

    vec<defTemplateArgSpec *> &defTemplateArg::get() {
        return spec;
    }

    typeSpec &templateArgSpec::get() const {
        return *spec;
    }

    vec<templateArgSpec *> &templateArg::get() {
        return spec;
    }

    vec<rExpr *> &invocationArguments::get() {
        return arg;
    }

    vec<identifierWithTypeSpec *> &definitionArguments::get() {
        return spec;
    }

    definitionArguments &funcTypeSpec::getArgs() const {
        return *args;
    }

    typeSpec &funcTypeSpec::getResultType() const {
        return *resultType;
    }

    externModuleAccessExpression &typeSpec::getMemberExpr() const {
        return *member;
    }

    funcTypeSpec &typeSpec::getTypeSpec() const {
        return *func;
    }

    rExpr &subscript::getExpr() const {
        return *expr;
    }

    identifier &identifierWithTemplateArg::getId() const {
        return *id;
    }

    templateArg &identifierWithTemplateArg::getArg() const {
        return *arg;
    }

    bool identifierWithTemplateArg::hasTemplateArg() const {
        return arg;
    }

    identifier &identifierWithDefTemplateArg::getId() const {
        return *id;
    }

    defTemplateArg &identifierWithDefTemplateArg::getArg() const {
        return *arg;
    }

    bool identifierWithDefTemplateArg::hasDefTemplateArg() const {
        return arg;
    }

    bool subscriptExpr::isIdentifier() const {
        return id;
    }

    vec<subscriptExpr *> &memberExpr::getTerms() {
        return terms;
    }

    memberExpr &primary::getMemberExpr() const {
        return *member;
    }

    basicLiterals &primary::getLiterals() const {
        return *literals;
    }

    rExpr &primary::getExpr() const {
        return *expr;
    }

    lexer::token &uniqueExpr::getOp() {
        return op;
    }

    primary &uniqueExpr::getLhs() const {
        return *lhs;
    }

    uniqueExpr::operator bool() const {
        return op.kind != lexer::token::tokenKind::unknown;
    }

    lexer::token & leftExpr::getOp() {
        return op;
    }

    uniqueExpr & leftExpr::getLhs() const {
        return *lhs;
    }

    rExpr & leftExpr::getRhs() const {
        return *rhs;
    }

    bool leftExpr::hasRhs() const {
        return rhs != nullptr;
    }

    vec<leftExpr *> &mulExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &mulExpr::getOp() {
        return ops;
    }

    mulExpr::operator bool() const {
        return !ops.empty();
    }

    vec<mulExpr *> &addExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &addExpr::getOp() {
        return ops;
    }

    addExpr::operator bool() const {
        return !ops.empty();
    }

    vec<addExpr *> &shiftExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &shiftExpr::getOp() {
        return ops;
    }

    shiftExpr::operator bool() const {
        return !ops.empty();
    }

    vec<shiftExpr *> &relationalExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &relationalExpr::getOp() {
        return ops;
    }

    relationalExpr::operator bool() const {
        return !ops.empty();
    }

    vec<relationalExpr *> &equalityExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &equalityExpr::getOp() {
        return ops;
    }

    equalityExpr::operator bool() const {
        return !ops.empty();
    }

    vec<equalityExpr *> &andExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &andExpr::getOp() {
        return ops;
    }

    andExpr::operator bool() const {
        return !ops.empty();
    }

    vec<andExpr *> &exclusiveExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &exclusiveExpr::getOp() {
        return ops;
    }

    exclusiveExpr::operator bool() const {
        return !ops.empty();
    }

    vec<exclusiveExpr *> &inclusiveExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &inclusiveExpr::getOp() {
        return ops;
    }

    inclusiveExpr::operator bool() const {
        return !ops.empty();
    }

    vec<inclusiveExpr *> &logicalAndExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &logicalAndExpr::getOp() {
        return ops;
    }

    logicalAndExpr::operator bool() const {
        return !ops.empty();
    }

    vec<logicalAndExpr *> &logicalOrExpr::getTerms() {
        return terms;
    }

    vec<lexer::token> &logicalOrExpr::getOp() {
        return ops;
    }

    logicalOrExpr::operator bool() const {
        return !ops.empty();
    }

    logicalOrExpr &rExpr::getExpr() const {
        return *expr;
    }

    identifier &useStmt::getName() {
        return *name;
    }

    lexer::token &useStmt::getPath() {
        return path;
    }

    identifierWithDefTemplateArg &funcDefStmt::getId() {
        return *id;
    }

    definitionArguments &funcDefStmt::getArgs() {
        return *args;
    }

    typeSpec &funcDefStmt::getResultType() {
        return *resultType;
    }

    codeBlock &funcDefStmt::getBlock() {
        return *block;
    }

    identifierWithTypeSpec &interfaceDefInnerPair::getVar() {
        return *var;
    }

    bool interfaceDefInnerPair::isMethod() {
        return !var;
    }

    innerMethodDecl &interfaceDefInnerPair::getMethod() {
        return *method;
    }

    vec<interfaceDefInnerPair *> &interfaceDefInner::getInner() {
        return inner;
    }

    identifierWithDefTemplateArg &interfaceDefStmt::getId() {
        return *id;
    }

    interfaceDefInner &interfaceDefStmt::getInner() {
        return *inner;
    }

    identifierWithTypeSpec &structDefInnerPair::getVar() {
        return *var;
    }

    constructorDecl &structDefInnerPair::getConstructor() {
        return *con;
    }

    innerMethodDecl &structDefInnerPair::getMethod() {
        return *method;
    }

    vec<structDefInnerPair *> &structDefInner::getInner() {
        return inner;
    }

    identifierWithDefTemplateArg &structDefStmt::getId() {
        return *id;
    }

    structDefInner &structDefStmt::getInner() {
        return *inner;
    }

    constructorDef &implInnerPair::getConstructor() {
        return *con;
    }

    innerMethodDef &implInnerPair::getMethod() {
        return *met;
    }

    bool implInnerPair::isConstructor() const {
        return con;
    }

    vec<implInnerPair *> &implInner::getInner() {
        return inner;
    }

    externModuleAccessExpression &implStmt::getInterfaceId() {
        return *interfaceName;
    }

    identifierWithTemplateArg &implStmt::getStructId() {
        return *structName;
    }

    bool implStmt::isImplForStmt() {
        return interfaceName;
    }

    implInner &implStmt::getInner() {
        return *inner;
    }

    identifier &letAssignmentPair::getLhs() {
        return *lhs;
    }

    rExpr &letAssignmentPair::getRhs() {
        return *rhs;
    }

    vec<letAssignmentPair *> &letStmt::getTerms() {
        return terms;
    }

    globalStmt::vKind &globalStmt::getKind() {
        return kind;
    }

    globalStmt::vValue &globalStmt::getValue() {
        return value;
    }

    rExpr &ifStmt::ifBlock::getCond() {
        return *cond;
    }

    codeBlock &ifStmt::ifBlock::getBlock() {
        return *block;
    }

    ifStmt::ifBlock &ifStmt::getIfBlock() {
        return ifB;
    }

    vec<ifStmt::ifBlock> &ifStmt::getElifBlock() {
        return elifB;
    }

    codeBlock &ifStmt::getElseBlock() {
        return *elseB;
    }

    bool ifStmt::hasElseBlock() const {
        return elseB;
    }

    rExpr &whileStmt::getCond() {
        return *cond;
    }

    codeBlock &whileStmt::getBlock() {
        return *block;
    }

    inCodeBlockStmt &forStmt::getInitStmt() {
        return *initStmt;
    }

    rExpr &forStmt::getCond() {
        return *cond;
    }

    inCodeBlockStmt &forStmt::getAfterStmt() {
        return *afterStmt;
    }

    codeBlock &forStmt::getBlock() {
        return *block;
    }

    identifier &forEachStmt::getVar() {
        return *var;
    }

    rExpr &forEachStmt::getContainer() {
        return *container;
    }

    codeBlock &forEachStmt::getBlock() {
        return *block;
    }

    rExpr &returnStmt::getValue() {
        return *value;
    }

    bool returnStmt::hasValue() const {
        return value;
    }

    inCodeBlockStmt::vKind &inCodeBlockStmt::getKind() {
        return kind;
    }

    inCodeBlockStmt::vValue &inCodeBlockStmt::getValue() {
        return value;
    }

    vec<inCodeBlockStmt *> &codeBlock::getStmts() {
        return stmts;
    }

    void finalizeAST(funcTypeSpec *ptr) {
        finalizeAST(ptr->args);
        finalizeAST(ptr->resultType);
        delete ptr;
    }

    void finalizeAST(definitionArguments *ptr) {
        for (auto &i: ptr->get())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(invocationArguments *ptr) {
        for (auto &i: ptr->get())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(templateArg *ptr) {
        for (auto &i: ptr->get())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(templateArgSpec *ptr) {
        finalizeAST(ptr->spec);
        delete ptr;
    }

    void finalizeAST(defTemplateArg *ptr) {
        for (auto &i: ptr->get())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(defTemplateArgSpec *ptr) {
        finalizeAST(ptr->id);
        if(ptr->impl) finalizeAST(ptr->impl);
        delete ptr;
    }

    void finalizeAST(identifierWithTypeSpec *ptr) {
        finalizeAST(ptr->id);
        finalizeAST(ptr->spec);
        delete ptr;
    }

    void finalizeAST(typeSpec *ptr) {
        switch (ptr->kind) {
            case 0:
                finalizeAST(ptr->member);
                break;
            case 1:
                finalizeAST(ptr->func);
                break;
            case 2:
                break;
        }
        delete ptr;
    }

    void finalizeAST(identifier *ptr) {
        delete ptr;
    }

    void finalizeAST(basicLiterals *ptr) {
        delete ptr;
    }

    void finalizeAST(identifierWithDefTemplateArg *ptr) {
        finalizeAST(ptr->id);
        if (ptr->hasDefTemplateArg())
            finalizeAST(ptr->arg);
        delete ptr;
    }

    void finalizeAST(identifierWithTemplateArg *ptr) {
        finalizeAST(ptr->id);
        if (ptr->hasTemplateArg())
            finalizeAST(ptr->arg);
        delete ptr;
    }

    void finalizeAST(subscriptExpr *ptr) {
        finalizeAST(ptr->id);
        for (auto &i: ptr->subscriptVal)
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(memberExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
    }

    void finalizeAST(primary *ptr) {
        switch (ptr->kind) {
            case 0:
                finalizeAST(ptr->member);
                break;
            case 1:
                finalizeAST(ptr->literals);
                break;
            case 2:
                finalizeAST(ptr->expr);
                break;
        }
        delete ptr;
    }

    void finalizeAST(uniqueExpr *ptr) {
        finalizeAST(ptr->lhs);
        delete ptr;
    }

    void finalizeAST(mulExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(addExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(shiftExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(relationalExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(equalityExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(andExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(exclusiveExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(inclusiveExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(logicalAndExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(logicalOrExpr *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(rExpr *ptr) {
        finalizeAST(ptr->expr);
        delete ptr;
    }

    void finalizeAST(codeBlock *ptr) {
        for (auto &i: ptr->getStmts())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(useStmt *ptr) {
        finalizeAST(ptr->name);
        delete ptr;
    }

    void finalizeAST(funcDefStmt *ptr) {
        finalizeAST(ptr->id);
        finalizeAST(ptr->resultType);
        finalizeAST(ptr->args);
        finalizeAST(ptr->block);
        delete ptr;
    }

    void finalizeAST(interfaceDefInner *ptr) {
        for (auto &i: ptr->getInner())
            finalizeAST(i);

        delete ptr;
    }

    void finalizeAST(interfaceDefInnerPair *ptr) {
        if (ptr->isMethod()) {
            finalizeAST(ptr->method);
        } else {
            finalizeAST(ptr->var);
        }
        delete ptr;
    }

    void finalizeAST(interfaceDefStmt *ptr) {
        finalizeAST(ptr->id);
        finalizeAST(ptr->inner);
        delete ptr;
    }

    void finalizeAST(structDefInnerPair *ptr) {
        switch (ptr->kind) {
            case 0:
                finalizeAST(ptr->var);
                break;
            case 1:
                finalizeAST(ptr->con);
                break;
            case 2:
                finalizeAST(ptr->method);
                break;
        }
        delete ptr;
    }

    void finalizeAST(structDefInner *ptr) {
        for (auto &i: ptr->getInner())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(structDefStmt *ptr) {
        finalizeAST(ptr->id);
        finalizeAST(ptr->inner);
        delete ptr;
    }

    void finalizeAST(implInnerPair *ptr) {
        if (ptr->isConstructor()) {
            finalizeAST(ptr->con);
        } else {
            finalizeAST(ptr->met);
        }
        delete ptr;
    }

    void finalizeAST(implInner *ptr) {
        for (auto &i: ptr->getInner())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(implStmt *ptr) {
        if (ptr->isImplForStmt())
            finalizeAST(ptr->interfaceName);
        finalizeAST(ptr->structName);
        finalizeAST(ptr->inner);
        delete ptr;
    }

    void finalizeAST(letAssignmentPair *ptr) {
        finalizeAST(ptr->lhs);
        finalizeAST(ptr->rhs);
        delete ptr;
    }

    void finalizeAST(letStmt *ptr) {
        for (auto &i: ptr->getTerms())
            finalizeAST(i);
        delete ptr;
    }

    void finalizeAST(globalStmt *ptr) {
        switch (ptr->kind) {
            case globalStmt::vKind::useStmt:
                finalizeAST(ptr->value.useStmtVal);
                break;
            case globalStmt::vKind::interfaceDefStmt:
                finalizeAST(ptr->value.interfaceDefStmtVal);
                break;
            case globalStmt::vKind::structDefStmt:
                finalizeAST(ptr->value.structDefStmtVal);
                break;
            case globalStmt::vKind::implStmt:
                finalizeAST(ptr->value.implStmtVal);
                break;
            case globalStmt::vKind::letStmt:
                finalizeAST(ptr->value.letStmtVal);
                break;
        }
        delete ptr;
    }

    void finalizeAST(ifStmt *ptr) {
        finalizeAST(ptr->ifB.cond);
        finalizeAST(ptr->ifB.block);
        for (auto &i: ptr->elifB) {
            finalizeAST(i.cond);
            finalizeAST(i.block);
        }
        if (ptr->elseB)
            finalizeAST(ptr->elseB);
        delete ptr;
    }

    void finalizeAST(whileStmt *ptr) {
        finalizeAST(ptr->cond);
        finalizeAST(ptr->block);
        delete ptr;
    }

    void finalizeAST(forStmt *ptr) {
        finalizeAST(ptr->initStmt);
        finalizeAST(ptr->cond);
        finalizeAST(ptr->afterStmt);
        finalizeAST(ptr->block);
        delete ptr;
    }

    void finalizeAST(forEachStmt *ptr) {
        finalizeAST(ptr->var);
        finalizeAST(ptr->container);
        finalizeAST(ptr->block);
        delete ptr;
    }

    void finalizeAST(returnStmt *ptr) {
        if (ptr->hasValue())
            finalizeAST(ptr->value);
        delete ptr;
    }

    void finalizeAST(continueStmt *ptr) {
        delete ptr;
    }

    void finalizeAST(breakStmt *ptr) {
        delete ptr;
    }

    void finalizeAST(inCodeBlockStmt *ptr) {
        switch (ptr->kind) {
            case inCodeBlockStmt::vKind::ifStmt:
                finalizeAST(ptr->value.ifStmtVal);
                break;
            case inCodeBlockStmt::vKind::whileStmt:
                finalizeAST(ptr->value.whileStmtVal);
                break;
            case inCodeBlockStmt::vKind::forEachStmt:
                finalizeAST(ptr->value.forStmtVal);
                break;
            case inCodeBlockStmt::vKind::returnStmt:
                finalizeAST(ptr->value.returnStmtVal);
                break;
            case inCodeBlockStmt::vKind::continueStmt:
                finalizeAST(ptr->value.continueStmtVal);
                break;
            case inCodeBlockStmt::vKind::breakStmt:
                finalizeAST(ptr->value.breakStmtVal);
                break;
            case inCodeBlockStmt::vKind::letStmt:
                finalizeAST(ptr->value.letStmtVal);
                break;
            case inCodeBlockStmt::vKind::codeBlock:
                finalizeAST(ptr->value.codeBlockVal);
                break;
            case inCodeBlockStmt::vKind::rExpr:
                finalizeAST(ptr->value.rExprVal);
                break;
        }
        delete ptr;
    }

    void finalizeAST(innerMethodDecl *ptr) {
        finalizeAST(ptr->resultType);
        finalizeAST(ptr->name);
        finalizeAST(ptr->args);
        delete ptr;
    }

    void finalizeAST(innerMethodDef *ptr) {
        finalizeAST(ptr->args);
        finalizeAST(ptr->name);
        finalizeAST(ptr->resultType);
        finalizeAST(ptr->block);
        delete ptr;
    }

    void finalizeAST(constructorDecl *ptr) {
        finalizeAST(ptr->args);
        delete ptr;
    }

    void finalizeAST(constructorDef *ptr) {
        finalizeAST(ptr->args);
        finalizeAST(ptr->block);
        delete ptr;
    }

    identifier &innerMethodDecl::getName() {
        return *name;
    }

    definitionArguments &innerMethodDecl::getArgs() {
        return *args;
    }

    typeSpec &innerMethodDecl::getResultType() {
        return *resultType;
    }

    identifier &innerMethodDef::getName() {
        return *name;
    }

    definitionArguments &innerMethodDef::getArgs() {
        return *args;
    }

    typeSpec &innerMethodDef::getResultType() {
        return *resultType;
    }

    codeBlock &innerMethodDef::getBlock() {
        return *block;
    }

    definitionArguments &constructorDecl::getArgs() {
        return *args;
    }

    definitionArguments &constructorDef::getArgs() {
        return *args;
    }

    codeBlock &constructorDef::getBlock() {
        return *block;
    }

    vec<globalStmt *> &hoshiModule::getStmts() {
        return stmts;
    }

    vec<identifierWithTemplateArg *> &externModuleAccessExpression::getTerms() {
        return terms;
    }

    bool externModuleAccessExpression::isIdentifier() const {
        return terms.size() == 1;
    }

    void finalizeAST(externModuleAccessExpression *ptr) {
        for (auto &term : ptr->getTerms()) {
            finalizeAST(term);
        }
        delete ptr;
    }

    void finalizeAST(hoshiModule *ptr) {
        for (auto stmt : ptr->getStmts()) {
            finalizeAST(stmt);
        }
        delete ptr;
    }

    void finalizeAST(leftExpr *ptr) {
        if (ptr->hasRhs()) {
            finalizeAST(ptr->rhs);
        }
        finalizeAST(ptr->lhs);
        delete ptr;
    }

    const std::tuple<yoi::indexT, yoi::indexT> &AST::getLocation() {
        return {token.line, token.col};
    }

    AST::AST() : token() {}

    AST::AST(lexer::token token) : token(std::move(token)) {}

    yoi::indexT AST::getColumn() {
        return token.col;
    }

    yoi::indexT AST::getLine() {
        return token.line;
    }

    void finalizeAST(exportDecl *ptr) {
        if (ptr->from) {
            finalizeAST(ptr->from);
        }
        if (ptr->as) {
            finalizeAST(ptr->as);
        }
        delete ptr;
    }

    void finalizeAST(importDecl *ptr) {
        if (ptr->inner) {
            finalizeAST(ptr->inner);
        }
        delete ptr;
    }

    void finalizeAST(subscript *ptr) {
        if (ptr->isSubscript()) {
            finalizeAST(ptr->expr);
        } else if (ptr->isInvocation()) {
            finalizeAST(ptr->args);
        }
        delete ptr;
    }

    bool subscript::isSubscript() const {
        return expr;
    }
    bool subscript::isInvocation() const {
        return args;
    }
    vec<subscript *> &subscriptExpr::getSubscript() {
        return subscriptVal;
    }
} // namespace yoi