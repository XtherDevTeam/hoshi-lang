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
        auto &id = identifier->node.strVal;
        try {
            auto valType = moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable()[id];
            auto res = moduleContext->getIRBuilder().createTempVar(managedPtr(IRValueType{IRValueType::valueType::lvalue, valType}));
            moduleContext->getIRBuilder().insert({
                IR{IR::Opcode::load_local, {
                    {IROperand::operandType::localVar, valType},
                    res
                    }}
            });
            return res;
        } catch (std::runtime_error &e) {

        }
        try {
            auto valType = irModule->globalVariables[id];
            auto res = moduleContext->getIRBuilder().createTempVar(managedPtr(IRValueType{IRValueType::valueType::lvalue, valType}));
            moduleContext->getIRBuilder().insert({
                IR{IR::Opcode::load_global, {
                    {IROperand::operandType::globalVar, valType},
                    res
                    }}
            });
            return res;
        } catch (std::runtime_error &e) {
            panic(identifier->node.line, identifier->node.col, "Undefined identifier");
        }
        return {};
    }

    yoi::IROperand visitor::visit(yoi::primary *primary) {
        switch (primary->kind) {
            case 0:
                return visit(primary->member);
            case 1:
                return visit(primary->literals);
            case 2:
                return visit(primary->expr);
            default: {
                panic(primary->getLine(), primary->getColumn(), "Unexpected primary type");
            }
        }
    }

    yoi::IROperand visitor::visit(yoi::uniqueExpr *uniqueExpr) {
        auto lhs = visit(&uniqueExpr->getLhs());
        auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
        switch (uniqueExpr->getOp().kind) {
            case lexer::token::tokenKind::incrementSign: {
                assert(lhsType->type == IRValueType::valueType::lvalue, uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue expected for increment");
                // TODO: add support for overloading
                assert(lhs.getLvalueType()->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue type must be basic type for decrement");
                moduleContext->getIRBuilder().insert({IR{IR::Opcode::increment, {lhs}}});
                return lhs;
            }
            case lexer::token::tokenKind::decrementSign: {
                assert(lhsType->type == IRValueType::valueType::lvalue, uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue expected for decrement");
                // TODO: add support for overloading
                assert(lhs.getLvalueType()->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue type must be basic type for decrement");
                moduleContext->getIRBuilder().insert({IR{IR::Opcode::decrement, {lhs}}});
                return lhs;
            }
            case lexer::token::tokenKind::binaryNot: {
                if (lhsType->type == IRValueType::valueType::lvalue) {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                }
                // TODO: add support for overloading
                assert(lhs.getLvalueType()->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Not basic type for bitwise not");
                moduleContext->getIRBuilder().insert({IR{IR::Opcode::bitwiseNot, {lhs, lhs = moduleContext->getIRBuilder().createTempVar(
                        moduleContext->getIRBuilder().extractValueType(lhs))}}});
                return lhs;
            }
            default: {
                panic(uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected unique expression operator");
                return {};
            }
        }
    }

    yoi::IROperand visitor::visit(yoi::mulExpr *mulExpr) {
        auto term = mulExpr->getTerms().begin();
        auto op = mulExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != mulExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType || rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::asterisk: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for multiplication");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer && rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::slash: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && lhsType->isBasicType(), op->line, op->col, "Not basic type for multiplication");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer && rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::div, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::percentSign: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->type == IRValueType::valueType::integer && rhsType->type == IRValueType::valueType::integer, op->line, op->col, "Not basic type for multiplication");
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mod, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected multiplication expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::addExpr *addExpr) {
        auto term = addExpr->getTerms().begin();
        auto op = addExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != addExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::plus: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for addition");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::minus: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for subtraction");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected addition expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::shiftExpr *shiftExpr) {
        auto term = shiftExpr->getTerms().begin();
        auto op = shiftExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != shiftExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryShiftLeft: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->type == IRValueType::valueType::integer &&
                           rhsType->type == IRValueType::valueType::integer, op->line, op->col,
                           "Not basic type for left shift");
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::left_shift, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::binaryShiftRight: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->type == IRValueType::valueType::integer &&
                           rhsType->type == IRValueType::valueType::integer, op->line, op->col,
                           "Not basic type for right shift");
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::right_shift, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected shift expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::relationalExpr *relationalExpr) {
        auto term = relationalExpr->getTerms().begin();
        auto op = relationalExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != relationalExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::lessThan: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for less than");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_than, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::greaterThan: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for greater than");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_than, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::lessEqual: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for less than or equal");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_equal, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::greaterEqual: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for greater than or equal");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_equal, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected relational expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::equalityExpr *equalityExpr) {
        auto term = equalityExpr->getTerms().begin();
        auto op = equalityExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != equalityExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::equal: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for equal");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::equal, lhs, rhs);
                    break;
                }
                case lexer::token::tokenKind::notEqual: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for not equal");
                    if (lhs.type == IROperand::operandType::decimal && rhs.type == IROperand::operandType::integer) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(rhs, lhsType);
                    } else if (lhs.type == IROperand::operandType::integer &&
                               rhs.type == IROperand::operandType::decimal) {
                        // cast
                        moduleContext->getIRBuilder().basicCast(lhs, rhsType);
                    }
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::not_equal, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected equality expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::andExpr *andExpr) {
        auto term = andExpr->getTerms().begin();
        auto op = andExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != andExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryAnd: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->type == IRValueType::valueType::integer &&
                           rhsType->type == IRValueType::valueType::integer, op->line, op->col,
                           "Not basic type for binary and");
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_and, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected and expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::exclusiveExpr *exclusiveExpr) {
        auto term = exclusiveExpr->getTerms().begin();
        auto op = exclusiveExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != exclusiveExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryXor: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->type == IRValueType::valueType::integer &&
                           rhsType->type == IRValueType::valueType::integer, op->line, op->col,
                           "Not basic type for binary xor");
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_xor, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected exclusive expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::inclusiveExpr *inclusiveExpr) {
        auto term = inclusiveExpr->getTerms().begin();
        auto op = inclusiveExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != inclusiveExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            if (lhsType->type == IRValueType::valueType::structType ||
                rhsType->type == IRValueType::valueType::structType) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryOr: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(lhsType->type == IRValueType::valueType::integer &&
                           rhsType->type == IRValueType::valueType::integer, op->line, op->col,
                           "Not basic type for binary or");
                    lhs = moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_or, lhs, rhs);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected inclusive expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::logicalAndExpr *logicalAndExpr) {
        auto term = logicalAndExpr->getTerms().begin();
        auto op = logicalAndExpr->getOp().begin();
        auto lhs = visit(*term);

        for (; op != logicalAndExpr->getOp().end(); ++op) {
            auto resultTmpVar = moduleContext->getIRBuilder().createTempVar(managedPtr(IRValueType{IRValueType::valueType::boolean}));
            auto exitWithTrueBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitWithFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::store, {{IROperand::operandType::boolean, true}, resultTmpVar}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::jump, {IROperand{IROperand::operandType::codeBlock, exitBlock}}});

            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::store, {IROperand{IROperand::operandType::boolean, IROperand::operandValue{false}}, resultTmpVar}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}});

            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            switch (op->kind) {
                case lexer::token::tokenKind::logicAnd: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    assert(lhsType->type == IRValueType::valueType::boolean, op->line, op->col,
                           "Not boolean type for logical and");
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, lhs, exitWithFalseBlock);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(rhsType->type == IRValueType::valueType::boolean, op->line, op->col,
                           "Not boolean type for logical and");
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, rhs, exitWithFalseBlock);
                    moduleContext->getIRBuilder().jumpOp(exitWithTrueBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(exitBlock);
                    lhs = resultTmpVar;
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected logical and expression operator");
                    return {};
                }
            }
        }

        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::logicalOrExpr *logicalOrExpr) {
        auto term = logicalOrExpr->getTerms().begin();
        auto op = logicalOrExpr->getOp().begin();
        auto lhs = visit(*term);

        for (; op != logicalOrExpr->getOp().end(); ++op) {
            auto resultTmpVar = moduleContext->getIRBuilder().createTempVar(managedPtr(IRValueType{IRValueType::valueType::boolean}));
            auto exitWithTrueBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitWithFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::store, {{IROperand::operandType::boolean, true}, resultTmpVar}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}});

            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::store, {{IROperand::operandType::boolean, IROperand::operandValue{false}}, resultTmpVar}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}});

            auto rhs = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().extractValueType(lhs);
            auto rhsType = moduleContext->getIRBuilder().extractValueType(rhs);
            switch (op->kind) {
                case lexer::token::tokenKind::logicOr: {
                    lhs = moduleContext->getIRBuilder().deref(lhs);
                    assert(lhsType->type == IRValueType::valueType::boolean, op->line, op->col,
                           "Not boolean type for logical or");
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, lhs, exitWithTrueBlock);
                    rhs = moduleContext->getIRBuilder().deref(rhs);
                    assert(rhsType->type == IRValueType::valueType::boolean, op->line, op->col,
                           "Not boolean type for logical or");
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, rhs, exitWithTrueBlock);
                    moduleContext->getIRBuilder().jumpOp(exitWithFalseBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(exitBlock);
                    lhs = resultTmpVar;
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected logical or expression operator");
                    return {};
                }
            }
        }
        return lhs;
    }

    yoi::IROperand visitor::visit(yoi::rExpr *rExpr) {
        return visit(&rExpr->getExpr());
    }

    void visitor::visit(yoi::codeBlock *codeBlock) {
        auto block = moduleContext->getIRBuilder().createCodeBlock();
        moduleContext->getIRBuilder().switchCodeBlock(block);
        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().createScope();
        for (auto stmt : codeBlock->getStmts()) {
            visit(stmt);
        }
        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().popScope();
    }

    yoi::IROperand visitor::visit(yoi::memberExpr *memberExpr) {
        auto it = memberExpr->getTerms().begin();
        yoi::indexT targetModule = -1;
        while (it != memberExpr->getTerms().end() && (targetModule = isModuleName(*it, -1)) != -1) {
            it++;
        }
        IROperand lhs;
        if (targetModule == -1) {
            lhs = visit(*it);
        } else {
            lhs = visitExtern(*it, targetModule);
        }

        for (; it != memberExpr->getTerms().end();) {
            auto rhsIt = *++it;
            auto termType = moduleContext->getIRBuilder().extractValueType(lhs);
            assert(termType->type == IRValueType::valueType::structType, (**it).getLine(), (**it).getColumn(), "Not struct type");
            if (rhsIt->isInvocation()) {
                // TODO: method call
            } else if (rhsIt->isSubscript()) {
                // TODO: subscript
            } else {
                auto memberName = rhsIt->id;
                if(memberName->hasTemplateArg()) {
                    // TODO: what the heck is this
                } else {
                    auto nameInfo = irModule->structTable[termType->typeIndex]->lookupName(memberName->getId().get().strVal);
                    switch (nameInfo.type) {
                        case IRStructDefinition::nameInfo::nameType::field: {
                            auto tempVar = moduleContext->getIRBuilder().createTempVar(
                                    managedPtr(IRValueType{IRValueType::valueType::lvalue, irModule->structTable[termType->typeIndex]->fieldTypes[nameInfo.index]}));
                            moduleContext->getIRBuilder().insert(
                                    {IR::Opcode::load_member, {lhs, {IROperand::operandType::index, nameInfo.index}, tempVar}}
                                    );
                            lhs = tempVar;
                            break;
                        }
                        case IRStructDefinition::nameInfo::nameType::method: {
                            panic(rhsIt->getLine(), rhsIt->getColumn(), "Method cannot be parsed without invocation");
                        }
                    }
                }
            }
        }
        return lhs;
    }

    void visitor::visit(yoi::inCodeBlockStmt *inCodeBlockStmt) {
        switch (inCodeBlockStmt->getKind()) {
            case inCodeBlockStmt::vKind::ifStmt:
                visit(inCodeBlockStmt->getValue().ifStmt);
            case inCodeBlockStmt::vKind::whileStmt:
                visit(inCodeBlockStmt->getValue().whileStmt);
            case inCodeBlockStmt::vKind::forStmt:
                visit(inCodeBlockStmt->getValue().forStmt);
            case inCodeBlockStmt::vKind::forEachStmt:
                visit(inCodeBlockStmt->getValue().forEachStmt);
            case inCodeBlockStmt::vKind::returnStmt:
                visit(inCodeBlockStmt->getValue().returnStmt);
            case inCodeBlockStmt::vKind::continueStmt:
                visit(inCodeBlockStmt->getValue().continueStmt);
            case inCodeBlockStmt::vKind::breakStmt:
                visit(inCodeBlockStmt->getValue().breakStmt);
            case inCodeBlockStmt::vKind::letStmt:
                visit(inCodeBlockStmt->getValue().letStmt);
            case inCodeBlockStmt::vKind::codeBlock:
                visit(inCodeBlockStmt->getValue().codeBlock);
            case inCodeBlockStmt::vKind::rExpr:
                visit(inCodeBlockStmt->getValue().rExpr);
        }
    }

    /*
    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::identifier *identifier) {
        auto &id = identifier->node.strVal;
        try {
            auto valType = moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable()[id];
            return valType;
        } catch (std::runtime_error &e) {

        }
        try {
            auto valType = irModule->globalVariables[id];
            return valType;
        } catch (std::runtime_error &e) {
            panic(identifier->node.line, identifier->node.col, "Undefined identifier");
        }
        return {};
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
            return {};
        } else {
            return getExprTypeInfo(identifierWithTemplateArg->id);
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::subscriptExpr *subscriptExpr) {
        if (subscriptExpr->isSubscript()) {
            // TODO: subscript
            return {};
        } else if (subscriptExpr->isInvocation()) {
            // TODO: method call
            return {};
        } else {
            return getExprTypeInfo(subscriptExpr->id);
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::memberExpr *memberExpr) {
        auto it = memberExpr->getTerms().begin();
        yoi::indexT targetModule = -1;
        while (it != memberExpr->getTerms().end() && (targetModule = isModuleName(*it, -1)) != -1) {
            it++;
        }
        std::shared_ptr<IRValueType> termType;
        if (targetModule == -1) {
            termType = getExprTypeInfo(*it);
        } else {
            termType = getExternType(*it, targetModule);
        }

        for (; it != memberExpr->getTerms().end();) {
            auto rhsIt = *++it;
            assert(termType->type == IRValueType::valueType::structType, rhsIt->getLine(), rhsIt->getColumn(), "Not struct type");
            if (rhsIt->isInvocation()) {
                // TODO: method call
            } else if (rhsIt->isSubscript()) {
                // TODO: subscript
            } else {
                auto memberName = rhsIt->id;
                if(memberName->hasTemplateArg()) {
                    // TODO: what the heck is this
                } else {
                    auto nameInfo = irModule->structTable[termType->typeIndex]->lookupName(memberName->getId().get().strVal);
                    switch (nameInfo.type) {
                        case IRStructDefinition::nameInfo::nameType::field: {
                            termType = managedPtr(IRValueType{IRValueType::valueType::lvalue, irModule->structTable[termType->typeIndex]->fieldTypes[nameInfo.index]});
                            break;
                        }
                        case IRStructDefinition::nameInfo::nameType::method: {
                            // TODO: method type
                            panic(rhsIt->getLine(), rhsIt->getColumn(), "Method cannot be parsed without invocation");
                            break;
                        }
                    }
                }
            }
        }
        return termType;
    }

    std::shared_ptr<yoi::IRValueType> visitor::getExprTypeInfo(yoi::basicLiterals *primary) {
        switch (primary->node.kind) {
            case lexer::token::tokenKind::integer: {
                return managedPtr(IRValueType{IRValueType::valueType::integer});
            }
            case lexer::token::tokenKind::boolean: {
                return managedPtr(IRValueType{IRValueType::valueType::boolean});
            }
            case lexer::token::tokenKind::decimal: {
                return managedPtr(IRValueType{IRValueType::valueType::decimal});
            }
            case lexer::token::tokenKind::string: {
                // TODO: string type
            }
            case lexer::token::tokenKind::character: {
                return managedPtr(IRValueType{IRValueType::valueType::character});
            }
            default: {
                panic(primary->getLine(), primary->getColumn(), "Unsupported literal type");
            }
        }
        return {};
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::primary *primary) {
        switch (primary->kind) {
            case 0: {
                // member expr
                return getExprTypeInfo(primary->member);
            }
            case 1: {
                // basic literals
                return getExprTypeInfo(primary->literals);
            }
            case 2: {
                // rExpr
                return getExprTypeInfo(primary->expr);
            }
        }
        return {};
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::uniqueExpr *uniqueExpr) {
        switch (uniqueExpr->op.kind) {
            case lexer::token::tokenKind::incrementSign:
            case lexer::token::tokenKind::decrementSign: {
                return getExprTypeInfo(uniqueExpr->lhs);
            }
            case lexer::token::tokenKind::binaryNot: {
                auto l = getExprTypeInfo(uniqueExpr->lhs);
                if (l->type == IRValueType::valueType::lvalue) {
                    return l->lvalueType;
                } else {
                    return l;
                }
            }
            default: {
                panic(uniqueExpr->getLine(), uniqueExpr->getColumn(), "Unsupported unique expression operator");
            }
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::mulExpr *mulExpr) {
        auto it = mulExpr->getTerms().begin();
        auto op = mulExpr->getOp().begin();
        auto lhs = getExprTypeInfo(*it);
        for (; op != mulExpr->getOp().end(); ++op) {
            auto rhs = getExprTypeInfo(*++it);
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structType or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structType) {
                // TODO: overload
            }
            if (lhs->type == IRValueType::valueType::decimal) {
                // pass
            } else if (rhs->type == IRValueType::valueType::decimal) {
                lhs = rhs;
            }
        }
        return lhs;
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::addExpr *addExpr) {
        auto it = addExpr->getTerms().begin();
        auto op = addExpr->getOp().begin();
        auto lhs = getExprTypeInfo(*it);
        for (; op != addExpr->getOp().end(); ++op) {
            auto rhs = getExprTypeInfo(*++it);
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structType or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structType) {
                // TODO: overload
            }
            if (lhs->type == IRValueType::valueType::lvalue)
                lhs = lhs->lvalueType;
            if (rhs->type == IRValueType::valueType::lvalue)
                rhs = rhs->lvalueType;

            if (lhs->type == IRValueType::valueType::decimal) {
                // pass
            } else if (rhs->type == IRValueType::valueType::decimal) {
                lhs = rhs;
            }
        }
        return lhs;
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::shiftExpr *shiftExpr) {
        auto it = shiftExpr->getTerms().begin();
        auto op = shiftExpr->getOp().begin();
        auto lhs = getExprTypeInfo(*it);
        for (; op != shiftExpr->getOp().end(); ++op) {
            auto rhs = getExprTypeInfo(*++it);
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structType or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structType) {
                // TODO: overload
            }
            if (lhs->type == IRValueType::valueType::lvalue)
                lhs = lhs->lvalueType;
            if (rhs->type == IRValueType::valueType::lvalue)
                rhs = rhs->lvalueType;
            assert(lhs->type == IRValueType::valueType::integer && rhs->type == IRValueType::valueType::integer, shiftExpr->getLine(), shiftExpr->getColumn(), "Invalid shift expression");
            lhs = rhs;
        }
        return lhs;
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::relationalExpr *relationalExpr) {
        if (relationalExpr->getOp().empty()) {
            // follow the first term
            return getExprTypeInfo(relationalExpr->getTerms().front());
        } else {
            // TODO: always booleans except for overload, but we don't care for now
            return managedPtr(IRValueType{IRValueType::valueType::boolean});
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::equalityExpr *equalityExpr) {
        if (equalityExpr->getOp().empty()) {
            // follow the first term
            return getExprTypeInfo(equalityExpr->getTerms().front());
        } else {
            // TODO: always booleans except for overload, but we don't care for now
            return managedPtr(IRValueType{IRValueType::valueType::boolean});
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::andExpr *andExpr) {
        if (andExpr->getOp().empty()) {
            // follow the first term
            return getExprTypeInfo(andExpr->getTerms().front());
        } else {
            // TODO: always booleans except for overload, but we don't care for now
            return managedPtr(IRValueType{IRValueType::valueType::boolean});
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::exclusiveExpr *exclusiveExpr) {
        auto it = exclusiveExpr->getTerms().begin();
        auto op = exclusiveExpr->getOp().begin();
        auto lhs = getExprTypeInfo(*it);
        for (; op != exclusiveExpr->getOp().end(); ++op) {
            auto rhs = getExprTypeInfo(*++it);
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structType or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structType) {
                // TODO: overload
                }
            if (lhs->type == IRValueType::valueType::lvalue)
                lhs = lhs->lvalueType;
            if (rhs->type == IRValueType::valueType::lvalue)
                rhs = rhs->lvalueType;
            assert(lhs->type == IRValueType::valueType::integer && rhs->type == IRValueType::valueType::integer, shiftExpr->getLine(), shiftExpr->getColumn(), "Invalid shift expression");
            lhs = rhs;
        }
        return lhs;
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::inclusiveExpr *inclusiveExpr) {
        auto it = inclusiveExpr->getTerms().begin();
        auto op = inclusiveExpr->getOp().begin();
        auto lhs = getExprTypeInfo(*it);
        for (; op != inclusiveExpr->getOp().end(); ++op) {
            auto rhs = getExprTypeInfo(*++it);
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structType or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structType) {
                // TODO: overload
                }
            if (lhs->type == IRValueType::valueType::lvalue)
                lhs = lhs->lvalueType;
            if (rhs->type == IRValueType::valueType::lvalue)
                rhs = rhs->lvalueType;
            assert(lhs->type == IRValueType::valueType::integer && rhs->type == IRValueType::valueType::integer, inclusiveExpr->getLine(), inclusiveExpr->getColumn(), "Invalid shift expression");
            lhs = rhs;
        }
        return lhs;
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::logicalAndExpr *logicalAndExpr) {
        if (logicalAndExpr->getOp().empty()) {
            // follow the first term
            return getExprTypeInfo(logicalAndExpr->getTerms().front());
        } else {
            // TODO: always booleans except for overload, but we don't care for now
            return managedPtr(IRValueType{IRValueType::valueType::boolean});
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::logicalOrExpr *logicalOrExpr) {
        if (logicalOrExpr->getOp().empty()) {
            // follow the first term
            return getExprTypeInfo(logicalOrExpr->getTerms().front());
        } else {
            // TODO: always booleans except for overload, but we don't care for now
            return managedPtr(IRValueType{IRValueType::valueType::boolean});
        }
    }

    std::shared_ptr<IRValueType> visitor::getExprTypeInfo(yoi::rExpr *rExpr) {
        return getExprTypeInfo(rExpr->expr);
    }
    */

    yoi::IROperand visitor::visit(yoi::subscriptExpr *subscriptExpr) {
        if (subscriptExpr->isSubscript()) {
            // TODO: subscript
            return {};
        } else if (subscriptExpr->isInvocation()) {
            // TODO: method call
            return {};
        } else {
            return visit(subscriptExpr->id);
        }
    }

    yoi::IROperand visitor::visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
            return {};
        } else {
            return visit(identifierWithTemplateArg->id);
        }
    }

    void visitor::visit(yoi::useStmt *useStmt) {
        // TODO: dummy
    }

    IRValueType visitor::parseTypeSpec(yoi::identifier *identifier) {
        auto &typeName = identifier->node.strVal;
        try {
            auto typeIndex = irModule->structTable.getIndex(typeName);
            return IRValueType{IRValueType::valueType::structType, typeIndex};
        } catch(std::runtime_error &e) {
                panic(identifier->getLine(), identifier->getColumn(), "Undefined type: " + wstring2string(typeName));
        }
    }

    void visitor::visit(yoi::funcDefStmt *funcDefStmt) {
        auto funcName = funcDefStmt->getId();
        if (funcName.hasDefTemplateArg()) {
            // TODO: function template
        } else {
            auto funcType = parseTypeSpec(&funcDefStmt->getResultType());
            std::vector<std::shared_ptr<IRValueType>> argTypes;
            std::shared_ptr<IRFunctionDefinition> func = std::make_shared<IRFunctionDefinition>(funcName.getId().node.strVal, argTypes, managedPtr(funcType));

        }
    }

    yoi::IROperand visitor::visit(yoi::interfaceDefStmt *interfaceDefStmt) {

    }

    yoi::IROperand visitor::visit(yoi::structDefStmt *structDefStmt) {
        // TODO: implement struct definition
        panic(structDefStmt->getLine(), structDefStmt->getColumn(), "Struct definition is not implemented yet");
    }

    yoi::IROperand visitor::visit(yoi::implStmt *implStmt) {
        // TODO: implement implementation
        panic(implStmt->getLine(), implStmt->getColumn(), "Implementation definition is not implemented yet");
    }

    yoi::IROperand visitor::visit(yoi::letStmt *letStmt) {
        for (auto &i : letStmt->terms) {
            auto operand = visit(i->rhs);
            auto type = moduleContext->getIRBuilder().extractValueType(operand);
            if (isVisitingGlobalScope()) {
                // global variable
                irModule->globalVariables.put(i->lhs->node.strVal, type);
            } else {
                moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().put(i->lhs->node.strVal, type);
            }
        }
    }

    void visitor::visit(yoi::globalStmt *globalStmt) {
        switch (globalStmt->kind) {
            case globalStmt::vKind::useStmt: {
                visit(globalStmt->value.useStmt);
                break;
            }
            case globalStmt::vKind::implStmt: {
                visit(globalStmt->value.implStmt);
                break;
            }
            case globalStmt::vKind::letStmt: {
                visit(globalStmt->value.letStmt);
                break;
            }
            case globalStmt::vKind::funcDefStmt: {
                visit(globalStmt->value.funcDefStmt);
                break;
            }
            case globalStmt::vKind::structDefStmt: {
                visit(globalStmt->value.structDefStmt);
                break;
            }
            case globalStmt::vKind::interfaceDefStmt: {
                visit(globalStmt->value.interfaceDefStmt);
                break;
            }
            default: {
                panic(globalStmt->getLine(), globalStmt->getColumn(), "Unsupported global statement type");
            }
        }
    }

    void visitor::visit(yoi::ifStmt *ifStmt) {
        auto cond = visit(ifStmt->getIfBlock().cond);
        cond = moduleContext->getIRBuilder().deref(cond);
        auto condType = moduleContext->getIRBuilder().extractValueType(cond);
        assert(condType->type == IRValueType::valueType::boolean, ifStmt->getLine(), ifStmt->getColumn(), "The type in if-condition must be boolean");

        auto ifBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, cond, ifBlock);
        auto back = moduleContext->getIRBuilder().switchCodeBlock(ifBlock);
        visit(ifStmt->getIfBlock().block);
        moduleContext->getIRBuilder().switchCodeBlock(back);

        for (auto &i : ifStmt->elifB) {
            auto elifCond = visit(i.cond);
            elifCond = moduleContext->getIRBuilder().deref(elifCond);
            auto elifCondType = moduleContext->getIRBuilder().extractValueType(elifCond);
            assert(elifCondType->type == IRValueType::valueType::boolean, i.cond->getLine(), i.cond->getColumn(), "The type in elif-condition must be boolean");

            auto elifBlock = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, elifCond, elifBlock);
            auto back = moduleContext->getIRBuilder().switchCodeBlock(elifBlock);
            visit(i.block);
            moduleContext->getIRBuilder().switchCodeBlock(back);
        }

        if (ifStmt->hasElseBlock()) {
            auto elseBlock = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpOp(elseBlock);
            auto back = moduleContext->getIRBuilder().switchCodeBlock(elseBlock);
            visit(ifStmt->elseB);
            moduleContext->getIRBuilder().switchCodeBlock(back);
        }
    }

    void visitor::visit(yoi::whileStmt *whileStmt) {
        auto condBlock = moduleContext->getIRBuilder().createCodeBlock();
        moduleContext->getIRBuilder().jumpOp(condBlock);
        auto back = moduleContext->getIRBuilder().switchCodeBlock(condBlock);

        auto cond = visit(whileStmt->cond);
        cond = moduleContext->getIRBuilder().deref(cond);
        auto condType = moduleContext->getIRBuilder().extractValueType(cond);
        assert(condType->type == IRValueType::valueType::boolean, whileStmt->getLine(), whileStmt->getColumn(), "The type in while-condition must be boolean");

        auto whileBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto outBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, cond, whileBlock);
        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, cond, outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(whileBlock);
        visit(whileStmt->block);

        // replace dummy_break and dummy_continue with jump to the cond block
        for (auto &i : moduleContext->getIRBuilder().getCurrentCodeBlock().getIRArray()) {
            if (i.opcode == IR::Opcode::dummy_break) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, outBlock}}};
            } else if (i.opcode == IR::Opcode::dummy_continue) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, condBlock}}};
            }
        }

        moduleContext->getIRBuilder().jumpOp(condBlock);
        moduleContext->getIRBuilder().switchCodeBlock(outBlock);
    }

    void visitor::visit(yoi::forStmt *forStmt) {
        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().createScope();
        auto initBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto condBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto codeBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto afterBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto outBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpOp(initBlock);

        moduleContext->getIRBuilder().switchCodeBlock(initBlock);
        visit(forStmt->initStmt);

        moduleContext->getIRBuilder().jumpOp(condBlock);
        moduleContext->getIRBuilder().switchCodeBlock(condBlock);
        auto cond = visit(forStmt->cond);
        cond = moduleContext->getIRBuilder().deref(cond);
        auto condType = moduleContext->getIRBuilder().extractValueType(cond);
        assert(condType->type == IRValueType::valueType::boolean, forStmt->getLine(), forStmt->getColumn(), "The type in for-condition must be boolean");

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, cond, codeBlock);
        moduleContext->getIRBuilder().jumpOp(outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(codeBlock);
        visit(forStmt->block);

        // replace dummy_break and dummy_continue with jump to the cond block
        for (auto &i : moduleContext->getIRBuilder().getCurrentCodeBlock().getIRArray()) {
            if (i.opcode == IR::Opcode::dummy_break) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, outBlock}}};
            } else if (i.opcode == IR::Opcode::dummy_continue) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, codeBlock}}};
            }
        }

        moduleContext->getIRBuilder().jumpOp(afterBlock);
        moduleContext->getIRBuilder().switchCodeBlock(afterBlock);
        visit(forStmt->afterStmt);

        moduleContext->getIRBuilder().jumpOp(condBlock);
        moduleContext->getIRBuilder().switchCodeBlock(outBlock);

        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().popScope();
    }

    void visitor::visit(yoi::forEachStmt *forEachStmt) {
        // TODO: implement foreach statement
        panic(forEachStmt->getLine(), forEachStmt->getColumn(), "forEach statement is not implemented yet");
    }

    yoi::IROperand visitor::visit(yoi::returnStmt *returnStmt) {
        if (returnStmt->hasValue()) {
            auto operand = visit(returnStmt->value);
            auto type = moduleContext->getIRBuilder().extractValueType(operand);
            moduleContext->getIRBuilder().insert({IR::Opcode::ret, {operand}});
        } else {
            moduleContext->getIRBuilder().insert({IR::Opcode::ret, {}});
        }
    }

    yoi::IROperand visitor::visit(yoi::continueStmt *continueStmt) {
        moduleContext->getIRBuilder().insert({IR::Opcode::dummy_continue, {}});
    }

    yoi::IROperand visitor::visit(yoi::breakStmt *breakStmt) {
        moduleContext->getIRBuilder().insert({IR::Opcode::dummy_break, {}});
    }

    IRValueType visitor::parseTypeSpec(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
            return {IRValueType::valueType::null};
        } else {
            return parseTypeSpec(identifierWithTemplateArg->id);
        }
    }

    IRValueType visitor::parseTypeSpec(yoi::subscriptExpr *subscriptExpr) {
        if (subscriptExpr->isInvocation()) {
            // TODO: method call
            return {IRValueType::valueType::null};
        } else if (subscriptExpr->isSubscript()) {
            // TODO: subscript
            return {IRValueType::valueType::null};
        } else {
            return parseTypeSpec(subscriptExpr->id);
        }
    }

    IRValueType visitor::parseTypeSpecExtern(yoi::identifier *identifier, yoi::indexT targetModule) {
        auto &mod = moduleContext->getCompilerContext()->getIRObjectFile()->modules[targetModule];
        auto exId = addExternEntryIfNotExists(targetModule, identifier);
        auto ex = irModule->externTable[exId];
        assert(ex->type == IRExternEntry::externType::structType, identifier->getLine(), identifier->getColumn(), "Invalid type specifier, expected struct type");
        return {IRValueType::valueType::structType, exId};
    }

    IRValueType visitor::parseTypeSpecExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg,
                                             yoi::indexT targetModule) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
            return {IRValueType::valueType::null};
        } else {
            return parseTypeSpecExtern(identifierWithTemplateArg->id, targetModule);
        }
    }

    IRValueType visitor::parseTypeSpecExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule) {
        if (subscriptExpr->isInvocation()) {
            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Invalid type specifier with function invocation");
        } else if (subscriptExpr->isSubscript()) {
            // TODO: subscript
            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Invalid type specifier with subscript");
        } else {
            return parseTypeSpecExtern(subscriptExpr->id, targetModule);
        }
        return {IRValueType::valueType::null};
    }

    IRValueType visitor::parseTypeSpec(yoi::typeSpec *typeSpec) {
        switch (typeSpec->kind) {
            case 0: {
                // member
                auto it = typeSpec->member->getTerms().begin();
                yoi::indexT targetModule = -1;
                while (it != typeSpec->member->getTerms().end() && (targetModule = isModuleName(*it, -1)) != -1) {
                    it++;
                }

                IRValueType lhs{IRValueType::valueType::integer};
                if (targetModule == -1) {
                    lhs = parseTypeSpec(*it);
                } else {
                    lhs = parseTypeSpecExtern(*it, targetModule);
                }
                assert(++it != typeSpec->member->getTerms().end(), typeSpec->getLine(), typeSpec->getColumn(), "Type specifier is not valid.");
                return lhs;
            }
            case 1: {
                // func
                // TODO: Implement function type
                return {IRValueType::valueType::null};
            }
            case 2: {
                // null
                return {IRValueType::valueType::null};
            }
            default: {
                panic(typeSpec->getLine(), typeSpec->getColumn(), "Type specifier is not valid.");
                return {IRValueType::valueType::null};
            }
        }
    }

    yoi::indexT visitor::isModuleName(subscriptExpr *it, yoi::indexT currentModule) const {
        if(it->isIdentifier() && !it->getId().hasTemplateArg()) {
            std::shared_ptr<yoi::IRModule> target = currentModule == -1 ? irModule : moduleContext->getCompilerContext()->getIRObjectFile()->modules[currentModule];
            if (auto x = target->moduleImports.find(it->getId().id->node.strVal); x != target->moduleImports.end() ) {
                return moduleContext->getCompilerContext()->getIRObjectFile()->modules.getIndex(x->second);
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    yoi::IRExternEntry visitor::getExternEntry(yoi::indexT moduleIndex, yoi::identifier *identifier) const {
        try {
            auto res = moduleContext->getCompilerContext()->getIRObjectFile()->modules[moduleIndex]->globalVariables.getIndex(identifier->node.strVal);
            return {IRExternEntry::externType::globalVar, identifier->node.strVal, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getIRObjectFile()->modules[moduleIndex]->functionTable.getIndex(identifier->node.strVal);
            return {IRExternEntry::externType::function, identifier->node.strVal, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getIRObjectFile()->modules[moduleIndex]->structTable.getIndex(identifier->node.strVal);
            return {IRExternEntry::externType::structType, identifier->node.strVal, moduleIndex, res};
        } catch (std::runtime_error &) {}
        panic(0, 0, "undefined identifier");
        return {};
    }


    yoi::indexT visitor::addExternEntryIfNotExists(yoi::indexT moduleIndex, yoi::identifier *identifier) {
        // extern entry format: moduleIndex#identifier
        yoi::wstr key = std::to_wstring(moduleIndex) + L"#" + identifier->node.strVal;
        try {
            auto it = irModule->externTable.getIndex(key);
            return it;
        }  catch (std::runtime_error &) {
            // not found, add a new entry
            return irModule->externTable.put(key, managedPtr(getExternEntry(moduleIndex, identifier)));
        }
    }

    bool visitor::isVisitingGlobalScope() const {
        return moduleContext->getIRBuilder().irFuncDefinition()->name == L"glob_initializer";
    }

    yoi::IROperand visitor::visitExtern(yoi::identifier *identifier, yoi::indexT targetModule) {
        try {
            auto entry = addExternEntryIfNotExists(targetModule, identifier);
            moduleContext->getIRBuilder().insert({IR::Opcode::load_extern,{{IROperand::operandType::index, entry}}});
        } catch (std::runtime_error &) {
            // not found, panic
            panic(identifier->getLine(), identifier->getColumn(), "undefined identifier: " + wstring2string(identifier->node.strVal));
            return {};
        }
    }

    yoi::IROperand
    visitor::visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
        } else {
            return visitExtern(identifierWithTemplateArg->id, targetModule);
        }
    }

    yoi::IROperand visitor::visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule) {
        if (subscriptExpr->isSubscript()) {
            // TODO: subscript
        } else if (subscriptExpr->isInvocation()) {
            // TODO: method call
        } else {
            return visitExtern(subscriptExpr->id, targetModule);
        }
    }
} // yoi