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
#include <vector>

namespace yoi {

    class moduleContext {
        yoi::wstr path;
        yoi::hoshiModule *moduleAST;
        std::map<yoi::wstr, std::shared_ptr<moduleContext>> referencedModules;
        std::shared_ptr<yoi::compilerContext> compilerContext;
        std::stack<yoi::IRBuilder> IRBuilderStack;
        std::vector<IRTemplateBuilder *> templateBuilders;
    public:
        moduleContext(std::shared_ptr<yoi::compilerContext> compilerContext, yoi::wstr path, yoi::hoshiModule *moduleAST);

        yoi::hoshiModule &getModuleAST();

        yoi::IRBuilder &getIRBuilder();

        void pushIRBuilder(const yoi::IRBuilder &builder);

        void popIRBuilder();

        void pushTemplateBuilder(IRTemplateBuilder &builder);

        void popTemplateBuilder();

        std::vector<IRTemplateBuilder *> &getTemplateBuilders();

        std::shared_ptr<yoi::compilerContext> getCompilerContext();
    };

} // hoshi

#endif //HOSHI_LANG_MODULECONTEXT_H
