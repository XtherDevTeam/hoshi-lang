//
// Created by XIaokang00010 on 2024/10/9.
//

#ifndef HOSHI_LANG_LLVMCODEGENCONTEXT_HPP
#define HOSHI_LANG_LLVMCODEGENCONTEXT_HPP

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include "compiler/ir/IR.h"
#include "share/def.hpp"

#include <map>
#include <memory>
#include <stack>
#include <vector>

namespace yoi {

    class LLVMCodegen {
      public:
        LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, std::shared_ptr<IRModule> yoiModule);

        // Generate the LLVM Module from the yoi::IRModule.
        void generate();

        // Get the generated module.
        llvm::Module *getModule();

        void generateTargetObjectCode(const yoi::wstr &pathToOutput);

      private:
        // Core LLVM components
        std::unique_ptr<llvm::LLVMContext> TheContext;
        std::unique_ptr<llvm::Module> TheModule;
        std::unique_ptr<llvm::IRBuilder<>> Builder;

        // Debug Info related
        std::unique_ptr<llvm::DIBuilder> DBuilder;
        std::map<yoi::wstr, llvm::DICompileUnit *> compileUnits;

        // Runtime functions
        llvm::Function *runtimeMalloc = nullptr;
        llvm::Function *runtimeObjectAllocReportFunc = nullptr;
        llvm::Function *runtimeObjectAllocFunc = nullptr;
        llvm::Function *runtimeFinalizeObjectReportFunc = nullptr;
        llvm::Function *runtimeFinalizeObjectFunc = nullptr;
        llvm::Function *runtimeDebugReportCurrentFunctionFunc = nullptr;
        llvm::Function *runtimeDebugPrintFunc = nullptr;
        llvm::Function *runtimeDebugPrintAddressFunc = nullptr;
        llvm::Function *runtimeDebugPrintIntFunc = nullptr;
        llvm::Function *runtimeDebugPrintDeciFunc = nullptr;
        llvm::Function *runtimeDebugPrintCurrentAllocatedMemoryFunc = nullptr;

        // Yoi language context
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> yoiModule;

        // Singleton None object
        llvm::GlobalVariable *noneObjectSingleton = nullptr;
        llvm::GlobalVariable *RTTITable = nullptr;
        llvm::StructType *RTTIEntryType = nullptr;

        struct StackValue {
            llvm::Value *llvmValue;
            std::shared_ptr<IRValueType> yoiType;
        };

        struct ControlFlowAnalysis {
            std::map<yoi::indexT, std::vector<indexT>> G;        // graph
            std::map<yoi::indexT, std::vector<indexT>> reverseG; // record the predecessors of each block

            ControlFlowAnalysis(const std::vector<std::shared_ptr<IRCodeBlock>> &blocks);
        } controlFlowAnalysis;

        // Codegen state
        std::map<yoi::indexT, std::map<yoi::indexT, yoi::vec<StackValue>>> valueStackMap;
        llvm::Function *currentFunction = nullptr;
        std::shared_ptr<yoi::IRFunctionDefinition> currentFunctionDef;
        std::map<yoi::indexT, llvm::AllocaInst *> namedValues; // Maps local var index to AllocaInst
        std::map<yoi::indexT, std::map<yoi::indexT, llvm::BasicBlock *>>
            basicBlockMap; // [from_block, to_block] => target basic block
        std::map<yoi::indexT, std::map<yoi::indexT, bool>> basicBlockVisited;

        // Mappings from yoi IR to LLVM IR
        std::map<yoi::indexT, llvm::GlobalVariable *> globalValues; // Maps global var index to GlobalVariable
        std::map<yoi::wstr, llvm::Function *> functionMap;          // Maps yoi function names to LLVM functions
        std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT>,
                 llvm::StructType *>
            structTypeMap; // Maps (type_enum, module_id, type_idx) to LLVM struct type
        std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT>,
                 llvm::Type *>
            foreignTypeMap; // Maps (type_enum, module_id, type_idx) to LLVM type
        std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT>,
                 llvm::StructType *>
            arrayTypeMap; // Maps (type_enum, module_id, type_idx, size) to LLVM array type
        std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT>,
                 llvm::DIType *>
            structTypeDIMap; // Maps (type_enum, module_id, type_idx) to LLVM DI type
        std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT>,
                 llvm::DIType *>
            arrayTypeDIMap; // Maps (type_enum, module_id, type_idx, size) to LLVM DI type
        std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT>,
                 yoi::indexT>
            typeIDMap; // Maps (type_enum, module_id, type_idx, size) to type ID (if no array, size = 0)
        yoi::vec<std::tuple<std::shared_ptr<IRValueType>, llvm::StructType *, llvm::Type *>>
            arrayToGenerateImplementations; // Array types to generate GC functions for
        yoi::indexT nextTypeId;

