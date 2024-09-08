//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_VISITOR_H
#define HOSHI_LANG_VISITOR_H

#include "compiler/moduleContext.h"

namespace yoi {

    class visitor {
    public:
        std::shared_ptr<moduleContext> moduleContext;

        visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext);

        void visit();

        void visit(yoi::hoshiModule *module);

        void visit(yoi::subscript *subscript);

        void visit(yoi::basicLiterals *basicLiterals);

        void visit(yoi::identifier *identifier);
    };

} // yoi

#endif //HOSHI_LANG_VISITOR_H
