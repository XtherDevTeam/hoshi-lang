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
                auto var = moduleContext->getIRBuilder().createTempVar(IRValueType::valueType::integer);
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::store, vec < IROperand > {
                                {IROperand::operandType::integer, {(int64_t) 1}}, var}});
                return var;
            }
            case yoi::lexer::token::tokenKind::decimal: {
                auto var = moduleContext->getIRBuilder().createTempVar(IRValueType::valueType::decimal);
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::store, vec < IROperand > {
                                {IROperand::operandType::decimal, {(int64_t) 1}}, var}});
                return var;
            }
            case yoi::lexer::token::tokenKind::string: {
                auto var = moduleContext->getIRBuilder().createTempVar(IRValueType::valueType::stringLiteral);
                auto literalIndex = irModule->stringLiteralPool.addStringLiteral(basicLiterals->node.strVal);
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert({
                    IR::Opcode::store, vec < IROperand > {
                            {IROperand::operandType::stringLiteral, literalIndex}, var}
                });
                return var;
            }
            case yoi::lexer::token::tokenKind::boolean: {
                auto var = moduleContext->getIRBuilder().createTempVar(IRValueType::valueType::boolean);
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::store, vec < IROperand > {
                                {IROperand::operandType::boolean, {(int64_t) 1}}, var}});
                return var;
            }
            case yoi::lexer::token::tokenKind::character: {
                auto var = moduleContext->getIRBuilder().createTempVar(IRValueType::valueType::character);
                moduleContext->getIRBuilder().getCurrentCodeBlock().insert(
                        {IR::Opcode::store, vec < IROperand > {
                                {IROperand::operandType::character, {(int64_t) 1}}, var}});
                return var;
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

    void visitor::visit(yoi::uniqueExpr *uniqueExpr) {

    }


} // yoi