//
// Created by XIaokang00010 on 2024/10/9.
//

#ifndef HOSHI_LANG_LLVMCODEGENCONTEXT_HPP
#define HOSHI_LANG_LLVMCODEGENCONTEXT_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h> // Added for PointerType and StructType

#include "compiler/ir/IR.h"

#include <map>
#include <vector>
#include <memory>
#include <stack>

namespace yoi {

class LLVMCodegen {
public:
    LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, std::shared_ptr<IRModule> yoiModule);

    // Generate the LLVM Module from the yoi::IRModule.
    void generate();

    // Get the generated module.
    llvm::Module* getModule();

private:
    // Core LLVM components
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;

    // Yoi language context
    std::shared_ptr<compilerContext> compilerCtx;
    std::shared_ptr<IRModule> yoiModule;

    struct StackValue {
        llvm::Value* llvmValue;
        std::shared_ptr<IRValueType> yoiType;
    };

    // Codegen state
    std::vector<StackValue> valueStack;
    llvm::Function* currentFunction = nullptr;
    std::shared_ptr<yoi::IRFunctionDefinition> currentFunctionDef;
    std::map<yoi::indexT, llvm::AllocaInst*> namedValues; // Maps local var index to AllocaInst
    std::map<yoi::indexT, llvm::BasicBlock*> blockMap; // Maps yoi block index to LLVM block

    // Mappings from yoi IR to LLVM IR
    std::map<yoi::indexT, llvm::GlobalVariable*> globalValues; // Maps global var index to GlobalVariable
    std::map<yoi::wstr, llvm::Function*> functionMap; // Maps yoi function names to LLVM functions
    std::map<std::tuple<yoi::IRValueType::valueType, yoi::indexT, yoi::indexT>, llvm::StructType*> structTypeMap; // Maps (module_id, struct_idx) to LLVM struct type

    // Helper methods
    void generateDeclarations();
    void generateStructDeclarations();
    void generateGlobalDeclarations();
    void generateFunctionDeclarations();

    void generateImplementations();
    void generateStructImplementations();
    void generateFunctionImplementations();
    void generateFunction(IRFunctionDefinition& funcDef);
    void generateCodeBlock(IRCodeBlock& block, yoi::indexT blockIdx);
    llvm::Type* getPointeeTypeFromValue(llvm::Value* PtrValue);
    void generateInstruction(const IR& instr);

    llvm::Type* yoiTypeToLLVMType(const std::shared_ptr<IRValueType>& type);
    llvm::FunctionType* getFunctionType(const std::shared_ptr<IRFunctionDefinition>& funcDef);
    llvm::Constant* getGlobalInitializer(const std::shared_ptr<IRValueType>& type);

    // Helpers for specific instructions
    void handleBinaryOp(llvm::Instruction::BinaryOps op, bool isFloat);
    void handleComparison(llvm::CmpInst::Predicate pred, bool isFloat);

};

} // yoi

#endif //HOSHI_LANG_LLVMCODEGENCONTEXT_HPP