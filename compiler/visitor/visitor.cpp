//
// Created by XIaokang00010 on 2024/9/6.
//

#include "visitor.h"

namespace yoi {
    visitor::visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext): moduleContext(moduleContext) {

    }

    void visitor::visit() {
        visit(&moduleContext->getModuleAST());
    }

    void visitor::visit(yoi::hoshiModule *module) {
        // TODO
    }

    void visitor::visit(yoi::basicLiterals *basicLiterals) {
        switch (basicLiterals->node.kind) {
            case yoi::lexer::token::tokenKind::integer:
                moduleContext->getIRBuilder()
        }
    }
} // yoi