//
// Created by XIaokang00010 on 2024/9/6.
//

#include "visitor.h"

namespace yoi {
    visitor::visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext): moduleContext(moduleContext) {
        irModule = std::make_shared<yoi::IRModule>();
    }

    std::shared_ptr<yoi::IRModule> visitor::visit() {
        visit(&moduleContext->getModuleAST());
        return irModule;
    }

    void visitor::visit(yoi::hoshiModule *module) {
        // TODO
    }

    yoi::IROperand visitor::visit(yoi::basicLiterals *basicLiterals) {
        switch (basicLiterals->node.kind) {
            case yoi::lexer::token::tokenKind::integer: {
                return {IROperand::operandType::integer, {basicLiterals->node.basicVal.vInt}};
            }
            case yoi::lexer::token::tokenKind::decimal: {
                return {IROperand::operandType::decimal, {basicLiterals->node.basicVal.vDeci}};
            }
            case yoi::lexer::token::tokenKind::string: {
                auto literalIndex = irModule->stringLiteralPool.addStringLiteral(basicLiterals->node.strVal);
                return {IROperand::operandType::stringLiteral, literalIndex};
            }
            case yoi::lexer::token::tokenKind::boolean: {
                return {IROperand::operandType::boolean, {basicLiterals->node.basicVal.vBool}};
            }
            case yoi::lexer::token::tokenKind::character: {
                return {IROperand::operandType::character, {basicLiterals->node.strVal.front()}};
            }
            default: {
                panic(basicLiterals->node.line, basicLiterals->node.col, "Unexpected basic literal type");
                return {};
            }
        }
    }

    yoi::IROperand visitor::visit(yoi::identifier *identifier) {

    }

    yoi::IROperand visitor::visit(yoi::primary *primary) {
        switch (primary->kind) {
            case 0:
                return visit(primary->member);
            case 1:
                return visit(primary->literals);
            case 2:
                return visit(primary->member);
            default:
                panic(0, 0, "Unexpected primary type");
        }
    }

    IROperand visitor::visit(yoi::uniqueExpr *uniqueExpr) {
        auto var = visit(&uniqueExpr->getLhs());
        switch (uniqueExpr->getOp().kind) {
            case lexer::token::tokenKind::incrementSign: {
                if (var.type == IROperand::operandType::objectReference) {} // TODO: dereference
                assert(var.type == IROperand::operandType::localVar, uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected operand type for increment/decrement");
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::increment, {var}}
                        );
                break;
            }
            case lexer::token::tokenKind::decrementSign: {
                if (var.type == IROperand::operandType::objectReference) {} // TODO: dereference
                assert(var.type == IROperand::operandType::localVar, uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected operand type for increment/decrement");
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::decrement, {var}}
                        );
                break;
            }
            case lexer::token::tokenKind::binaryNot: {
                if (var.type == IROperand::operandType::objectReference) {} // TODO: dereference
                assert(var.type == IROperand::operandType::localVar, uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected operand type for unary operator");
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::bitwiseNot, {var}}
                        );
                break;
            }
            case lexer::token::tokenKind::minus: {
                if (var.type == IROperand::operandType::objectReference) {} // TODO: dereference
                assert(var.type == IROperand::operandType::localVar, uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected operand type for unary operator");
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::negate, {var}}
                        );
                break;
            }
            default: {
                panic(uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected unique expression type");
                return {};
            }
        }
        return var;
    }

    yoi::IROperand visitor::visit(yoi::mulExpr *mulExpr) {
        auto term = mulExpr->getTerms().begin();
        auto op = mulExpr->getOp().begin();
        auto lhs = visit(*term);
        for (term++; term != mulExpr->getTerms().end(); op++) {
            if (term == mulExpr->getTerms().end()) {
                break;
            }
            auto rhs = visit(*term);
            switch (op->kind) {
                case lexer::token::tokenKind::asterisk: {
                    moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                            {IR::Opcode::mul, {lhs, rhs}}
                    );
                }
                case lexer::token::tokenKind::slash: {
                    moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                            {IR::Opcode::div, {lhs, rhs}}
                    );
                }
                case lexer::token::tokenKind::percentSign: {
                    moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                            {IR::Opcode::mod, {lhs, rhs}}
                    );
                }
                default: {
                    panic(op->line, op->col, "Unexpected binary operator");
                    return {};
                }
            }
        }
    }


} // yoi