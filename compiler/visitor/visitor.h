//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_VISITOR_H
#define HOSHI_LANG_VISITOR_H

#include "compiler/builtinModule.hpp"
#include "compiler/compilerContext.h"
#include "compiler/frontend/lexer.hpp"
#include "compiler/frontend/parser.hpp"
#include "share/def.hpp"
#include <compiler/ir/IR.h>
#include <compiler/moduleContext.h>
#include <memory>
#include <ranges>
#include <stdexcept>

namespace yoi {

    class visitor {
        struct OverloadResult {
            yoi::indexT functionIndex = -1;
            bool isVariadic = false;
            bool isVirtual = false;
            yoi::indexT fixedArgCount = 0;
            std::shared_ptr<IRValueType> variadicElementType = nullptr;
            std::shared_ptr<IRFunctionDefinition> function = nullptr;

            bool found() const;
        };

        std::stack<std::pair<std::shared_ptr<yoi::moduleContext>, yoi::indexT>> moduleContextStack;

        void pushModuleContext(yoi::indexT moduleIndex);

        void popModuleContext();

      public:
        std::shared_ptr<yoi::moduleContext> moduleContext;
        std::shared_ptr<yoi::IRModule> irModule;
        yoi::indexT currentModuleIndex;

        visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext,
                const std::shared_ptr<yoi::IRModule> &irModule,
                yoi::indexT moduleIndex);

        std::shared_ptr<yoi::IRModule> visit();

        /**
         * @brief check if the subscript expression is a module name
         * @param it The subscript expression being checked
         * @param currentModule The current module being checked, -1 if not in a module
         * @return -1 if not a module name, otherwise the index of the module in the module table
         */
        yoi::indexT isModuleName(identifierWithTemplateArg *it, yoi::indexT currentModule) const;

        yoi::indexT isModuleName(identifier *it, yoi::indexT currentModule) const;

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
         * @deprecated Extern entries are not used anymore, use getExternEntry to get the direct entry instead.
         */
        [[deprecated(
            "Extern entries are not used anymore, use getExternEntry to get the direct entry instead.")]] yoi::indexT
        addExternEntryIfNotExists(yoi::indexT moduleIndex, const yoi::wstr &identifier);

        bool isVisitingGlobalScope() const;

        void emitBasicCastInBasicArithOpByLhsAndRhs(yoi::indexT lhs, yoi::indexT rhs);

        void emitBasicCastTo(const std::shared_ptr<IRValueType> &toType);

        yoi::wstr getInterfaceNameStr(const std::pair<yoi::indexT, yoi::indexT> &interfaceSrc);

        yoi::wstr getTypeSpecUniqueNameStr(const std::shared_ptr<IRValueType> &type);

        yoi::wstr getFuncUniqueNameStr(const std::vector<std::shared_ptr<IRValueType>> &argumentTypes,
                                       bool whetherIgnoreFirstParam = false);

        std::shared_ptr<IRValueType> getIncompleteType(const yoi::wstr &typeName) const;

        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument>
        getTemplateArgs(const yoi::defTemplateArg &templateArgs);

        yoi::vec<std::shared_ptr<IRValueType>> parseTemplateArgs(const yoi::templateArg &templateArgs);

        yoi::indexT specializeFunctionTemplate(yoi::funcDefStmt *astNode,
                                               const yoi::vec<std::shared_ptr<IRValueType>> &templateArgs, yoi::indexT moduleIndex);

        yoi::indexT specializeStructTemplate(const yoi::wstr &templateName,
                                             const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                             yoi::implStmt *pureTemplateImplAst, yoi::indexT moduleIndex);

        std::pair<yoi::indexT, yoi::wstr>  specializeStructMethodDeclaration(IRTemplateBuilder &structTemplate,
                                               yoi::structDefInnerPair *methodAstNode,
                                               const yoi::wstr &specializedStructName,
                                               const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                               yoi::indexT moduleIndex);

        void specializeStructMethodDefinition(IRTemplateBuilder &structTemplate,
                                             const std::shared_ptr<IRStructDefinition> &specializedStruct,
                                             yoi::implInnerPair *methodAstNode,
                                             const yoi::wstr &specializedStructName,
                                             const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                             yoi::indexT moduleIndex);

