//
// Created by XIaokang00010 on 2023/3/4.
//

#ifndef HOSHI_LANG_VISITOR_HPP
#define HOSHI_LANG_VISITOR_HPP

#include <compiler/frontend/ast.hpp>
#include <compiler/ir/ir.hpp>

namespace hoshi {
    class visitor {
        irContext *cxt;
        irBuilder *builder;
    public:
        void setContext(irContext *c);

        void visitModule(const wstr &moduleName, hoshiModule *target);

        void visitInFunc(basicLiterals *literals);
    };

} // hoshi

#endif //HOSHI_LANG_VISITOR_HPP
