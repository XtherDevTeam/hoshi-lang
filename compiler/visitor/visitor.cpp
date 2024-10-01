//
// Created by XIaokang00010 on 2024/9/6.
//

#include "visitor.h"

namespace yoi {
    visitor::visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext,
                     const std::shared_ptr<yoi::IRModule> &irModule) : moduleContext(moduleContext), irModule(irModule) {

    }

    std::shared_ptr<yoi::IRModule> visitor::visit() {
        auto globInitializer = managedPtr(IRFunctionDefinition{L"yoimiya_glob_initializer", {}, managedPtr(moduleContext->getCompilerContext()->getIntObjectType())});
        irModule->functionTable.put(L"yoimiya_glob_initializer", globInitializer);
        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, globInitializer});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(&moduleContext->getModuleAST());
        moduleContext->getIRBuilder().pushOp(IR::Opcode::push_integer, {IROperand::operandType::integer, IROperand::operandValue(0ull)});
        moduleContext->getIRBuilder().retOp();
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();
        return irModule;
    }

    void visitor::visit(yoi::hoshiModule *module) {
        for (auto &stmt : module->stmts) {
            visit(stmt);
        }
    }

    yoi::indexT visitor::visit(yoi::basicLiterals *basicLiterals) {
        switch (basicLiterals->node.kind) {
            case yoi::lexer::token::tokenKind::integer: {
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_integer, {IROperand::operandType::integer,basicLiterals->node.basicVal.vInt});
                break;
            }
            case yoi::lexer::token::tokenKind::decimal: {
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_decimal, {IROperand::operandType::decimal,basicLiterals->node.basicVal.vDeci});
                break;
            }
            case yoi::lexer::token::tokenKind::string: {
                auto literalIndex = irModule->stringLiteralPool.addStringLiteral(basicLiterals->node.strVal);
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_string, {IROperand::operandType::stringLiteral, literalIndex});
                break;
            }
            case yoi::lexer::token::tokenKind::boolean: {
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_boolean, {IROperand::operandType::boolean,basicLiterals->node.basicVal.vBool});
                break;
            }
            case yoi::lexer::token::tokenKind::character: {
                // TODO: add support for character literals
                panic(basicLiterals->node.line, basicLiterals->node.col, "Unsupported character literal");
                break;
            }
            default: {
                panic(basicLiterals->node.line, basicLiterals->node.col, "Unexpected basic literal type");
                break;
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::identifier *identifier, bool isStoreOp) {
        auto &id = identifier->node.strVal;
        try {
            auto index = moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().lookup(id);
            auto valType = moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().get(index);
            if (isStoreOp) {
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local, {IROperand::operandType::localVar, yoi::indexT{index}});
            } else {
                moduleContext->getIRBuilder().loadOp(IR::Opcode::load_local, {IROperand::operandType::localVar, yoi::indexT{index}}, valType);
            }
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } catch (std::runtime_error &e) {
            // let it go, try to find it in global variables
        }
        try {
            auto index = irModule->globalVariables.getIndex(id);
            auto valType = irModule->globalVariables[index];
            if (isStoreOp) {
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global, {IROperand::operandType::globalVar, yoi::indexT{index}});
            } else {
                moduleContext->getIRBuilder().loadOp(IR::Opcode::load_global, {IROperand::operandType::globalVar, yoi::indexT{index}}, valType);
            }
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } catch (std::runtime_error &e) {
            panic(identifier->node.line, identifier->node.col, "Undefined identifier: " + wstring2string(id));
        }
        // TODO: add support for extern variables

    }

    yoi::indexT visitor::visit(yoi::primary *primary, bool isStoreOp) {
        switch (primary->kind) {
            case 0:
                visit(primary->member, isStoreOp);
                break;
            case 1:
                visit(primary->literals);
                break;
            case 2:
                visit(primary->expr);
                break;
            default: {
                panic(primary->getLine(), primary->getColumn(), "Unexpected primary type");
            }
        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::uniqueExpr *uniqueExpr, bool isStoreOp) {
        visit(uniqueExpr->lhs, isStoreOp);
        auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        switch (uniqueExpr->getOp().kind) {
            case lexer::token::tokenKind::incrementSign: {
                // TODO: add support for overloading
                yoi_assert(lhs->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue type must be basic type for decrement");
                moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::increment);
                break;
            }
            case lexer::token::tokenKind::decrementSign: {
                // TODO: add support for overloading
                yoi_assert(lhs->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue type must be basic type for decrement");
                moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::decrement);
                break;
            }
            case lexer::token::tokenKind::binaryNot: {
                // TODO: add support for overloading
                yoi_assert(lhs->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Not basic type for bitwise not");
                moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::bitwiseNot);
                break;
            }
            case lexer::token::tokenKind::unknown: {
                // no op now
                break;
            }
            default: {
                panic(uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Unexpected unique expression operator");
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::leftExpr *leftExpr) {
        if (leftExpr->hasRhs()) {
            switch (leftExpr->getOp().kind) {
                case lexer::token::tokenKind::assignSign: {
                    visit(leftExpr->rhs);
                    visit(leftExpr->lhs, true);
                    break;
                }
                case lexer::token::tokenKind::additionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->type == IRValueType::valueType::structObject || rhs->type == IRValueType::valueType::structObject) {
                        // TODO: add support for overloading
                    }
                    yoi_assert(lhs->isBasicType() && rhs->isBasicType(), leftExpr->getOp().line, leftExpr->getOp().col, "Not basic type for addition");
                    if (lhs->type == IRValueType::valueType::decimalObject && rhs->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                    } else if (lhs->type == IRValueType::valueType::integerObject && rhs->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                    visit(leftExpr->lhs, true);
                    break;
                }
                case lexer::token::tokenKind::subtractionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->type == IRValueType::valueType::structObject || rhs->type == IRValueType::valueType::structObject) {
                        // TODO: add support for overloading
                    }
                    yoi_assert(lhs->isBasicType() && rhs->isBasicType(), leftExpr->getOp().line, leftExpr->getOp().col, "Not basic type for subtraction");
                    if (lhs->type == IRValueType::valueType::decimalObject && rhs->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                    } else if (lhs->type == IRValueType::valueType::integerObject && rhs->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhs, lhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub);
                    visit(leftExpr->lhs, true);
                    break;
                }
                case lexer::token::tokenKind::multiplicationAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->type == IRValueType::valueType::structObject || rhs->type == IRValueType::valueType::structObject) {
                        // TODO: add support for overloading
                    }
                    yoi_assert(lhs->isBasicType() && rhs->isBasicType(), leftExpr->getOp().line, leftExpr->getOp().col, "Not basic type for multiplication");
                    if (lhs->type == IRValueType::valueType::decimalObject && rhs->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                    } else if (lhs->type == IRValueType::valueType::integerObject && rhs->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhs, lhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                    visit(leftExpr->lhs, true);
                    break;
                }
                case lexer::token::tokenKind::divisionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->type == IRValueType::valueType::structObject || rhs->type == IRValueType::valueType::structObject) {
                        // TODO: add support for overloading
                    }
                    yoi_assert(lhs->isBasicType() && rhs->isBasicType(), leftExpr->getOp().line, leftExpr->getOp().col, "Not basic type for division");
                    if (lhs->type == IRValueType::valueType::decimalObject && rhs->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                    } else if (lhs->type == IRValueType::valueType::integerObject && rhs->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhs, lhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::div);
                    visit(leftExpr->lhs, true);
                    break;
                }
                default: {
                    panic(leftExpr->getOp().line, leftExpr->getOp().col, "Unexpected left expression operator, received: " + wstring2string(leftExpr->getOp().strVal));
                }
            }
        }
        visit(leftExpr->lhs);
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::mulExpr *mulExpr) {
        auto term = mulExpr->getTerms().begin();
        auto op = mulExpr->getOp().begin();
        auto lhsPos = visit(*term); // lhs
        for (; op != mulExpr->getOp().end(); ++op) {
            auto rhsPos = visit(*++term); // rhs
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject || rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::asterisk: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for multiplication");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                    break;
                }
                case lexer::token::tokenKind::slash: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for multiplication");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::div);
                    break;
                }
                case lexer::token::tokenKind::percentSign: {
                    yoi_assert(lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::integerObject, op->line, op->col, "Not basic type for multiplication");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mod);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected multiplication expression operator");
                }
            }
            lhsPos = rhsPos;
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::addExpr *addExpr) {
        auto term = addExpr->getTerms().begin();
        auto op = addExpr->getOp().begin();
        auto lhsPos = visit(*term);
        for (; op != addExpr->getOp().end(); ++op) {
            auto rhsPos = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::plus: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for addition");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                    break;
                }
                case lexer::token::tokenKind::minus: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for subtraction");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected addition expression operator");
                }
            }
            lhsPos = rhsPos;
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::shiftExpr *shiftExpr) {
        auto term = shiftExpr->getTerms().begin();
        auto op = shiftExpr->getOp().begin();
        visit(*term);
        for (; op != shiftExpr->getOp().end(); ++op) {
            visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryShiftLeft: {
                    yoi_assert(lhsType->type == IRValueType::valueType::integerObject &&
                           rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for left shift");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::left_shift);
                    break;
                }
                case lexer::token::tokenKind::binaryShiftRight: {
                    yoi_assert(lhsType->type == IRValueType::valueType::integerObject &&
                           rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for right shift");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::right_shift);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected shift expression operator");
                    break;
                }
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::relationalExpr *relationalExpr) {
        auto term = relationalExpr->getTerms().begin();
        auto op = relationalExpr->getOp().begin();
        auto lhsPos = visit(*term);
        for (; op != relationalExpr->getOp().end(); ++op) {
            auto rhsPos = visit(*++term);
            auto lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::lessThan: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for less than");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_than);
                    break;
                }
                case lexer::token::tokenKind::greaterThan: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for greater than");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_than);
                    break;
                }
                case lexer::token::tokenKind::lessEqual: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for less than or equal");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_equal);
                    break;
                }
                case lexer::token::tokenKind::greaterEqual: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for greater than or equal");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_equal);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected relational expression operator");
                }
            }
            lhsPos = rhsPos;
        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::equalityExpr *equalityExpr) {
        auto term = equalityExpr->getTerms().begin();
        auto op = equalityExpr->getOp().begin();
        auto lhsPos = visit(*term);
        for (; op != equalityExpr->getOp().end(); ++op) {
            auto rhsPos = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::equal: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for equal");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::equal);
                    break;
                }
                case lexer::token::tokenKind::notEqual: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for not");
                    if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(lhsType, rhsPos);
                    } else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
                        moduleContext->getIRBuilder().basicCast(rhsType, lhsPos, true);
                    }
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::not_equal);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected equality expression operator");
                    return {};
                }
            }
            lhsPos = rhsPos;
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::andExpr *andExpr) {
        auto term = andExpr->getTerms().begin();
        auto op = andExpr->getOp().begin();
        auto lhsPos = visit(*term);
        for (; op != andExpr->getOp().end(); ++op) {
            auto rhsPos = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryAnd: {
                    yoi_assert(lhsType->type == IRValueType::valueType::integerObject &&
                           rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for binary and");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_and);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected and expression operator");
                    return {};
                }
            }
            lhsPos = rhsPos;
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::exclusiveExpr *exclusiveExpr) {
        auto term = exclusiveExpr->getTerms().begin();
        auto op = exclusiveExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != exclusiveExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryXor: {
                    yoi_assert(lhsType->type == IRValueType::valueType::integerObject &&
                           rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for binary xor");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_xor);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected exclusive expression operator");
                    return {};
                }
            }

        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::inclusiveExpr *inclusiveExpr) {
        auto term = inclusiveExpr->getTerms().begin();
        auto op = inclusiveExpr->getOp().begin();
        auto lhs = visit(*term);
        for (; op != inclusiveExpr->getOp().end(); ++op) {
            auto rhs = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (lhsType->type == IRValueType::valueType::structObject ||
                rhsType->type == IRValueType::valueType::structObject) {
                // TODO: add support for overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryOr: {
                    yoi_assert(lhsType->type == IRValueType::valueType::integerObject &&
                           rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for binary or");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_or);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected inclusive expression operator");
                    return {};
                }
            }

        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::logicalAndExpr *logicalAndExpr) {
        auto term = logicalAndExpr->getTerms().begin();
        auto op = logicalAndExpr->getOp().begin();
        auto lhs = visit(*term);

        for (; op != logicalAndExpr->getOp().end(); ++op) {
            auto exitWithTrueBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitWithFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::push_boolean, {{IROperand::operandType::boolean, true}}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::jump, {IROperand{IROperand::operandType::codeBlock, exitBlock}}});

            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::push_boolean, {IROperand{IROperand::operandType::boolean, IROperand::operandValue{false}}}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}});

            auto &lhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            switch (op->kind) {
                case lexer::token::tokenKind::logicAnd: {
                    yoi_assert(lhsType->type == IRValueType::valueType::booleanObject, op->line, op->col,
                           "Not boolean type for logical and");
                    // FIXED: jump_if_false should receive a parameter from stack
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false,exitWithFalseBlock);

                    visit(*++term);
                    auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    yoi_assert(rhsType->type == IRValueType::valueType::booleanObject, op->line, op->col,
                           "Not boolean type for logical and");
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, exitWithFalseBlock);
                    moduleContext->getIRBuilder().jumpOp(exitWithTrueBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(exitBlock);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected logical and expression operator");
                    return {};
                }
            }

        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::logicalOrExpr *logicalOrExpr) {
        auto term = logicalOrExpr->getTerms().begin();
        auto op = logicalOrExpr->getOp().begin();
        auto lhs = visit(*term);

        for (; op != logicalOrExpr->getOp().end(); ++op) {
            auto exitWithTrueBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitWithFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::push_boolean, {{IROperand::operandType::boolean, true}}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithTrueBlock).insert({IR::Opcode::jump, {IROperand{IROperand::operandType::codeBlock, exitBlock}}});

            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::push_boolean, {IROperand{IROperand::operandType::boolean, IROperand::operandValue{false}}}});
            moduleContext->getIRBuilder().getCodeBlock(exitWithFalseBlock).insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}});

            auto &lhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            switch (op->kind) {
                case lexer::token::tokenKind::logicAnd: {
                    yoi_assert(lhsType->type == IRValueType::valueType::booleanObject, op->line, op->col,
                           "Not boolean type for logical and");
                    // FIXED: jump_if_false should receive a parameter from stack
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, exitWithTrueBlock);

                    visit(*++term);
                    auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    yoi_assert(rhsType->type == IRValueType::valueType::booleanObject, op->line, op->col,
                           "Not boolean type for logical and");
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, exitWithTrueBlock);
                    moduleContext->getIRBuilder().jumpOp(exitWithFalseBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(exitBlock);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected logical and expression operator");
                    return {};
                }
            }

        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::rExpr *rExpr) {
        return visit(&rExpr->getExpr());
    }

    void visitor::visit(yoi::codeBlock *codeBlock, bool notEmitNewBlockInstruction) {
        if (!notEmitNewBlockInstruction) {
            auto block = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpOp(block);
            moduleContext->getIRBuilder().switchCodeBlock(block);
        }
        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().createScope();
        for (auto stmt : codeBlock->getStmts()) {
            visit(stmt);
        }
        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().popScope();
    }

    yoi::indexT visitor::visit(yoi::memberExpr *memberExpr, bool isStoreOp) {
        auto it = memberExpr->getTerms().begin();
        yoi::indexT targetModule = -1;
        while (it != memberExpr->getTerms().end() && (targetModule = isModuleName(*it, -1)) != -1) {
            it++;
        }
        if (targetModule == -1) {
            visit(*it, isStoreOp);
        } else {
            visitExtern(*it, targetModule, isStoreOp);
        }

        for (; it != memberExpr->getTerms().end();) {
            ++it;
            if (it == memberExpr->getTerms().end()) {
                break;
            }
            auto rhsIt = *it;
            auto termType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            yoi_assert(termType->type == IRValueType::valueType::structObject, (**it).getLine(), (**it).getColumn(), "Not struct type");
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
                            auto tempVarType = irModule->structTable[termType->typeIndex]->fieldTypes[nameInfo.index];
                            if (isStoreOp) {
                                moduleContext->getIRBuilder().storeMemberOp({IROperand::operandType::index, nameInfo.index});
                            } else {
                                moduleContext->getIRBuilder().loadMemberOp({IROperand::operandType::index, nameInfo.index}, termType);
                            }
                            break;
                        }
                        case IRStructDefinition::nameInfo::nameType::method: {
                            panic(rhsIt->getLine(), rhsIt->getColumn(), "Method cannot be parsed without invocation");
                        }
                    }
                }
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::visit(yoi::inCodeBlockStmt *inCodeBlockStmt) {
        switch (inCodeBlockStmt->getKind()) {
            case inCodeBlockStmt::vKind::ifStmt:
                visit(inCodeBlockStmt->getValue().ifStmt);
                break;
            case inCodeBlockStmt::vKind::whileStmt:
                visit(inCodeBlockStmt->getValue().whileStmt);
                break;
            case inCodeBlockStmt::vKind::forStmt:
                visit(inCodeBlockStmt->getValue().forStmt);
                break;
            case inCodeBlockStmt::vKind::forEachStmt:
                visit(inCodeBlockStmt->getValue().forEachStmt);
                break;
            case inCodeBlockStmt::vKind::returnStmt:
                visit(inCodeBlockStmt->getValue().returnStmt);
                break;
            case inCodeBlockStmt::vKind::continueStmt:
                visit(inCodeBlockStmt->getValue().continueStmt);
                break;
            case inCodeBlockStmt::vKind::breakStmt:
                visit(inCodeBlockStmt->getValue().breakStmt);
                break;
            case inCodeBlockStmt::vKind::letStmt:
                visit(inCodeBlockStmt->getValue().letStmt);
                break;
            case inCodeBlockStmt::vKind::codeBlock:
                visit(inCodeBlockStmt->getValue().codeBlock);
                break;
            case inCodeBlockStmt::vKind::rExpr:
                visit(inCodeBlockStmt->getValue().rExpr);
                // balance the stack
                moduleContext->getIRBuilder().popFromTempVarStack();
                break;
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
            assert(termType->type == IRValueType::valueType::structObject, rhsIt->getLine(), rhsIt->getColumn(), "Not struct type");
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
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structObject or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structObject) {
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
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structObject or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structObject) {
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
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structObject or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structObject) {
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
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structObject or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structObject) {
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
            if (lhs->type == IRValueType::valueType::lvalue && lhs->lvalueType->type == IRValueType::valueType::structObject or
                rhs->type == IRValueType::valueType::lvalue && rhs->lvalueType->type == IRValueType::valueType::structObject) {
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

    yoi::indexT visitor::visit(yoi::subscriptExpr *subscriptExpr, bool isStoreOp) {
        if (subscriptExpr->isSubscript()) {
            // TODO: subscript
            return {};
        } else if (subscriptExpr->isInvocation()) {
            try {
                auto funcIndex = irModule->functionTable.getIndex(subscriptExpr->id->getId().get().strVal);
                auto func = irModule->functionTable[funcIndex];
                for (auto &arg : subscriptExpr->args->get()) {
                    visit(arg);
                }
                moduleContext->getIRBuilder().invokeOp(funcIndex, subscriptExpr->args->get().size(), func->returnType);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            } catch(...) {
                panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Undefined function: " + wstring2string(subscriptExpr->id->getId().get().strVal));
            }
        } else {
            return visit(subscriptExpr->id);
        }
    }

    yoi::indexT visitor::visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg, bool isStoreOp) {
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
            return IRValueType{IRValueType::valueType::structObject, typeIndex};
        } catch(std::runtime_error &e) {
            // let it go
        }
        if (typeName == L"int") {
            return moduleContext->getCompilerContext()->getIntObjectType();
        } else if (typeName == L"bool") {
            return moduleContext->getCompilerContext()->getBoolObjectType();
        } else if (typeName == L"deci") {
            return moduleContext->getCompilerContext()->getDeciObjectType();
        } else if (typeName == L"string") {
            return moduleContext->getCompilerContext()->getStrObjectType();
        } else {
            panic(identifier->getLine(), identifier->getColumn(), "Unsupported type: " + wstring2string(typeName));
        }
    }

    yoi::indexT visitor::visit(yoi::funcDefStmt *funcDefStmt) {
        auto funcName = funcDefStmt->getId();
        if (funcName.hasDefTemplateArg()) {
            // TODO: function template
        } else {
            auto funcType = parseTypeSpec(&funcDefStmt->getResultType());
            IRFunctionDefinition::Builder builder;
            builder.setName(funcName.getId().node.strVal);
            builder.setReturnType(managedPtr(funcType));
            for (auto &i : funcDefStmt->getArgs().get()) {
                auto argName = i->getId().node.strVal;
                auto argType = managedPtr(parseTypeSpec(i->spec));
                builder.addArgument(argName, argType);
            }
            auto func = builder.yield();

            irModule->functionTable.put(funcName.getId().node.strVal, func);

            moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
            moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
            visit(funcDefStmt->block, true);
            moduleContext->getIRBuilder().yield();
            moduleContext->popIRBuilder();
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::IROperand visitor::visit(yoi::interfaceDefStmt *interfaceDefStmt) {

    }

    yoi::indexT visitor::visit(yoi::structDefStmt *structDefStmt) {
        if (structDefStmt->id->hasDefTemplateArg()) {
            // TODO: struct template
        }
        auto &structName = structDefStmt->id->getId().get().strVal;
        // occupy a slot in the struct table
        auto structIndex = irModule->structTable.put(structName, {});

        IRStructDefinition::Builder builder;
        builder.setName(structName);
        for (auto &i : structDefStmt->getInner().getInner()) {
            switch (i->kind) {
                case 0: {
                    // member
                    auto memberName = i->getVar().getId().get().strVal;
                    auto memberType = managedPtr(parseTypeSpec(i->getVar().spec));
                    builder.addField(memberName, memberType);
                    break;
                }
                case 1: {
                    // constructor
                    IRFunctionDefinition::Builder constructorBuilder;
                    auto funcName = L"struct#" + structName + L"#" + L"constructor";
                    constructorBuilder.setName(funcName).setReturnType(managedPtr(moduleContext->getCompilerContext()->getNoneObjectType()));
                    // add this pointer as the first argument
                    constructorBuilder.addArgument(L"this", managedPtr(IRValueType{IRValueType::valueType::structObject, structIndex}));

                    for (auto &arg : i->getConstructor().getArgs().get()) {
                        auto argName = arg->getId().get().strVal;
                        auto argType = managedPtr(parseTypeSpec(arg->spec));
                        constructorBuilder.addArgument(argName, argType);
                    }
                    auto func = constructorBuilder.yield();
                    builder.addMethod(L"constructor", irModule->functionTable.put(funcName, func));
                    break;
                }
                case 2: {
                    // method
                    auto methodName = L"struct#" + structName + L"#" + i->getMethod().getName().get().strVal;
                    auto methodType = managedPtr(parseTypeSpec(i->getMethod().resultType));
                    IRFunctionDefinition::Builder methodBuilder;

                    methodBuilder.setName(methodName);

                    // add this pointer as the first argument
                    methodBuilder.addArgument(L"this", managedPtr(IRValueType{IRValueType::valueType::structObject, structIndex}));

                    for (auto &arg : i->getMethod().getArgs().get()) {
                        auto argName = arg->getId().get().strVal;
                        auto argType = managedPtr(parseTypeSpec(arg->spec));
                        methodBuilder.addArgument(argName, argType);
                    }
                    auto func = methodBuilder.yield();
                    builder.addMethod(methodName, irModule->functionTable.put(methodName, func));
                    break;
                }
            }
        }
        auto structType = builder.yield();
        irModule->structTable[structIndex] = structType;
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::implStmt *implStmt) {
        if (implStmt->isImplForStmt()) {
            // TODO: interface implementation
        } else {
            // struct implementation
            auto structName = implStmt->getStructId().get().strVal;
            yoi::indexT structIndex;

            try {
                auto structIndex = irModule->structTable.getIndex(structName);
            } catch(std::runtime_error &e) {
                panic(implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(implStmt->getStructId().get().strVal));
            }

            auto &structType = irModule->structTable[structName];
            for (auto &i : implStmt->getInner().getInner()) {
                if (i->isConstructor()) {
                    // fetch constructor func decl from irModule
                    auto funcName = L"struct#" + structName + L"#" + L"constructor";
                    auto funcIndex = irModule->functionTable.getIndex(funcName);
                    auto func = irModule->functionTable[funcIndex];
                    // push a new irFuncBuilder for the constructor
                    moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
                    // build code blocks
                    moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                    visit(i->getConstructor().block, true);
                    moduleContext->getIRBuilder().yield();
                    moduleContext->popIRBuilder();
                } else {
                    // fetch method decl from irModule
                    auto methodName = L"struct#" + structName + L"#" + i->getMethod().getName().get().strVal;
                    auto methodIndex = irModule->functionTable.getIndex(methodName);
                    auto method = irModule->functionTable[methodIndex];
                    // push a new irFuncBuilder for the method
                    moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, method});
                    // build code blocks
                    moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                    visit(i->getMethod().block, true);
                    moduleContext->getIRBuilder().yield();
                    moduleContext->popIRBuilder();
                }
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::letStmt *letStmt) {
        for (auto &i : letStmt->terms) {
            visit(i->rhs);
            auto type = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (isVisitingGlobalScope()) {
                // global variable
                auto index = irModule->globalVariables.put(i->lhs->node.strVal, type);
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global, {IROperand::operandType::globalVar, index});

            } else {
                auto index = moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().put(i->lhs->node.strVal, type);
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local, {IROperand::operandType::localVar, index});
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
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

    yoi::indexT visitor::visit(yoi::ifStmt *ifStmt) {
        visit(ifStmt->getIfBlock().cond);
        auto condType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        yoi_assert(condType->type == IRValueType::valueType::booleanObject, ifStmt->getLine(), ifStmt->getColumn(), "The type in if-condition must be boolean");

        auto ifBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, ifBlock);
        auto back = moduleContext->getIRBuilder().switchCodeBlock(ifBlock);
        visit(ifStmt->getIfBlock().block, true);
        moduleContext->getIRBuilder().switchCodeBlock(back);

        for (auto &i : ifStmt->elifB) {
            visit(i.cond);
            auto elifCondType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            yoi_assert(elifCondType->type == IRValueType::valueType::booleanObject, i.cond->getLine(), i.cond->getColumn(), "The type in elif-condition must be boolean");

            auto elifBlock = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, elifBlock);
            auto back = moduleContext->getIRBuilder().switchCodeBlock(elifBlock);
            visit(i.block, true);
            moduleContext->getIRBuilder().switchCodeBlock(back);
        }

        if (ifStmt->hasElseBlock()) {
            auto elseBlock = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpOp(elseBlock);
            auto back = moduleContext->getIRBuilder().switchCodeBlock(elseBlock);
            visit(ifStmt->elseB, true);
            moduleContext->getIRBuilder().switchCodeBlock(back);
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::whileStmt *whileStmt) {
        auto condBlock = moduleContext->getIRBuilder().createCodeBlock();
        moduleContext->getIRBuilder().jumpOp(condBlock);
        auto back = moduleContext->getIRBuilder().switchCodeBlock(condBlock);

        visit(whileStmt->cond);
        auto condType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        yoi_assert(condType->type == IRValueType::valueType::booleanObject, whileStmt->getLine(), whileStmt->getColumn(), "The type in while-condition must be boolean");

        auto whileBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto outBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, whileBlock);
        moduleContext->getIRBuilder().jumpOp(outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(whileBlock);
        visit(whileStmt->block, true);

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

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::forStmt *forStmt) {
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
        visit(forStmt->cond);
        auto condType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        yoi_assert(condType->type == IRValueType::valueType::booleanObject, forStmt->getLine(), forStmt->getColumn(), "The type in for-condition must be boolean");

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, codeBlock);
        moduleContext->getIRBuilder().jumpOp(outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(codeBlock);
        visit(forStmt->block, true);

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

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::visit(yoi::forEachStmt *forEachStmt) {
        // TODO: implement foreach statement
        panic(forEachStmt->getLine(), forEachStmt->getColumn(), "forEach statement is not implemented yet");
    }

    yoi::indexT visitor::visit(yoi::returnStmt *returnStmt) {
        if (returnStmt->hasValue()) {
            visit(returnStmt->value);
            moduleContext->getIRBuilder().retOp();
        } else {
            // TODO: return void
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::continueStmt *continueStmt) {
        moduleContext->getIRBuilder().insert({IR::Opcode::dummy_continue, {}});
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::breakStmt *breakStmt) {
        moduleContext->getIRBuilder().insert({IR::Opcode::dummy_break, {}});
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
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
        auto mod = moduleContext->getCompilerContext()->getImportedModule(targetModule);
        auto exId = addExternEntryIfNotExists(targetModule, identifier);
        auto ex = irModule->externTable[exId];
        yoi_assert(ex->type == IRExternEntry::externType::structType, identifier->getLine(), identifier->getColumn(), "Invalid type specifier, expected struct type");
        return {IRValueType::valueType::structObject, exId};
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

                IRValueType lhs{IRValueType::valueType::integerObject};
                if (targetModule == -1) {
                    lhs = parseTypeSpec(*it);
                } else {
                    lhs = parseTypeSpecExtern(*it, targetModule);
                }
                yoi_assert(it != typeSpec->member->getTerms().end(), typeSpec->getLine(), typeSpec->getColumn(), "Type specifier is not valid.");
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
            std::shared_ptr<yoi::IRModule> target = currentModule == -1 ? irModule : moduleContext->getCompilerContext()->getImportedModule(currentModule);
            if (auto x = target->moduleImports.find(it->getId().id->node.strVal); x != target->moduleImports.end() ) {
                return moduleContext->getCompilerContext()->getModuleIndexByRealPath(x->second);
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    yoi::IRExternEntry visitor::getExternEntry(yoi::indexT moduleIndex, yoi::identifier *identifier) const {
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->globalVariables.getIndex(identifier->node.strVal);
            return {IRExternEntry::externType::globalVar, identifier->node.strVal, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->functionTable.getIndex(identifier->node.strVal);
            return {IRExternEntry::externType::function, identifier->node.strVal, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->structTable.getIndex(identifier->node.strVal);
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
        return moduleContext->getIRBuilder().irFuncDefinition()->name == L"yoimiya_glob_initializer";
    }

    yoi::indexT visitor::visitExtern(yoi::identifier *identifier, yoi::indexT targetModule, bool isStoreOp) {
        try {
            auto entryIndex = addExternEntryIfNotExists(targetModule, identifier);
            auto entry = irModule->externTable[entryIndex];
            yoi_assert(entry->type == IRExternEntry::externType::globalVar, identifier->getLine(), identifier->getColumn(), "Invalid type specifier, expected global variable");
            auto valType = moduleContext->getCompilerContext()->getImportedModule(targetModule)->globalVariables[identifier->node.strVal];
            // moduleContext->getIRBuilder().insert({IR::Opcode::load_extern,{{IROperand::operandType::index, entry}}});
            moduleContext->getIRBuilder().loadOp(IR::Opcode::load_extern, {IROperand::operandType::index, entryIndex}, valType);\
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } catch (std::runtime_error &) {
            // not found, panic
            panic(identifier->getLine(), identifier->getColumn(), "undefined identifier: " + wstring2string(identifier->node.strVal));
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }
    }

    yoi::indexT
    visitor::visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg, yoi::indexT targetModule, bool isStoreOp) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
        } else {
            return visitExtern(identifierWithTemplateArg->id, targetModule);
        }
    }

    yoi::indexT visitor::visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule, bool isStoreOp) {
        if (subscriptExpr->isSubscript()) {
            // TODO: subscript
        } else if (subscriptExpr->isInvocation()) {
            auto funcIndex = addExternEntryIfNotExists(targetModule, subscriptExpr->id->id);
            yoi_assert(irModule->externTable[funcIndex]->type == IRExternEntry::externType::function, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Invalid type specifier, expected function");
            auto funcType = moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable[subscriptExpr->id->id->node.strVal];
            for (auto &arg : subscriptExpr->args->arg) {
                visit(arg);
            }
            moduleContext->getIRBuilder().invokeOp(funcIndex, subscriptExpr->args->arg.size(), funcType->returnType, true);
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } else {
            return visitExtern(subscriptExpr->id, targetModule);
        }
    }
} // yoi