//
// Created by XIaokang00010 on 2023/3/11.
//

#ifndef HOSHI_LANG_VISITOR_HPP
#define HOSHI_LANG_VISITOR_HPP

#include <compiler/visitor/codegenEnv.hpp>
#include "compiler/frontend/ast.hpp"

namespace hoshi {
    class visitor {
        codegenEnv &env;
    public:
        visitor(codegenEnv &e);

        llvm::Value *visit(basicLiterals *target);

        llvm::Value *visit(identifier *target);

        llvm::Value *visit(subscriptExpr *target);

        llvm::Value *visit(memberExpr *target);

        llvm::Value *visit(primary *target);

        llvm::Value *visit(uniqueExpr *target);

        llvm::Value *visit(mulExpr *target);

        llvm::Value *visit(addExpr *target);

        llvm::Value *visit(shiftExpr *target);

        llvm::Value *visit(relationalExpr *target);

        llvm::Value *visit(equalityExpr *target);

        llvm::Value *visit(andExpr *target);

        llvm::Value *visit(exclusiveExpr *target);

        llvm::Value *visit(inclusiveExpr *target);

        llvm::Value *visit(logicalAndExpr *target);

        llvm::Value *visit(logicalOrExpr *target);

        llvm::Value *visit(rExpr *target);
    };
}

#endif //HOSHI_LANG_VISITOR_HPP
