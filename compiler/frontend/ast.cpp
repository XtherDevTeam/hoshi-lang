//
// Created by XIaokang00010 on 2023/1/24.
//

#include "ast.hpp"

namespace hoshi {
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

    identifier &defTemplateArgSpec::getImpl() const {
        return *impl;
    }

    vec<defTemplateArgSpec *> &defTemplateArg::get() {
        return spec;
    }

    typeSpec &templateArgSpec::get() const {
        return *spec;
    }

    vec<typeSpec *> &templateArg::get() {
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

    codeBlock &funcTypeSpec::getBlock() const {
        return *block;
    }

    bool typeSpec::isFuncTypeSpec() const {
        return func;
    }

    accessExpr &typeSpec::getAccessExpr() const {
        return *access;
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

    identifier &identifierWithDefTemplateArg::getId() const {
        return *id;
    }

    defTemplateArg &identifierWithDefTemplateArg::getArg() const {
        return *arg;
    }

    bool subscriptExpr::isInvocation() const {
        return arg;
    }

    identifierWithTemplateArg &subscriptExpr::getId() const {
        return *id;
    }

    invocationArguments &subscriptExpr::getArg() const {
        return *arg;
    }

    subscriptExpr &subscriptExpr::getSubscript() const {
        return *subscript;
    }

    vec<identifier *> &accessExpr::getPrefix() {
        return prefix;
    }

    identifierWithTemplateArg &accessExpr::getTerm() const {
        return *term;
    }

    vec<identifier *> &memberExpr::getPrefix() {
        return prefix;
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
        return op.kind != lexer::token::tokenKind::eof;
    }

    vec<uniqueExpr *> &mulExpr::getTerms() {
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

    identifierWithDefTemplateArg &interfaceDefInnerPair::getMethodId() {
        return *id;
    }

    definitionArguments &interfaceDefInnerPair::getMethodArgs() {
        return *args;
    }

    typeSpec &interfaceDefInnerPair::getMethodResultType() {
        return *resultType;
    }

    bool interfaceDefInnerPair::isMethod() {
        return !var;
    }

    vec<interfaceDefInnerPair *> &interfaceDefInner::getInner() {
        return inner;
    }

    identifier &interfaceDefStmt::getId() {
        return *id;
    }

    interfaceDefInner &interfaceDefStmt::getInner() {
        return *inner;
    }

    identifierWithTypeSpec &structDefInnerPair::getVar() {
        return *var;
    }

    definitionArguments &structDefInnerPair::getConArgs() {
        return *conArgs;
    }

    identifierWithDefTemplateArg &structDefInnerPair::getMethodId() {
        return *methodId;
    }

    definitionArguments &structDefInnerPair::getMethodArgs() {
        return *methodArgs;
    }

    typeSpec &structDefInnerPair::getMethodResultType() {
        return *methodResultType;
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

    identifierWithTypeSpec &implInnerPair::getVar() {
        return *var;
    }

    definitionArguments &implInnerPair::getConArgs() {
        return *conArgs;
    }

    codeBlock &implInnerPair::getConBlock() {
        return *conBlock;
    }

    identifierWithDefTemplateArg &implInnerPair::getMethodId() {
        return *methodId;
    }

    definitionArguments &implInnerPair::getMethodArgs() {
        return *methodArgs;
    }

    typeSpec &implInnerPair::getMethodResultType() {
        return *methodResultType;
    }

    codeBlock &implInnerPair::getMethodBlock() {
        return *methodBlock;
    }

    vec<implInnerPair *> &implInner::getInner() {
        return inner;
    }

    identifierWithDefTemplateArg &implStmt::getInterfaceId() {
        return *interfaceName;
    }

    identifierWithDefTemplateArg &implStmt::getStructId() {
        return *structName;
    }

    bool implStmt::isImplForStmt() {
        return interfaceName;
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

    identifier &forEachStmt::getVar() {
        return *var;
    }

    rExpr &forEachStmt::getContainer() {
        return *container;
    }

    rExpr &returnStmt::getValue() {
        return *value;
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
} // hoshi