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
#include "compiler/llvmCodegen/codegenObjectCache.hpp"
#include "compiler/llvmCodegen/codegenTaskDispatcher.hpp"
#include "share/def.hpp"

#include <map>
#include <memory>
#include <stack>
#include <vector>

#ifdef LLVM_CODEGEN_DEBUG
#define TIMER(X, Y) { auto start = std::chrono::high_resolution_clock::now(); Y; auto end = std::chrono::high_resolution_clock::now(); std::cout << X << " took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl; }
#else
#define TIMER(X, Y) Y
#endif

namespace yoi {

    class LLVMCodegen {
    public:
        LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, const std::shared_ptr<IRModule> & yoiModule);

        yoi::vec<yoi::wstr> generate();

        void dumpIR(const yoi::wstr& modulePath, const std::string& filename);

    private:
        CodegenObjectCache codegenObjectCache;
        CodegenTaskDispatcher codegenTaskDispatcher;

        struct ControlFlowAnalysis {
            std::map<yoi::indexT, std::vector<indexT>> G;        // graph
            std::map<yoi::indexT, std::vector<indexT>> reverseG; // record the predecessors of each block

            ControlFlowAnalysis(const std::vector<std::shared_ptr<IRCodeBlock>> &blocks);
        };

        /**
         * @brief Stack Value struct exposed to original code base for compatibility.
         * @note value stack receives a StackValue and convert it into a phi node onto the stack, so it gets a phi node when acquiring, wherea for the block who has only one predecessor or no predecessor, it won't be preprocessed.
         */
        struct StackValue {
            llvm::Value *llvmValue;
            std::shared_ptr<IRValueType> yoiType;
            IRMetadata metadata;
        };

        struct ValueStackWithPhi {
            std::map<yoi::indexT, yoi::vec<StackValue>> valueStackStateIn;
            std::map<yoi::indexT, yoi::vec<StackValue>> valueStackStateOut;
            std::map<yoi::indexT, yoi::vec<llvm::PHINode *>> phiNodes;
            enum class StackState {
                Finalized,
                InEvaluation
            } stackState;
            const ControlFlowAnalysis &cfa;
            yoi::indexT currentState;
            llvm::IRBuilder<> *builder;
            std::shared_ptr<IRModule> yoiModule;

            ValueStackWithPhi(const ControlFlowAnalysis &cfa, llvm::IRBuilder<> *builder, const std::shared_ptr<IRModule> &yoiModule);

            void enterNode(yoi::indexT currentState,
                           yoi::indexT fromState,
                           llvm::BasicBlock *currentBlock,
                           llvm::BasicBlock *fromBlock);

            void enterNode(yoi::indexT currentState, llvm::BasicBlock *currentBlock);

            void finalizeNode();

            void push_back(const StackValue &value);

            StackValue &back();

            void pop_back();

            void clear();

            StackValue &operator[](yoi::indexT index);

            yoi::indexT size() const;

            bool empty() const;
        };

        class LLVMModuleContext {
        public:
            std::unique_ptr<llvm::LLVMContext> TheContext;
            yoi::wstr absolute_path;
            std::unique_ptr<llvm::Module> TheModule;
            std::unique_ptr<llvm::IRBuilder<>> Builder;
            std::unique_ptr<llvm::DIBuilder> DBuilder;
            std::map<yoi::wstr, llvm::Function *> runtimeFunctions;
            std::map<yoi::wstr, llvm::DICompileUnit *> compileUnits;
            std::map<yoi::wstr, llvm::DIType *> basicDITypeMap;

            // Singleton None object
            llvm::GlobalVariable *noneObjectSingleton = nullptr;
            llvm::GlobalVariable *RTTITable = nullptr;
            llvm::StructType *RTTIEntryType = nullptr;

            // Codegen state
            ValueStackWithPhi valueStackPhi;
            llvm::Function *currentFunction = nullptr;
            std::shared_ptr<yoi::IRFunctionDefinition> currentFunctionDef;
            std::map<yoi::indexT, llvm::AllocaInst *> namedValues; // Maps local var index to AllocaInst
            std::map<yoi::indexT, llvm::BasicBlock *> basicBlockMap; // [from_block, to_block] => target basic block
            std::map<yoi::indexT, bool> basicBlockVisited;

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

            ControlFlowAnalysis controlFlowAnalysis;
            
            yoi::indexT nextTypeId;

            LLVMModuleContext(const std::shared_ptr<IRModule> &yoiModule,
                              yoi::indexT hash,
                              const yoi::wstr &absolute_path);
        };

        // Yoi language context
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> yoiModule;
        mutable std::mutex contextMutex;

        std::map<yoi::wstr, std::unique_ptr<LLVMModuleContext>> llvmModuleContext;

        bool is_rtti_table_frozen = false;

        void generate(LLVMModuleContext &llvmModCtx);

        llvm::Module *getModule(LLVMModuleContext &llvmModCtx);

        // Helper methods
        void declareRuntimeFunctions(LLVMModuleContext &llvmModCtx);
        void generateRuntimeFunctionImplementations(LLVMModuleContext &llvmModCtx);

        void generateBasicTypeDeclarations(LLVMModuleContext &llvmModCtx);
        void generateBasicTypeImplementations(LLVMModuleContext &llvmModCtx);

        void generateDeclarations(LLVMModuleContext &llvmModCtx);
        void generateStructShallowDeclarations(LLVMModuleContext &llvmModCtx);
        void generateGlobalDeclarations(LLVMModuleContext &llvmModCtx);
        void generateFunctionDeclarations(LLVMModuleContext &llvmModCtx);
        void generateImportFunctionDeclarations(LLVMModuleContext &llvmModCtx);