        // Helper methods
        void declareRuntimeFunctions();

        void generateBasicTypesAndFunctions();

        void generateDeclarations();
        void generateStructDeclarations();
        void generateGlobalDeclarations();
        void generateFunctionDeclarations();
        void generateImportFunctionDeclarations();

        void generateImplementations();
        void generateStructImplementations();
        void generateStructGCFunctionDeclarations();
        void generateStructGCFunctionImplementations();
        void generateInterfaceImplementationGCFunctions();
        void generateInterfaceObjectGCFunctionDeclarations();
        void generateInterfaceObjectGCFunctionImplementations();
        void generateForeignStructTypes();
        void generateExportFunctionDecls();
        void generateImportFunctionImplementations();
        void generateMainFunction();

        void generateFunctionImplementations();
        void generateFunction(IRFunctionDefinition &funcDef);
        void generateFunctionExitCleanup();
        void generateCodeBlock(IRCodeBlock &block, yoi::indexT fromBlock, yoi::indexT toBlock);
        void generateInstruction(const IR &instr, yoi::indexT fromBlock, yoi::indexT toBlock);
        void generateDescription();
        void generateRTTIDeclaration();
        void generateRTTIImplmentation();

        std::shared_ptr<IRValueType> normalizeForeignType(const std::shared_ptr<IRValueType> &type);
        llvm::Type *yoiTypeToLLVMType(const std::shared_ptr<IRValueType> &type, bool enforceForeignType = false);
        llvm::Type *getArrayLLVMType(const std::shared_ptr<IRValueType> &type, bool enforceForeignType = false);
        llvm::Type *getDynamicArrayLLVMType(const std::shared_ptr<IRValueType> &type, bool enforceForeignType = false);
        void generateArrayGCFunctionDeclarations(const std::shared_ptr<IRValueType> &type, llvm::StructType *structType, llvm::Type *baseType);
        llvm::FunctionType *getFunctionType(const std::shared_ptr<IRFunctionDefinition> &funcDef);
        llvm::Constant *getGlobalInitializer(const std::shared_ptr<IRValueType> &type);

        // Helpers for specific instructions & object model
        void handleBinaryOp(llvm::Instruction::BinaryOps op, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock);
        void handleComparison(llvm::CmpInst::Predicate pred, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock);
        llvm::Value *createBasicObject(const std::shared_ptr<IRValueType> &yoiType, llvm::Value *rawValue);
        llvm::Value *unboxValue(llvm::Value *objectPtr, const std::shared_ptr<IRValueType> &yoiType);
        llvm::Value *
        loadArrayElement(const std::shared_ptr<IRValueType> &type, llvm::Value *arrayPtr, llvm::Value *index);
        void callGcFunction(llvm::Value *objectPtr, const std::shared_ptr<IRValueType> &yoiType, bool isIncrease, bool forceForPermanent = false, bool forceForBorrow = false);
        llvm::Value *
        handleForeignTypeConv(llvm::Value *val, yoi::indexT foreignTypeIndex, yoi::indexT isArray, bool convertToForeign = false);
        llvm::Value *handleForeignTypeConv(llvm::Value *val,
                                           const std::shared_ptr<IRValueType> &foreignType,
                                           bool convertToForeign = false);
        llvm::Value *createArrayObject(const std::shared_ptr<IRValueType> &type,
                                       const yoi::vec<StackValue> &elements);
        llvm::DIType *getDIType(const std::shared_ptr<IRValueType> &type);
        llvm::Value *createDynamicArrayObject(const std::shared_ptr<IRValueType> &type,
                                              const yoi::vec<StackValue> &elements,
                                              llvm::Value *size);
        void storeArrayElement(const std::shared_ptr<IRValueType> &type,
                               const std::shared_ptr<IRValueType> &valueToStoreType,
                               llvm::Value *arrayPtr,
                               llvm::Value *index,
                               llvm::Value *value);
        void generateArrayGCFunctionImplementations(const std::shared_ptr<IRValueType> &type,
                                                    llvm::StructType *structType,
                                                    llvm::Type *baseType);

        std::pair<std::shared_ptr<IRValueType>, llvm::Value *> ensureObject(const std::shared_ptr<IRValueType> &type,
                                                                            llvm::Value *val);
    };

} // namespace yoi

#endif // HOSHI_LANG_LLVMCODEGENCONTEXT_HPP