//
// Created by XIaokang00010 on 2023/3/11.
//

#include "codegenEnv.hpp"

namespace hoshi {
    void codegenEnv::openBasicBlock(llvm::BasicBlock *basicBlock) {
        basicBlocks.emplace_back(basicBlockIndexes++, basicBlock);
    }

    void codegenEnv::closeBasicBlock() {
        basicBlocks.pop_back();
    }

    llvm::LLVMContext &codegenEnv::getLLVMCxt() {
        return *llvmContext;
    }

    llvm::IRBuilder<> &codegenEnv::getBuilder() {
        return *irBuilder;
    }

    int64_t codegenEnv::pushLocalVar(const wstr &name, llvm::Value *value) {
        wstr varName = name + L"." + std::to_wstring(basicBlocks.back().first);
        llvm::Value *v = getBuilder().CreateAlloca(value->getType(), nullptr, wstring2string(varName));
        return localVars.put(varName, v);
    }

    std::pair<wstr, llvm::Value *> codegenEnv::getLocalVar(const wstr &name) {
        wstr realName{};
        for (auto it = basicBlocks.rbegin(); it != basicBlocks.rend(); it++) {
            realName = name + L"." + std::to_wstring(it->first);
            try {
                return {realName, localVars[realName]};
            } catch (std::runtime_error &e) {
                continue;
            }
        }
        panic(0, 0, "unknown variable name: " + wstring2string(name));
        return {};
    }
}