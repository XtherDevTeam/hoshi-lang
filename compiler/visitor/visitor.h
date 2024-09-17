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

        /**
         * @brief check if the subscript expression is a module name
         * @param it The subscript expression being checked
         * @param currentModule The current module being checked, -1 if not in a module
         * @return -1 if not a module name, otherwise the index of the module in the module table
         */
        yoi::indexT isModuleName(subscriptExpr *it, yoi::indexT currentModule) const;

        /**
         * @brief search and return extern entry by identifier in target module
         * @param moduleIndex the index of target module
         * @param identifier the identifier to search
         * @return the extern entry
         * @throw std::runtime_error if identifier not found
         */
        yoi::IRExternEntry getExternEntry(yoi::indexT moduleIndex, yoi::identifier *identifier) const;

        /**
         * Add an extern entry to the module if it does not exist.
         * @param moduleIndex the index of module begin imported
         * @param identifier the identifier of the extern entry
         * @return the index of the extern entry in the module's extern table
         * @throws std::runtime_error if the identifier is not found in the module
         */
        yoi::indexT addExternEntryIfNotExists(yoi::indexT moduleIndex, yoi::identifier *identifier);

        void visit(yoi::hoshiModule *module);

        yoi::IROperand visit(yoi::subscript *subscript);

        yoi::IROperand visit(yoi::basicLiterals *basicLiterals);

        yoi::IROperand visit(yoi::identifier *identifier);

        yoi::IROperand visitExtern(yoi::identifier *identifier, yoi::indexT targetModule);

        IROperand visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        yoi::IROperand visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule);

        yoi::IROperand visit(yoi::subscriptExpr *subscriptExpr);

        yoi::IROperand visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule);

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

        void visit(yoi::useStmt *useStmt);

        IRValueType parseTypeSpec(yoi::typeSpec *typeSpec);

        void visit(yoi::funcDefStmt *funcDefStmt);

        yoi::IROperand visit(yoi::interfaceDefStmt *interfaceDefStmt);

        yoi::IROperand visit(yoi::structDefStmt *structDefStmt);

        yoi::IROperand visit(yoi::implStmt *implStmt);

        yoi::IROperand visit(yoi::letStmt *letStmt);

        yoi::IROperand visit(yoi::globalStmt *globalStmt);

        yoi::IROperand visit(yoi::ifStmt *ifStmt);

        yoi::IROperand visit(yoi::whileStmt *whileStmt);

        yoi::IROperand visit(yoi::forStmt *forStmt);

        yoi::IROperand visit(yoi::forEachStmt *forEachStmt);

        yoi::IROperand visit(yoi::returnStmt *returnStmt);

        yoi::IROperand visit(yoi::continueStmt *continueStmt);

        yoi::IROperand visit(yoi::breakStmt *breakStmt);

        yoi::IROperand visit(yoi::inCodeBlockStmt *inCodeBlockStmt);
    };

} // yoi

#endif //HOSHI_LANG_VISITOR_H
