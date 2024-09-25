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

        bool isVisitingGlobalScope() const;

        /**
         * Visitor methods
         * These methods received a pointer to the corresponding AST node and
         * return an index of the current position in IR array.
         */

        void visit(yoi::hoshiModule *module);

        yoi::indexT visit(yoi::basicLiterals *basicLiterals);

        yoi::indexT visit(yoi::identifier *identifier);

        yoi::IROperand visitExtern(yoi::identifier *identifier, yoi::indexT targetModule);

        yoi::indexT visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        yoi::IROperand visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule);

        yoi::indexT visit(yoi::subscriptExpr *subscriptExpr);

        yoi::IROperand visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule);

        yoi::indexT visit(yoi::memberExpr *memberExpr);

        yoi::indexT visit(yoi::primary *primary);

        yoi::indexT visit(yoi::uniqueExpr *uniqueExpr);

        yoi::indexT visit(yoi::mulExpr *mulExpr);

        void visit(yoi::addExpr *addExpr);

        yoi::indexT visit(yoi::shiftExpr *shiftExpr);

        yoi::indexT visit(yoi::relationalExpr *relationalExpr);

        yoi::indexT visit(yoi::equalityExpr *equalityExpr);

        yoi::indexT visit(yoi::andExpr *andExpr);

        yoi::indexT visit(yoi::exclusiveExpr *exclusiveExpr);

        yoi::indexT visit(yoi::inclusiveExpr *inclusiveExpr);

        yoi::indexT visit(yoi::logicalAndExpr *logicalAndExpr);

        yoi::indexT visit(yoi::logicalOrExpr *logicalOrExpr);

        yoi::indexT visit(yoi::rExpr *rExpr);

        void visit(yoi::codeBlock *codeBlock);

        void visit(yoi::useStmt *useStmt);

        IRValueType parseTypeSpec(yoi::identifier *identifier);

        IRValueType parseTypeSpec(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        IRValueType parseTypeSpec(yoi::subscriptExpr *subscriptExpr);

        IRValueType parseTypeSpecExtern(yoi::identifier *identifier, yoi::indexT targetModule);

        IRValueType parseTypeSpecExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule);

        IRValueType parseTypeSpecExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule);

        IRValueType parseTypeSpec(yoi::typeSpec *typeSpec);

        void visit(yoi::funcDefStmt *funcDefStmt);

        yoi::IROperand visit(yoi::interfaceDefStmt *interfaceDefStmt);

        yoi::IROperand visit(yoi::structDefStmt *structDefStmt);

        yoi::IROperand visit(yoi::implStmt *implStmt);

        yoi::indexT visit(yoi::letStmt *letStmt);

        void visit(yoi::globalStmt *globalStmt);

        yoi::indexT visit(yoi::ifStmt *ifStmt);

        yoi::indexT visit(yoi::whileStmt *whileStmt);

        yoi::indexT visit(yoi::forStmt *forStmt);

        void visit(yoi::forEachStmt *forEachStmt);

        yoi::indexT visit(yoi::returnStmt *returnStmt);

        yoi::indexT visit(yoi::continueStmt *continueStmt);

        yoi::indexT visit(yoi::breakStmt *breakStmt);

        void visit(yoi::inCodeBlockStmt *inCodeBlockStmt);

        /*
        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::identifier *identifier);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::subscriptExpr *subscriptExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::memberExpr *memberExpr);

        std::shared_ptr<yoi::IRValueType> getExprTypeInfo(yoi::basicLiterals * primary);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::primary *primary);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::uniqueExpr *uniqueExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::mulExpr *mulExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::addExpr *addExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::shiftExpr *shiftExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::relationalExpr *relationalExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::equalityExpr *equalityExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::andExpr *andExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::exclusiveExpr *exclusiveExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::inclusiveExpr *inclusiveExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::logicalAndExpr *logicalAndExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::logicalOrExpr *logicalOrExpr);

        std::shared_ptr<IRValueType> getExprTypeInfo(yoi::rExpr *rExpr);
        */
    };

} // yoi

#endif //HOSHI_LANG_VISITOR_H
