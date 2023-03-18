//
// Created by XIaokang00010 on 2023/3/11.
//

#ifndef HOSHI_LANG_CODEGENENV_HPP
#define HOSHI_LANG_CODEGENENV_HPP

#include <map>
#include <share/def.hpp>

namespace hoshi {
    class structType {
    };

    class codegenEnv {
        std::shared_ptr<llvm::LLVMContext> llvmContext;
        std::shared_ptr<llvm::IRBuilder<>> irBuilder;
        indexTable<wstr, std::shared_ptr<llvm::Module>> modules;
        int64_t basicBlockIndexes{};
        std::vector<std::pair<int64_t , llvm::BasicBlock *>> basicBlocks;
        indexTable<wstr, llvm::Value *> localVars;
    public:
        void openBasicBlock(llvm::BasicBlock *basicBlock);

        void closeBasicBlock();

        llvm::LLVMContext &getLLVMCxt();

        llvm::IRBuilder<> &getBuilder();

        int64_t pushLocalVar(const wstr &name, llvm::Value *value);

        std::pair<wstr, llvm::Value *> getLocalVar(const wstr &name);
    };
}

#endif //HOSHI_LANG_CODEGENENV_HPP
