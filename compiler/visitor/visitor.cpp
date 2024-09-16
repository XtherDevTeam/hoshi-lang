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

            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::store, {{IROperand::operandType::boolean, false}, resultTmpVar}});
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

            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::store, {{IROperand::operandType::boolean, false}, resultTmpVar}});
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
        auto lhs = visit(*it);
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
    }

    yoi::IROperand visitor::visit(yoi::inCodeBlockStmt *inCodeBlockStmt) {
        switch (inCodeBlockStmt->getKind()) {
            case inCodeBlockStmt::vKind::ifStmt:
                visit(inCodeBlockStmt->getValue().ifStmt);
                return {};
            case inCodeBlockStmt::vKind::whileStmt:
                visit(inCodeBlockStmt->getValue().whileStmt);
                return {};
            case inCodeBlockStmt::vKind::forStmt:
                visit(inCodeBlockStmt->getValue().forStmt);
                return {};
            case inCodeBlockStmt::vKind::forEachStmt:
                visit(inCodeBlockStmt->getValue().forEachStmt);
                return {};
            case inCodeBlockStmt::vKind::returnStmt:
                visit(inCodeBlockStmt->getValue().returnStmt);
                return {};
            case inCodeBlockStmt::vKind::continueStmt:
                visit(inCodeBlockStmt->getValue().continueStmt);
                return {};
            case inCodeBlockStmt::vKind::breakStmt:
                visit(inCodeBlockStmt->getValue().breakStmt);
                return {};
            case inCodeBlockStmt::vKind::letStmt:
                visit(inCodeBlockStmt->getValue().letStmt);
                return {};
            case inCodeBlockStmt::vKind::codeBlock:
                visit(inCodeBlockStmt->getValue().codeBlock);
                return {};
            case inCodeBlockStmt::vKind::rExpr:
                return visit(inCodeBlockStmt->getValue().rExpr);
        }
    }

    yoi::IROperand visitor::visit(yoi::subscriptExpr *subscriptExpr) {
        if (subscriptExpr->isSubscript()) {
            // TODO: subscript
        } else if (subscriptExpr->isInvocation()) {
            // TODO: method call
        } else {
            return visit(subscriptExpr->id);
        }
    }

    yoi::IROperand visitor::visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
        } else {
            return visit(identifierWithTemplateArg->id);
        }
    }
} // yoi