        void generateImplementations(LLVMModuleContext &llvmModCtx);
        void generateStructDeclarations(LLVMModuleContext &llvmModCtx);
        void generateStructGCFunctionDeclarations(LLVMModuleContext &llvmModCtx);
        void generateStructGCFunctionImplementations(LLVMModuleContext &llvmModCtx);
        void generateInterfaceObjectGCFunctionDeclarations(LLVMModuleContext &llvmModCtx);
        void generateInterfaceObjectGCFunctionImplementations(LLVMModuleContext &llvmModCtx);
        void generateForeignStructTypes(LLVMModuleContext &llvmModCtx);
        void generateExportFunctionDecls(LLVMModuleContext &llvmModCtx);
        void generateImportFunctionImplementations(LLVMModuleContext &llvmModCtx);
        void generateMainFunction(LLVMModuleContext &llvmModCtx);

        void generateWrapperForForeignCallablesIfNotExists(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type);

        void generateFunctionImplementations(LLVMModuleContext &llvmModCtx);
        void generateFunction(LLVMModuleContext &llvmModCtx, IRFunctionDefinition &funcDef);
        void generateFunctionExitCleanup(LLVMModuleContext &llvmModCtx);
        void generateCodeBlock(LLVMModuleContext &llvmModCtx, IRCodeBlock &block, yoi::indexT fromBlock, yoi::indexT toBlock, llvm::BasicBlock *actualFromBlock = nullptr);
        void generateInstruction(LLVMModuleContext &llvmModCtx, const IR &instr, yoi::indexT fromBlock, yoi::indexT toBlock);
        void generateDescription(LLVMModuleContext &llvmModCtx);
        void generateRTTIDeclaration(LLVMModuleContext &llvmModCtx);
        void generateRTTIImplmentation(LLVMModuleContext &llvmModCtx);

        std::shared_ptr<IRValueType> normalizeForeignType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type);
        llvm::Type *yoiTypeToLLVMType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, bool enforceForeignType = false);
        llvm::Type *getArrayLLVMType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, bool enforceForeignType = false);
        llvm::Type *getDynamicArrayLLVMType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, bool enforceForeignType = false);
        void generateArrayGCFunctionDeclarations(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, llvm::StructType *structType, llvm::Type *baseType);
        llvm::FunctionType *getFunctionType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRFunctionDefinition> &funcDef);
        llvm::Constant *getGlobalInitializer(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type);

        // Helpers for specific instructions & object model
        void handleBinaryOp(LLVMModuleContext &llvmModCtx, llvm::Instruction::BinaryOps op, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock);
        void handleComparison(LLVMModuleContext &llvmModCtx, llvm::CmpInst::Predicate pred, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock);
        llvm::Value *createBasicObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &yoiType, llvm::Value *rawValue);
        llvm::Value *unboxValue(LLVMModuleContext &llvmModCtx, llvm::Value *objectPtr, const std::shared_ptr<IRValueType> &yoiType);
        llvm::Value *
        loadArrayElement(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, llvm::Value *arrayPtr, llvm::Value *index);
        llvm::Function *getGcFunction(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &yoiType, bool isIncrease);
        void callGcFunction(LLVMModuleContext &llvmModCtx, llvm::Value *objectPtr, const std::shared_ptr<IRValueType> &yoiType, bool isIncrease, bool forceForPermanent = false, bool forceForBorrow = false);
        llvm::Value *
        handleForeignTypeConv(LLVMModuleContext &llvmModCtx, llvm::Value *val, yoi::indexT foreignTypeIndex, yoi::indexT isArray, bool convertToForeign = false);
        llvm::Value *handleForeignTypeConv(LLVMModuleContext &llvmModCtx, llvm::Value *val,
                                           const std::shared_ptr<IRValueType> &foreignType,
                                           bool convertToForeign = false);
        llvm::Value *createArrayObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                       const yoi::vec<StackValue> &elements);
        llvm::DIType *getDIType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type);
        llvm::Value *createDynamicArrayObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                              const yoi::vec<StackValue> &elements,
                                              llvm::Value *size);
        void storeArrayElement(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                               const std::shared_ptr<IRValueType> &valueToStoreType,
                               llvm::Value *arrayPtr,
                               llvm::Value *index,
                               llvm::Value *value);
        void generateArrayGCFunctionImplementations(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                                    llvm::StructType *structType,
                                                    llvm::Type *baseType);

        std::pair<std::shared_ptr<IRValueType>, llvm::Value *> ensureObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                                                            llvm::Value *val);

        void generateIfTargetNotNull(LLVMModuleContext &llvmModCtx, llvm::Value *objectPtr,
                                     const std::shared_ptr<IRValueType> &yoiType,
                                     const std::function<void()> &func, bool enforced = false);

        StackValue actualizeInterfaceObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                            llvm::Value *objectPtr,
                                            yoi::indexT implIndex);

        StackValue wrapInterfaceObjectIfRegressed(LLVMModuleContext &llvmModCtx, const StackValue &objectVal);

        llvm::Value * unwrapInterfaceObject(LLVMModuleContext &llvmModCtx, const StackValue &objectVal);

        StackValue promiseInterfaceObjectIfInterface(LLVMModuleContext &llvmModCtx, const StackValue &objectVal);

        void handleIntrinsicCall(LLVMModuleContext &llvmModCtx, const IR &instr);

        LLVMModuleContext &getLLVMModuleContext(const yoi::wstr &absolutePath);

        void generateTargetObjectCode(LLVMModuleContext &llvmModCtx, const yoi::wstr &pathToOutput);
    };

} // namespace yoi

#endif // HOSHI_LANG_LLVMCODEGENCONTEXT_HPP