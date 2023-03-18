//
// Created by XIaokang00010 on 2023/3/11.
//

#include "visitor.hpp"

namespace hoshi {

    visitor::visitor(codegenEnv &e) : env(e) {

    }

    llvm::Value *visitor::visit(basicLiterals *target) {
        switch (target->node.kind) {
            case lexer::token::tokenKind::integer:
                return env.getBuilder().getInt64(target->node.basicVal.vInt);
            case lexer::token::tokenKind::decimal:
                return llvm::ConstantFP::get(env.getLLVMCxt(), llvm::APFloat(target->node.basicVal.vDeci));
            case lexer::token::tokenKind::boolean:
                return env.getBuilder().getInt8(target->node.basicVal.vBool);
            default:
                panic(0, 0, "expected constant");
                return nullptr;
        }
    }

    llvm::Value *visitor::visit(identifier *target) {
        return env.getLocalVar(target->node.strVal).second;
    }

    llvm::Value *visitor::visit(primary *target) {
        switch (target->kind) {
            case 0:
                return visit(target->member);
            case 1:
                return visit(target->literals);
            case 2:
                return visit(target->expr);
            default:
                panic(0, 0, "internal error");
                return nullptr;
        }
    }

    llvm::Value *visitor::visit(uniqueExpr *target) {
        llvm::Value *v = visit(&target->getLhs());
        switch (target->getOp().kind) {
            case lexer::token::tokenKind::incrementSign: {
                if (v->getType()->isIntegerTy(64)) {
                    // v += 1
                    return env.getBuilder().CreateStore(env.getBuilder().CreateAdd(v, env.getBuilder().getInt64(1)), v);
                } else if (v->getType()->isDoubleTy()) {
                    // v -= 1.0
                    return env.getBuilder().CreateStore(
                            env.getBuilder().CreateFAdd(v, llvm::ConstantFP::get(env.getLLVMCxt(), llvm::APFloat(1.0))),
                            v);
                } else {
                    panic(target->op.line, target->op.col, "invalid op");
                }
            }
            case lexer::token::tokenKind::decrementSign: {
                if (v->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateSub(v, env.getBuilder().getInt64(1));
                } else if (v->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFSub(v, llvm::ConstantFP::get(env.getLLVMCxt(), llvm::APFloat(1.0)));
                } else {
                    panic(target->op.line, target->op.col, "invalid op");
                }
            }
            case lexer::token::tokenKind::minus: {
                if (v->getType()->isIntegerTy(64)) {
                    // 0 - v
                    return env.getBuilder().CreateSub(env.getBuilder().getInt64(0), v);
                } else if (v->getType()->isDoubleTy()) {
                    // 0.0 - v
                    return env.getBuilder().CreateFSub(llvm::ConstantFP::get(env.getLLVMCxt(), llvm::APFloat(0.0)), v);
                } else {
                    panic(target->op.line, target->op.col, "invalid op");
                }
            }
            case lexer::token::tokenKind::binaryNot: {
                if (v->getType()->isIntegerTy(64)) {
                    // v & -1
                    return env.getBuilder().CreateXor(v, env.getBuilder().getInt64(-1));
                }
            }
            default: {
                panic(target->getOp().line, target->getOp().col, "invalid op");
            }
        }
    }