        yoi::wstr getSpecializedMangledMethodName(yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArgs,
                                                  const yoi::wstr &baseMethodName,
                                                  const yoi::vec<std::shared_ptr<IRValueType>> &specializedArgTypes);
                                                  
        yoi::indexT specializeInterfaceTemplate(const yoi::wstr &templateName,
                                              const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs, yoi::indexT moduleIndex);

        void specializeInterfaceImplementation(yoi::implStmt *implAst,
                                               const std::shared_ptr<IRValueType> &concreteStructType,
                                               const yoi::wstr& specializedStructName,
                                               const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs);

        yoi::wstr getMangledTemplateName(const yoi::wstr &baseName,
                                         const yoi::vec<std::shared_ptr<IRValueType>> &templateArgs);

        void tryCastTo(const std::shared_ptr<IRValueType> &toType);

        yoi::vec<IRFunctionDefinition::FunctionAttrs> getFunctionAttributes(const yoi::vec<lexer::token> &attrs);

        yoi::indexT generateNullInterfaceImplementation(const std::shared_ptr<IRValueType> &structType);

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

        yoi::indexT visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg,
                                yoi::indexT targetModule,
                                bool isStoreOp = false);

        yoi::indexT visit(yoi::subscriptExpr *subscriptExpr, bool isStoreOp = false);

        yoi::indexT visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule, bool isStoreOp = false);

        yoi::indexT visit(yoi::memberExpr *memberExpr, bool isStoreOp = false);

        yoi::indexT visit(yoi::primary *primary, bool isStoreOp = false);

        yoi::indexT visit(yoi::abstractExpr *abstractExpr, bool isStoreOp = false);

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

        yoi::indexT visit(yoi::typeIdExpression *typeIdExpression);

        yoi::indexT visit(yoi::dynCastExpression *dynCastExpression);

        yoi::indexT visit(yoi::newExpression *newExpression);

        void visit(yoi::codeBlock *codeBlock, bool notEmitNewBlockInstruction = false);

        yoi::indexT visit(yoi::useStmt *useStmt);

        IRValueType parseTypeSpec(yoi::identifier *identifier);

        IRValueType parseTypeSpec(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        IRValueType parseTypeSpec(yoi::subscriptExpr *subscriptExpr);

        IRValueType parseTypeSpecExtern(yoi::identifier *identifier, yoi::indexT targetModule);

        IRValueType parseTypeSpecExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg,
                                        yoi::indexT targetModule);

        IRValueType parseTypeSpecExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule);

        IRValueType parseTypeSpec(yoi::funcTypeSpec *typeSpec);

        IRValueType parseTypeSpec(yoi::typeSpec *typeSpec);

        IRValueType parseTypeSpec(yoi::externModuleAccessExpression *emaExpression);

        yoi::wstr parseIdentifierWithTemplateArg(yoi::identifierWithTemplateArg *identifierWithTemplateArg);

        yoi::wstr getInterfaceImplName(const std::pair<yoi::indexT, yoi::indexT> &interfaceSrc,
                                       const std::shared_ptr<IRValueType> &typeSrc);

        std::pair<std::pair<yoi::indexT, yoi::indexT>, std::shared_ptr<IRInterfaceInstanceDefinition>>
        parseInterfaceName(yoi::externModuleAccessExpression *structDef);

        yoi::indexT visit(yoi::funcDefStmt *funcDefStmt);

        yoi::indexT visit(yoi::interfaceDefStmt *interfaceDefStmt);

        yoi::indexT visit(yoi::structDefStmt *structDefStmt);

        yoi::indexT visit(yoi::implStmt *implStmt);

        yoi::indexT visit(yoi::letStmt *letStmt);

        yoi::indexT visit(yoi::exportDecl *exportDecl);

        yoi::indexT visit(yoi::importDecl *importDecl);

        void visit(yoi::globalStmt *globalStmt);

        yoi::indexT visit(yoi::ifStmt *ifStmt);

        yoi::indexT visit(yoi::whileStmt *whileStmt);

        yoi::indexT visit(yoi::forStmt *forStmt);

        void visit(yoi::forEachStmt *forEachStmt);

        yoi::indexT visit(yoi::returnStmt *returnStmt);

        yoi::indexT visit(yoi::continueStmt *continueStmt);

        yoi::indexT visit(yoi::breakStmt *breakStmt);

        void visit(yoi::inCodeBlockStmt *inCodeBlockStmt);

        yoi::indexT visit(yoi::callableExpression *callableExpression);

        /**
        * @brief Visits a list of argument expressions and returns their types.
        * @param args The AST node for the argument list.
        * @return A vector of shared pointers to the argument types.
        * @note This function leaves the evaluated arguments on the IRBuilder's temporary stack.
        */
        yoi::vec<std::shared_ptr<IRValueType>> evaluateArguments(yoi::invocationArguments *args);

        /**
         * @brief Resolves an function overload within the interface context.
         * 
         * @param baseName The base name of the function (e.g., "println" or "constructor").
         * @param argTypes The types of the arguments provided at the call site.
         * @param targetModule The index of the module being looked into.
         * @param interfaceContext The interface context where the function is being called.
         * @return visitor::OverloadResult with the resolution details.
         */
        visitor::OverloadResult
        resolveOverloadInterface(const yoi::wstr &baseName,
                                 const yoi::vec<std::shared_ptr<IRValueType>> &argTypes,
                                 yoi::indexT targetModule,
                                 const std::shared_ptr<IRInterfaceInstanceDefinition> &interfaceContext);

        /**
         * @brief Resolves an external function overload within a target module.
         * @param baseName The base name of the function (e.g., "println" or "constructor").
         * @param argTypes The types of the arguments provided at the call site.
         * @param targetModule The index of the module being looked into.
         * @param structContext Optional. If not null, searches for a method within this external struct.
         * @return An OverloadResult struct with the resolution details.
         */
        OverloadResult resolveOverloadExtern(const yoi::wstr &baseName,
                                             const yoi::vec<std::shared_ptr<IRValueType>> &argTypes,
                                             yoi::indexT targetModule,
                                             const std::shared_ptr<IRStructDefinition> &structContext = nullptr);

        /**
         * @brief Orchestrates an external function/method invocation IR generation.
         */
        bool handleInvocationExtern(const yoi::wstr &baseName,
                                    yoi::invocationArguments *args,
                                    yoi::indexT targetModule,
                                    const std::shared_ptr<IRValueType> &structContext = nullptr, bool noThisCall = false);

        /**
         * @brief Generates a call to certain operator overload function when left hand side or right hand side owns a appropriate overloaded operator method.
         * 
         * @param overloadName the name of the operator overload method
         * @return yoi::indexT current insertion point after the invocation
         * @note Make sure the builder state is saved before calling this helper function.
         */
        yoi::indexT handleBinaryOperatorOverload(const yoi::wstr &overloadName);

        /**
         * @brief Handles unary operator overload function when the operand owns a appropriate overloaded operator method.
         * 
         * @param overloadName the name of the operator overload method
         * @return yoi::indexT current insertion point after the invocation
         */
        yoi::indexT handleUnaryOperatorOverload(const yoi::wstr &overloadName);

        /**
         * @brief Handle subscript
         * @return bool whether the it need to continue to handle subscript
         */
        bool handleSubscript(yoi::vec<yoi::subscript *>::iterator &it,
                             yoi::vec<yoi::subscript *>::iterator end,
                             bool isStoreOp,
                             bool isLastTerm);

        yoi::indexT createCallableInterface(const yoi::vec<std::shared_ptr<IRValueType>> &parameterTypes,
                                            const std::shared_ptr<IRValueType> &returnType);

        std::pair<yoi::indexT, std::pair<yoi::indexT, yoi::indexT>> createCallableImplementationForLambda(const std::shared_ptr<IRStructDefinition> &lambda,
                                                          yoi::indexT lambdaStructIndex,
                                                          yoi::indexT moduleIndex);

        yoi::indexT createLambdaUnnamedStruct(yoi::lambdaExpr *lambdaExpr);
    };

} // namespace yoi

#endif // HOSHI_LANG_VISITOR_H