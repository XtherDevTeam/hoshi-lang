//
// Created by XIaokang00010 on 2024/9/4.
//

#include "moduleContext.h"

#include <utility>

namespace yoi {

    moduleContext::moduleContext(std::shared_ptr<yoi::compilerContext> compilerContext, yoi::wstr path,
        yoi::hoshiModule *moduleAST) : compilerContext(std::move(compilerContext)), path(std::move(path)), moduleAST(moduleAST) {
    }

    yoi::hoshiModule &moduleContext::getModuleAST() {
        return *moduleAST;
    }

    yoi::IRBuilder &moduleContext::getIRBuilder() {
        return IRBuilderStack.top();
    }

    void moduleContext::pushIRBuilder(const IRBuilder &builder) {
        IRBuilderStack.push(builder);
    }

    void moduleContext::popIRBuilder() {
        IRBuilderStack.pop();
    }

    std::shared_ptr<yoi::compilerContext> moduleContext::getCompilerContext() {
        return compilerContext;
    }
    void moduleContext::pushTemplateBuilder(IRTemplateBuilder &builder) {
        templateBuilders.push_back(&builder);
    }
    void moduleContext::popTemplateBuilder() {
        templateBuilders.pop_back();
    }
    std::vector<IRTemplateBuilder *> &moduleContext::getTemplateBuilders() {
        return templateBuilders;
    }
} // namespace yoi