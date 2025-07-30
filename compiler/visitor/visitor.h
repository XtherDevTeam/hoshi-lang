//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_VISITOR_H
#define HOSHI_LANG_VISITOR_H

#include <compiler/ir/IR.h>
#include <compiler/moduleContext.h>
#include <memory>

namespace yoi {

    class visitor {
    public:
        std::shared_ptr<yoi::moduleContext> moduleContext;
        std::shared_ptr<yoi::IRModule> irModule;
        yoi::indexT currentModuleIndex;

        visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext, const std::shared_ptr<yoi::IRModule> &irModule, yoi::indexT moduleIndex);

        std::shared_ptr<yoi::IRModule> visit();

        /**
         * @brief check if the subscript expression is a module name
         * @param it The subscript expression being checked
         * @param currentModule The current module being checked, -1 if not in a module
         * @return -1 if not a module name, otherwise the index of the module in the module table
         */
        yoi::indexT isModuleName(identifierWithTemplateArg *it, yoi::indexT currentModule) const;

        /**
         * @brief search and return extern entry by identifier in target module
         * @param moduleIndex the index of target module
         * @param identifier the identifier to search
         * @return the extern entry
         * @throw std::runtime_error if identifier not found
         */
        yoi::IRExternEntry getExternEntry(yoi::indexT moduleIndex, const yoi::wstr &identifier) const;

        /**
         * Add an extern entry to the module if it does not exist.
         * @param moduleIndex the index of module begin imported
         * @param identifier the identifier of the extern entry
         * @return the index of the extern entry in the module's extern table
         * @throws std::runtime_error if the identifier is not found in the module
         */
        yoi::indexT addExternEntryIfNotExists(yoi::indexT moduleIndex, const yoi::wstr &identifier);

        bool isVisitingGlobalScope() const;

        void emitBasicCastInBasicArithOpByLhsAndRhs(yoi::indexT lhs, yoi::indexT rhs);

        yoi::wstr getInterfaceNameStr(const std::pair<yoi::indexT, yoi::indexT> &interfaceSrc);

        yoi::wstr getTypeSpecUniqueNameStr(const std::shared_ptr<IRValueType> &type);

        yoi::wstr getFuncUniqueNameStr(const std::shared_ptr<IRFunctionDefinition> &func);

        /**
         * Visitor methods
         * These methods received a pointer to the corresponding AST node and
         * return an index of the current position in IR array.
         */

        void visit(yoi::hoshiModule *module);

        yoi::indexT visit(yoi::basicLiterals *basicLiterals);

        yoi::indexT visit(yoi::identifier *identifier, bool isStoreOp = false);

        yoi::indexT visitExtern(yoi::identifier *identifier, yoi::indexT targetModule, bool isStoreOp = false);

        yoi::indexT visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg, bool isStoreOp = false);

        yoi::indexT visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule, bool isStoreOp = false);

        yoi::indexT visit(yoi::subscriptExpr *subscriptExpr, bool isStoreOp = false);

        yoi::indexT visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule, bool isStoreOp = false);

        yoi::indexT visit(yoi::memberExpr *memberExpr, bool isStoreOp = false);

        yoi::indexT visit(yoi::primary *primary, bool isStoreOp = false);

        yoi::indexT visit(yoi::uniqueExpr *uniqueExpr, bool isStoreOp = false);

        yoi::indexT visit(yoi::leftExpr *leftExpr);

        yoi::indexT visit(yoi::mulExpr *mulExpr);

        yoi::indexT visit(yoi::addExpr *addExpr);

        yoi::indexT visit(yoi::shiftExpr *shiftExpr);

        yoi::indexT visit(yoi::relationalExpr *relationalExpr);

        yoi::indexT visit(yoi::equalityExpr *equalityExpr);

        yoi::indexT visit(yoi::andExpr *andExpr);

        yoi::indexT visit(yoi::exclusiveExpr *exclusiveExpr);

        yoi::indexT visit(yoi::inclusiveExpr *inclusiveExpr);

        yoi::indexT visit(yoi::logicalAndExpr *logicalAndExpr);

        yoi::indexT visit(yoi::logicalOrExpr *logicalOrExpr);

        yoi::indexT visit(yoi::rExpr *rExpr);

        void visit(yoi::codeBlock *codeBlock, bool notEmitNewBlockInstruction = false);

        yoi::indexT visit(yoi::useStmt *useStmt);

        IRValueType parseTypeSpec(yoi::identifier *identifier);

        IRValueType parseTypeSpec(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        IRValueType parseTypeSpec(yoi::subscriptExpr *subscriptExpr);

        IRValueType parseTypeSpecExtern(yoi::identifier *identifier, yoi::indexT targetModule);

        IRValueType parseTypeSpecExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule);

        IRValueType parseTypeSpecExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule);

        IRValueType parseTypeSpec(yoi::typeSpec *typeSpec);

        yoi::wstr parseIdentifierWithTemplateArg(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        yoi::wstr getInterfaceImplName(const std::pair<yoi::indexT, yoi::indexT> &interfaceSrc, const std::pair<yoi::indexT, yoi::indexT> &structSrc);

        std::pair<std::pair<yoi::indexT, yoi::indexT>, std::shared_ptr<IRInterfaceInstanceDefinition>> parseInterfaceName(
         yoi::externModuleAccessExpression *structDef);

        yoi::indexT visit(yoi::funcDefStmt *funcDefStmt);

        yoi::indexT visit(yoi::interfaceDefStmt *interfaceDefStmt);

        yoi::indexT visit(yoi::structDefStmt *structDefStmt);

        yoi::indexT visit(yoi::implStmt *implStmt);

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

    };

} // yoi

#endif //HOSHI_LANG_VISITOR_H
