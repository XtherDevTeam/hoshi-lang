//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_VISITOR_H
#define HOSHI_LANG_VISITOR_H

#include "compiler/moduleContext.h"

namespace yoi {

    class visitor {
    public:
        std::shared_ptr<moduleContext> moduleContext;
        std::shared_ptr<yoi::IRModule> irModule;

        visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext);

        std::shared_ptr<yoi::IRModule> visit();

        void visit(yoi::hoshiModule *module);

        yoi::IROperand visit(yoi::subscript *subscript);

        yoi::IROperand visit(yoi::basicLiterals *basicLiterals);

        yoi::IROperand visit(yoi::identifier *identifier);

        void visit(yoi::innerMethodDef *innerMethodDef);

        void visit(yoi::constructorDef *constructorDef);

        void visit(yoi::innerMethodDecl *innerMethodDecl);

        void visit(yoi::constructorDecl *constructorDecl);

        void visit(yoi::identifierWithTypeSpec *identifierWithTypeSpec);

        void visit(yoi::defTemplateArgSpec *defTemplateArgSpec);

        void visit(yoi::defTemplateArg *defTemplateArg);

        void visit(yoi::templateArgSpec *templateArgSpec);

        void visit(yoi::templateArg *templateArg);

        void visit(yoi::invocationArguments *invocationArguments);

        void visit(yoi::definitionArguments *definitionArguments);

        void visit(yoi::funcTypeSpec *funcTypeSpec);

        void visit(yoi::typeSpec *typeSpec);

        IROperand visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        void visit(yoi::identifierWithDefTemplateArg *identifierWithDefTemplateArg);

        yoi::IROperand visit(yoi::subscriptExpr *subscriptExpr);

        yoi::IROperand visit(yoi::memberExpr *memberExpr);

        yoi::IROperand visit(yoi::primary *primary);

        yoi::IROperand visit(yoi::uniqueExpr *uniqueExpr);

        yoi::IROperand visit(yoi::mulExpr *mulExpr);

        yoi::IROperand visit(yoi::addExpr *addExpr);

        yoi::IROperand visit(yoi::shiftExpr *shiftExpr);

        yoi::IROperand visit(yoi::relationalExpr *relationalExpr);

        yoi::IROperand visit(yoi::equalityExpr *equalityExpr);

        yoi::IROperand visit(yoi::andExpr *andExpr);

        yoi::IROperand visit(yoi::exclusiveExpr *exclusiveExpr);

        yoi::IROperand visit(yoi::inclusiveExpr *inclusiveExpr);

        yoi::IROperand visit(yoi::logicalAndExpr *logicalAndExpr);

        yoi::IROperand visit(yoi::logicalOrExpr *logicalOrExpr);

        yoi::IROperand visit(yoi::rExpr *rExpr);

        void visit(yoi::codeBlock *codeBlock);

        yoi::IROperand visit(yoi::useStmt *useStmt);

        yoi::IROperand visit(yoi::funcDefStmt *funcDefStmt);

        yoi::IROperand visit(yoi::interfaceDefInnerPair *interfaceDefInnerPair);

        yoi::IROperand visit(yoi::interfaceDefInner *interfaceDefInner);

        yoi::IROperand visit(yoi::interfaceDefStmt *interfaceDefStmt);

        yoi::IROperand visit(yoi::structDefInnerPair *structDefInnerPair);

        yoi::IROperand visit(yoi::structDefInner *structDefInner);

        yoi::IROperand visit(yoi::structDefStmt *structDefStmt);

        yoi::IROperand visit(yoi::implInnerPair *implInnerPair);

        yoi::IROperand visit(yoi::implInner *implInner);

        yoi::IROperand visit(yoi::implStmt *implStmt);

        yoi::IROperand visit(yoi::letAssignmentPair *letAssignmentPair);

        yoi::IROperand visit(yoi::letStmt *letStmt);

        yoi::IROperand visit(yoi::globalStmt *globalStmt);

        yoi::IROperand visit(yoi::ifStmt *ifStmt);

        yoi::IROperand visit(yoi::whileStmt *whileStmt);

        yoi::IROperand visit(yoi::forStmt *forStmt);

        yoi::IROperand visit(yoi::forEachStmt *forEachStmt);

        yoi::IROperand visit(yoi::returnStmt *returnStmt);

        yoi::IROperand visit(yoi::continueStmt *continueStmt);

        yoi::IROperand visit(yoi::breakStmt *breakStmt);

        void visit(yoi::inCodeBlockStmt *inCodeBlockStmt);
    };

} // yoi

#endif //HOSHI_LANG_VISITOR_H
