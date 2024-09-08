//
// Created by XIaokang00010 on 2024/9/4.
//

#ifndef HOSHI_LANG_MODULECONTEXT_H
#define HOSHI_LANG_MODULECONTEXT_H

#include "share/def.hpp"
#include "symbolTable.h"
#include "compiler/frontend/ast.hpp"
#include "compilerContext.h"
#include "compiler/ir/IR.h"
#include <map>

namespace yoi {

    class moduleContext {
        yoi::wstr path;
        std::shared_ptr<yoi::hoshiModule> moduleAST;
        std::map<yoi::wstr, std::shared_ptr<moduleContext>> referencedModules;
        std::shared_ptr<yoi::compilerContext> compilerContext;
        std::stack<yoi::IRBuilder> IRBuilderStack;

        yoi::symbolTable symbols;
    public:
        moduleContext(std::shared_ptr<yoi::compilerContext> compilerContext, yoi::wstr path, const std::shared_ptr<yoi::hoshiModule> &AST);

        yoi::symbolTable &getSymbolTable();

        yoi::hoshiModule &getModuleAST();

        yoi::IRBuilder &getIRBuilder();

        void pushIRBuilder(const yoi::IRBuilder &builder);

        void popIRBuilder();
    };

} // hoshi

#endif //HOSHI_LANG_MODULECONTEXT_H