    llvm::Value *visitor::visit(mulExpr *target) {
        auto lamMul = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateMul(lhs, rhs);
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFMul(
                            env.getBuilder().CreateSIToFP(lhs, env.getBuilder().getDoubleTy()), rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else if (lhs->getType()->isDoubleTy()) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateFMul(lhs, env.getBuilder().CreateSIToFP(rhs,
                                                                                          env.getBuilder().getDoubleTy()));
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFMul(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };
        auto lamDiv = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateSDiv(lhs, rhs);
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFDiv(
                            env.getBuilder().CreateSIToFP(lhs, env.getBuilder().getDoubleTy()), rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else if (lhs->getType()->isDoubleTy()) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateFDiv(lhs, env.getBuilder().CreateSIToFP(rhs,
                                                                                          env.getBuilder().getDoubleTy()));
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFDiv(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };
        auto lamRem = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateSRem(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };

        llvm::Value *lhs = visit(target->getTerms()[0]);
        int64_t opIdx = 0, RhsIdx = 1;
        if (*target) {
            for (; opIdx < target->getOp().size(); opIdx++, RhsIdx++) {
                auto &op = target->getOp()[opIdx];
                llvm::Value *rhs = visit(target->getTerms()[RhsIdx]);
                switch (op.kind) {
                    case lexer::token::tokenKind::asterisk:
                        lhs = lamMul(lhs, op, rhs);
                        break;
                    case lexer::token::tokenKind::slash:
                        lhs = lamDiv(lhs, op, rhs);
                        break;
                    case lexer::token::tokenKind::percentSign:
                        lhs = lamRem(lhs, op, rhs);
                        break;
                    default:
                        break;
                }
            }
        }
        return lhs;
    }

    llvm::Value *visitor::visit(addExpr *target) {
        auto lamAdd = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateAdd(lhs, rhs);
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFAdd(
                            env.getBuilder().CreateSIToFP(lhs, env.getBuilder().getDoubleTy()), rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else if (lhs->getType()->isDoubleTy()) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateFAdd(lhs, env.getBuilder().CreateSIToFP(rhs,
                                                                                          env.getBuilder().getDoubleTy()));
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFAdd(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };
        auto lamSub = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateSub(lhs, rhs);
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFSub(
                            env.getBuilder().CreateSIToFP(lhs, env.getBuilder().getDoubleTy()), rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else if (lhs->getType()->isDoubleTy()) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateFSub(lhs, env.getBuilder().CreateSIToFP(rhs,
                                                                                          env.getBuilder().getDoubleTy()));
                } else if (rhs->getType()->isDoubleTy()) {
                    return env.getBuilder().CreateFSub(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            } else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };

        llvm::Value *lhs = visit(target->getTerms()[0]);
        int64_t opIdx = 0, RhsIdx = 1;
        if (*target) {
            for (; opIdx < target->getOp().size(); opIdx++, RhsIdx++) {
                auto &op = target->getOp()[opIdx];
                llvm::Value *rhs = visit(target->getTerms()[RhsIdx]);
                switch (op.kind) {
                    case lexer::token::tokenKind::plus:
                        lhs = lamAdd(lhs, op, rhs);
                        break;
                    case lexer::token::tokenKind::minus:
                        lhs = lamSub(lhs, op, rhs);
                        break;
                    default:
                        break;
                }
            }
        }
        return lhs;
    }

    llvm::Value *visitor::visit(shiftExpr *target) {
        auto lamShl = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateShl(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            }  else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };
        auto lamShr = [&](llvm::Value *lhs, lexer::token &op, llvm::Value *rhs) {
            if (lhs->getType()->isIntegerTy(64)) {
                if (rhs->getType()->isIntegerTy(64)) {
                    return env.getBuilder().CreateLShr(lhs, rhs);
                } else {
                    panic(op.line, op.col, "invalid lhs and rhs");
                }
            }  else {
                panic(op.line, op.col, "invalid lhs and rhs");
            }
            return (llvm::Value *) {nullptr};
        };

        llvm::Value *lhs = visit(target->getTerms()[0]);
        int64_t opIdx = 0, RhsIdx = 1;
        if (*target) {
            for (; opIdx < target->getOp().size(); opIdx++, RhsIdx++) {
                auto &op = target->getOp()[opIdx];
                llvm::Value *rhs = visit(target->getTerms()[RhsIdx]);
                switch (op.kind) {
                    case lexer::token::tokenKind::binaryShiftLeft:
                        lhs = lamShl(lhs, op, rhs);
                        break;
                    case lexer::token::tokenKind::binaryShiftRight:
                        lhs = lamShr(lhs, op, rhs);
                        break;
                    default:
                        break;
                }
            }
        }
        return lhs;
    }
}