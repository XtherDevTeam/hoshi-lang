//
// Created by XIaokang00010 on 2024/9/6.
//

#include "visitor.h"
#include "compiler/compilerContext.h"
#include "compiler/ir/IR.h"
#include "compiler/moduleContext.h"
#include "share/def.hpp"
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace yoi {
    visitor::visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext,
                     const std::shared_ptr<yoi::IRModule> &irModule, yoi::indexT moduleIndex) : moduleContext(moduleContext), irModule(irModule), currentModuleIndex(moduleIndex) {

    }

    std::shared_ptr<yoi::IRModule> visitor::visit() {
        auto globInitializer = managedPtr(IRFunctionDefinition{L"yoimiya_glob_initializer", {}, moduleContext->getCompilerContext()->getNoneObjectType()});
        irModule->functionTable.put(L"yoimiya_glob_initializer", globInitializer);
        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, globInitializer});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(&moduleContext->getModuleAST());
        moduleContext->getIRBuilder().retOp(true);
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
        } catch (std::out_of_range &e) {
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

        switch (uniqueExpr->getOp().kind) {
            case lexer::token::tokenKind::incrementSign: {
                // TODO: add support for operator overloading
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                yoi_assert(lhs->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue type must be basic type for decrement");
                moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::increment);
                break;
            }
            case lexer::token::tokenKind::decrementSign: {
                // TODO: add support for operator overloading
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                yoi_assert(lhs->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Lvalue type must be basic type for decrement");
                moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::decrement);
                break;
            }
            case lexer::token::tokenKind::binaryNot: {
                // TODO: add support for operator overloading
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                yoi_assert(lhs->isBasicType(), uniqueExpr->getOp().line, uniqueExpr->getOp().col, "Not basic type for bitwise not");
                moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::bitwise_not);
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
                        // TODO: add support for operator overloading
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
                        // TODO: add support for operator overloading
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
                        // TODO: add support for operator overloading
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
                        // TODO: add support for operator overloading
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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::asterisk: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for multiplication");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                    break;
                }
                case lexer::token::tokenKind::slash: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for multiplication");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

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
            lhsPos = moduleContext->getIRBuilder().getCurrentInsertionPoint();
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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::plus: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for addition");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                    break;
                }
                case lexer::token::tokenKind::minus: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for subtraction");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected addition expression operator");
                }
            }
            lhsPos = moduleContext->getIRBuilder().getCurrentInsertionPoint();
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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryShiftLeft: {
                    yoi_assert(lhsType->isBasicType() && rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for left shift or right hand side is not integer");
                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::left_shift);
                    break;
                }
                case lexer::token::tokenKind::binaryShiftRight: {
                    yoi_assert(lhsType->isBasicType() && rhsType->type == IRValueType::valueType::integerObject, op->line, op->col,
                           "Not basic type for right shift or right hand side is not integer");
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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::lessThan: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for less than");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_than);
                    break;
                }
                case lexer::token::tokenKind::greaterThan: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for greater than");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_than);
                    break;
                }
                case lexer::token::tokenKind::lessEqual: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for less than or equal");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_equal);
                    break;
                }
                case lexer::token::tokenKind::greaterEqual: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for greater than or equal");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::equal: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for equal");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

                    moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::equal);
                    break;
                }
                case lexer::token::tokenKind::notEqual: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col, "Not basic type for not");

                    emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos);

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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryAnd: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for binary and");

                    if (lhsType->isBasicType() && lhsType->type != IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(moduleContext->getCompilerContext()->getIntObjectType(), lhsPos, true);
                    }
                    if (rhsType->isBasicType() && rhsType->type != IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast(moduleContext->getCompilerContext()->getIntObjectType(), rhsPos);
                    }

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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryXor: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for binary xor");

                    if (lhsType->isBasicType() && lhsType->type != IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getIntObjectType()), lhs, true);
                    }
                    if (rhsType->isBasicType() && rhsType->type != IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getIntObjectType()), rhs);
                    }

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
                // TODO: add support for operator overloading
            }
            switch (op->kind) {
                case lexer::token::tokenKind::binaryOr: {
                    yoi_assert(lhsType->isBasicType() && rhsType->isBasicType(), op->line, op->col,
                           "Not basic type for binary or");

                    if (lhsType->isBasicType() && lhsType->type != IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getIntObjectType()), lhs, true);
                    }
                    if (rhsType->isBasicType() && rhsType->type != IRValueType::valueType::integerObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getIntObjectType()), rhs);
                    }

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
                    yoi_assert(lhsType->isBasicType(), op->line, op->col,
                           "Not basic type for logical and");
                    // if not bool, convert it to bool
                    if (lhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getBoolObjectType()), lhs, true);
                    }

                    // FIXED: jump_if_false should receive a parameter from stack
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false,exitWithFalseBlock);

                    auto rhs = visit(*++term);
                    auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    yoi_assert(lhsType->isBasicType(), op->line, op->col,
                           "Not basic type for logical and");
                    // same as above
                    if (rhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getBoolObjectType()), rhs);
                    }

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
                case lexer::token::tokenKind::logicOr: {
                    yoi_assert(lhsType->isBasicType(), op->line, op->col,
                           "Not basic type for logical or");
                    // if not bool, convert it to bool
                    if (lhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getBoolObjectType()), lhs, true);
                    }
                    // FIXED: jump_if_false should receive a parameter from stack
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, exitWithTrueBlock);

                    auto rhs = visit(*++term);
                    auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    yoi_assert(lhsType->isBasicType(), op->line, op->col,
                           "Not basic type for logical or");
                    // same as above
                    if (rhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast((moduleContext->getCompilerContext()->getBoolObjectType()), rhs);
                    }
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
        while (it + 1 != memberExpr->getTerms().end() && (targetModule = isModuleName((*it)->id, -1)) != -1) {
            it++;
        }
        
        bool whetherLastTerm = it + 1 == memberExpr->getTerms().end();

        if (targetModule == -1) {
            visit(*it, isStoreOp && whetherLastTerm);
        } else {
            visitExtern(*it, targetModule, isStoreOp && whetherLastTerm);
        }

        for (; it != memberExpr->getTerms().end();) {
            ++it;
            if (it == memberExpr->getTerms().end()) {
                break;
            }
            auto rhsIt = *it;
            auto termType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            for (auto& sub : rhsIt->getSubscript()) {
                if (sub->isInvocation()) {
                    if (termType->type == IRValueType::valueType::structObject) {
                        auto memberName = rhsIt->id;
                        try {
                            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                            for (auto &arg : sub->args->get()) {
                                visit(arg);
                                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                            }
                            auto actualName = memberName->getId().get().strVal + getFuncUniqueNameStr(argTypes);

                            auto nameInfo = moduleContext->getCompilerContext()->getImportedModule(termType->typeAffiliateModule)->structTable[termType->typeIndex]->lookupName(actualName);
                            switch (nameInfo.type) {
                                case IRStructDefinition::nameInfo::nameType::field: {
                                    panic(rhsIt->getLine(), rhsIt->getColumn(), "Field cannot be parsed within an invocation");
                                    break;
                                }
                                case IRStructDefinition::nameInfo::nameType::method: {
                                    auto funcIndex = nameInfo.index;
                                    auto func = moduleContext->getCompilerContext()->getImportedModule(termType->typeAffiliateModule)->functionTable[funcIndex];
                                    
                                    if (termType->typeAffiliateModule == currentModuleIndex) {
                                        moduleContext->getIRBuilder().invokeMethodOp(funcIndex, sub->args->get().size(), func->returnType);
                                    } else {
                                        auto externEntry = addExternEntryIfNotExists(termType->typeAffiliateModule, func->name);
                                        moduleContext->getIRBuilder().invokeMethodOp(externEntry, sub->args->get().size(), func->returnType, true);
                                    }
                                }
                            }
                        } catch (std::out_of_range &e) {
                            panic(rhsIt->getLine(), rhsIt->getColumn(), "Undefined field or function overload: " + yoi::wstring2string(rhsIt->id->getId().get().strVal));
                        }
                    } else if (termType->type == IRValueType::valueType::interfaceObject) {
                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                        for (auto &arg : sub->args->get()) {
                            visit(arg);
                            argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                        }

                        auto methodName = rhsIt->id->getId().get().strVal + getFuncUniqueNameStr(argTypes);
                        auto methodIdx = moduleContext->getCompilerContext()->getImportedModule(termType->typeAffiliateModule)->interfaceTable[termType->typeIndex]->methodMap.getIndex(methodName);
                        auto method = moduleContext->getCompilerContext()->getImportedModule(termType->typeAffiliateModule)->interfaceTable[termType->typeIndex]->methodMap[methodIdx];
                        yoi_assert(method->argumentTypes.size() == sub->args->get().size(), rhsIt->getLine(), rhsIt->getColumn(), "Argument count does not match");
                        moduleContext->getIRBuilder().invokeVirtualOp(methodIdx, sub->args->get().size(), method->returnType);
                    }
                } else if (sub->isSubscript()) {
                    panic(rhsIt->getLine(), rhsIt->getColumn(), "TODO: Subscript on member expression not implemented yet.");
                }
            }

            if (rhsIt->getSubscript().empty()) {
                yoi_assert(termType->type == IRValueType::valueType::structObject, (**it).getLine(), (**it).getColumn(), "Not struct type");
                auto memberName = rhsIt->id;
                if(memberName->hasTemplateArg()) {
                    // TODO: what the heck is this
                } else {
                    try {
                        auto nameInfo = moduleContext->getCompilerContext()->getImportedModule(termType->typeAffiliateModule)->structTable[termType->typeIndex]->lookupName(memberName->getId().get().strVal);
                        switch (nameInfo.type) {
                            case IRStructDefinition::nameInfo::nameType::field: {
                                auto tempVarType = irModule->structTable[termType->typeIndex]->fieldTypes[nameInfo.index];
                                if (isStoreOp) {
                                    moduleContext->getIRBuilder().storeMemberOp({IROperand::operandType::index, nameInfo.index});
                                } else {
                                    moduleContext->getIRBuilder().loadMemberOp({IROperand::operandType::index, nameInfo.index}, tempVarType);
                                }
                                break;
                            }
                            case IRStructDefinition::nameInfo::nameType::method: {
                                panic(rhsIt->getLine(), rhsIt->getColumn(), "Method cannot be parsed without invocation");
                            }
                        }
                    } catch (std::out_of_range &e) {
                        panic(rhsIt->getLine(), rhsIt->getColumn(), "Undefined field or function: " + yoi::wstring2string(rhsIt->id->getId().get().strVal));
                    }
                }
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::visit(yoi::inCodeBlockStmt *inCodeBlockStmt) {
        switch (inCodeBlockStmt->getKind()) {
            case inCodeBlockStmt::vKind::ifStmt:
                visit(inCodeBlockStmt->getValue().ifStmtVal);
                break;
            case inCodeBlockStmt::vKind::whileStmt:
                visit(inCodeBlockStmt->getValue().whileStmtVal);
                break;
            case inCodeBlockStmt::vKind::forStmt:
                visit(inCodeBlockStmt->getValue().forStmtVal);
                break;
            case inCodeBlockStmt::vKind::forEachStmt:
                visit(inCodeBlockStmt->getValue().forEachStmtVal);
                break;
            case inCodeBlockStmt::vKind::returnStmt:
                visit(inCodeBlockStmt->getValue().returnStmtVal);
                break;
            case inCodeBlockStmt::vKind::continueStmt:
                visit(inCodeBlockStmt->getValue().continueStmtVal);
                break;
            case inCodeBlockStmt::vKind::breakStmt:
                visit(inCodeBlockStmt->getValue().breakStmtVal);
                break;
            case inCodeBlockStmt::vKind::letStmt:
                visit(inCodeBlockStmt->getValue().letStmtVal);
                break;
            case inCodeBlockStmt::vKind::codeBlock:
                visit(inCodeBlockStmt->getValue().codeBlockVal);
                break;
            case inCodeBlockStmt::vKind::rExpr:
                visit(inCodeBlockStmt->getValue().rExprVal);
                // balance the stack
                moduleContext->getIRBuilder().popFromTempVarStack();
                break;
        }
    }

    yoi::indexT visitor::visit(yoi::subscriptExpr *subscriptExpr, bool isStoreOp) {
        if (subscriptExpr->getSubscript().empty()) {
                return visit(subscriptExpr->id, isStoreOp);
            }
        
            auto it = subscriptExpr->getSubscript().begin();
            auto end = subscriptExpr->getSubscript().end();
            auto& first_term = *it;

            // Determine if the base `id` is a type name.
            bool is_a_type = false;
            try {
                parseTypeSpec(subscriptExpr->id);
                is_a_type = true;
            } catch (const std::runtime_error&) {
                is_a_type = false;
            }

            bool first_term_handled = false;

            // Case 1: Array initializer like `int[2](...)`
            if (is_a_type && first_term->isSubscript()) {
                auto dim_it = it;
                while (dim_it != end && (*dim_it)->isSubscript()) {
                    visit((*dim_it)->expr);
                    // TODO: Collect dimension from stack
                    dim_it++;
                }

                if (dim_it != end && (*dim_it)->isInvocation()) {
                    for (auto& val : (*dim_it)->args->get()) {
                        visit(val);
                        // TODO: Collect initializer value from stack
                    }
                    it = ++dim_it;
                } else {
                    it = dim_it;
                }
                // TODO: Create array and initialize it.
                panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "TODO: Array initializer is not implemented yet.");
            }
            // Case 2: First term is an invocation `id(...)`
            else if (first_term->isInvocation()) {
                first_term_handled = true;
                // This block is adapted from the old visitor's logic for `isInvocation()`.
                auto baseName = subscriptExpr->id->getId().get().strVal;
                auto args = first_term->args;

                // --- 1. Explicit Template Instantiation ---
                if (subscriptExpr->id->hasTemplateArg()) {
                    auto concreteTemplateArgs = parseTemplateArgs(subscriptExpr->id->getArg());

                    // Try as function template
                    if (irModule->functionTemplateTable.contains(baseName)) {
                        auto funcTemplate = irModule->functionTemplateTable[baseName];
                        auto astNode = irModule->funcTemplateAsts.at(baseName);
                        auto specializedFuncIndex = specializeFunctionTemplate(funcTemplate, astNode, concreteTemplateArgs);
                        auto specializedFunc = irModule->functionTable[specializedFuncIndex];

                        for (auto& arg : args->get()) {
                            visit(arg);
                        }
                        moduleContext->getIRBuilder().invokeOp(specializedFuncIndex, args->get().size(), specializedFunc->returnType);
                    }
                    // Try as struct template constructor
                    else if (irModule->structTemplateTable.contains(baseName)) {
                        if (irModule->templateImplAsts.count(baseName)) {
                            auto pureTemplateAst = irModule->templateImplAsts.at(baseName);

                            auto specializedStructIndex = specializeStructTemplate(baseName, concreteTemplateArgs, pureTemplateAst);
                            auto specializedStruct = irModule->structTable[specializedStructIndex];

                            moduleContext->getIRBuilder().newStructOp(specializedStructIndex);

                            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                            for (auto& arg : args->get()) {
                                visit(arg);
                                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                            }

                            auto ctorName = L"constructor" + getFuncUniqueNameStr(argTypes);
                            yoi_assert(specializedStruct->nameIndexMap.count(ctorName), subscriptExpr->getLine(), subscriptExpr->getColumn(), "No matching constructor for specialized struct " + wstring2string(specializedStruct->name));

                            auto ctorInfo = specializedStruct->lookupName(ctorName);
                            auto ctorFunc = irModule->functionTable[ctorInfo.index];
                            moduleContext->getIRBuilder().invokeMethodOp(ctorInfo.index, args->get().size(), ctorFunc->returnType);
                        } else {
                            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "No implementation found for template: " + wstring2string(baseName));
                        }
                    } else {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "No matching function or struct template for explicit instantiation of: " + wstring2string(baseName));
                    }
                } else {
                    bool resolved = false;
                    // Try regular function
                    try {
                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                        for (auto &arg : args->get()) {
                            visit(arg);
                            argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                        }
                        auto mangledFuncName = baseName + getFuncUniqueNameStr(argTypes);

                        auto funcIndex = irModule->functionTable.getIndex(mangledFuncName);
                        auto func = irModule->functionTable[funcIndex];
                        moduleContext->getIRBuilder().invokeOp(funcIndex, args->get().size(), func->returnType);
                        resolved = true;
                    } catch (std::out_of_range &) {}

                    // Try implicit function template instantiation
                    if (!resolved) {
                        try {
                            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                            for (auto &arg : args->get()) {
                                visit(arg);
                                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                            }
                            if (irModule->functionTemplateTable.contains(baseName)) {
                                auto funcTemplate = irModule->functionTemplateTable[baseName];
                                auto astNode = irModule->funcTemplateAsts.at(baseName);

                                yoi::vec<std::shared_ptr<IRValueType>> deducedArgs(funcTemplate->templateArguments.size());
                                for (yoi::indexT i = 0; i < argTypes.size(); i++) {
                                    if (i < funcTemplate->templateDefinition->argumentTypes.size() && funcTemplate->templateDefinition->argumentTypes[i]->type == IRValueType::valueType::incompleteTemplateType) {
                                        auto &srcTypeToPlace = argTypes[i];
                                        auto incompleteTypeIndex = funcTemplate->templateDefinition->argumentTypes[i]->typeIndex;
                                        if(deducedArgs[incompleteTypeIndex]) {
                                            yoi_assert(*deducedArgs[incompleteTypeIndex] == *srcTypeToPlace, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Same template argument type cannot be interpreted as different types.");
                                        }
                                        deducedArgs[incompleteTypeIndex] = srcTypeToPlace;
                                    }
                                }
                                for (yoi::indexT i = 0; i < deducedArgs.size(); i++) {
                                    yoi_assert(deducedArgs[i] != nullptr, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Cannot deduce template arguments: incomplete type: " + yoi::wstring2string(funcTemplate->templateArguments.getKey(i)));
                                }

                                auto specializedFuncIndex = specializeFunctionTemplate(funcTemplate, astNode, deducedArgs);
                                auto specializedFunc = irModule->functionTable[specializedFuncIndex];

                                moduleContext->getIRBuilder().invokeOp(specializedFuncIndex, args->get().size(), specializedFunc->returnType);
                                resolved = true;
                            }
                        } catch(std::out_of_range &) {}
                    }

                    // Try struct constructor
                    if (!resolved) {
                        try {
                            auto structIndex = irModule->structTable.getIndex(baseName);
                            auto structType = irModule->structTable[structIndex];
                            moduleContext->getIRBuilder().newStructOp(structIndex);

                            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                            for (auto &arg : args->get()) {
                                visit(arg);
                                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                            }

                            auto ctorName = L"constructor" + getFuncUniqueNameStr(argTypes);
                            auto ctorInfo = structType->lookupName(ctorName);
                            auto ctorFunc = irModule->functionTable[ctorInfo.index];
                            moduleContext->getIRBuilder().invokeMethodOp(ctorInfo.index, argTypes.size(), ctorFunc->returnType);
                            resolved = true;
                        } catch (std::out_of_range &) {}
                    }

                    // Try interface constructor
                    if (!resolved) {
                        try {
                            auto interfaceIndex = irModule->interfaceTable.getIndex(baseName);
                            moduleContext->getIRBuilder().newInterfaceOp(interfaceIndex);

                            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                            for (auto &arg : args->get()) {
                                visit(arg);
                                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                            }

                            yoi_assert(argTypes.size() == 1, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Interface constructor expects exactly one argument (the struct instance).");

                            auto structValue = argTypes[0];

                            auto interfaceImplName = getInterfaceImplName({currentModuleIndex, interfaceIndex}, {structValue->typeAffiliateModule, structValue->typeIndex});
                            auto interfaceImplIndex = irModule->interfaceImplementationTable.getIndex(interfaceImplName);
                            moduleContext->getIRBuilder().constructInterfaceImplOp(interfaceImplIndex);
                            resolved = true;
                        } catch (std::out_of_range &) {}
                    }

                    // Try imported function
                    if (!resolved) {
                        try {
                            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                            for (auto &arg : args->get()) {
                                visit(arg);
                                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                            }

                            auto importedFunctionIndex = irModule->externTable.getIndex(baseName);
                            yoi_assert(irModule->externTable[importedFunctionIndex]->type == IRExternEntry::externType::importedFunction, subscriptExpr->getLine(), subscriptExpr->getColumn(), "This is not an imported function: " + yoi::wstring2string(baseName));
                            auto importedFunc = moduleContext->getCompilerContext()->getIRFFITable()->importedLibraries[
                                irModule->externTable[importedFunctionIndex]->affiliateModule].importedFunctionTable[
                                    irModule->externTable[importedFunctionIndex]->itemIndex];

                            moduleContext->getIRBuilder().invokeImported(irModule->externTable[importedFunctionIndex]->affiliateModule, irModule->externTable[importedFunctionIndex]->itemIndex, argTypes.size(), importedFunc->returnType);
                            resolved = true;
                        } catch (std::out_of_range &) {}
                    }

                    if (!resolved) {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Undefined function, struct, interface, or template: " + wstring2string(baseName));
                    }
                }
            }
            // Case 4: First term is a subscript on a variable `var[...]`
            else { // first_term->isSubscript()
                visit(subscriptExpr->id, false); // Load the variable
            }

            if (first_term_handled) {
                it++; // Move past the first term if we handled it.
            }

            // --- Loop for subsequent terms ---
            while (it != end) {
                auto current_term = *it;
                bool isLastTerm = (std::next(it) == end);

                auto object_on_stack_type = moduleContext->getIRBuilder().getRhsFromTempVarStack();

                if (current_term->isSubscript()) {
                    // Case 3: Array access `...[index]`
                    // TODO: Assert object_on_stack_type is an array type.
                    visit(current_term->expr); // Pushes index onto stack.

                    if (isStoreOp && isLastTerm) {
                        moduleContext->getIRBuilder().storeOp(IR::Opcode::store_element, {});
                    } else {
                        // TODO: Need to know element type to load.
                        // auto element_type = object_on_stack_type->getElementType();
                        // moduleContext->getIRBuilder().loadOp(IR::Opcode::load_element, {}, element_type);
                    }
                    panic(current_term->getLine(), current_term->getColumn(), "TODO: Array access is not fully implemented yet.");

                } else if (current_term->isInvocation()) {
                    // Case 3: Function call on object `...(...)`
                    panic(current_term->getLine(), current_term->getColumn(), "TODO: Operator '()' overloading is not implemented yet.");
                }
                it++;
            }

            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

    yoi::indexT visitor::visitExtern(yoi::subscriptExpr *subscriptExpr, yoi::indexT targetModule, bool isStoreOp) {
        if (subscriptExpr->getSubscript().empty()) {
            return visitExtern(subscriptExpr->id, targetModule, isStoreOp);
        }

        auto it = subscriptExpr->getSubscript().begin();
        auto end = subscriptExpr->getSubscript().end();
        auto& first_term = *it;

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);

        bool is_a_type = false;
        try {
            parseTypeSpecExtern(subscriptExpr->id, targetModule);
            is_a_type = true;
        } catch (const std::runtime_error&) {
            is_a_type = false;
        }

        bool first_term_handled = false;

        if (is_a_type && first_term->isSubscript()) {
            // Extern array initializer
            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "TODO: Extern array initializer is not implemented yet.");
        } else if (first_term->isInvocation()) {
            first_term_handled = true;
            auto baseName = subscriptExpr->id->getId().get().strVal;
            auto args = first_term->args;
            yoi::wstr mangledName = baseName;

            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
            for (auto &arg : args->get()) {
                visit(arg);
                argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
            }

            if (subscriptExpr->id->hasTemplateArg()) {
                auto concreteTemplateArgs = parseTemplateArgs(subscriptExpr->id->getArg());
                mangledName = getMangledTemplateName(baseName, concreteTemplateArgs);
            }

            bool resolved = false;

            // Try finding an extern struct constructor
            try {
                auto structIndex = targetedModule->structTable.getIndex(mangledName);
                auto externStructIndex = addExternEntryIfNotExists(targetModule, mangledName);
                moduleContext->getIRBuilder().newStructOp(externStructIndex, true);

                auto ctorNamePart = L"constructor" + getFuncUniqueNameStr(argTypes);
                auto ctorFullName = mangledName + L"::" + ctorNamePart;
                
                auto externCtorIndex = addExternEntryIfNotExists(targetModule, ctorFullName);
                auto ctorFunc = targetedModule->functionTable[ctorFullName];
                moduleContext->getIRBuilder().invokeMethodOp(externCtorIndex, argTypes.size(), ctorFunc->returnType, true);
                resolved = true;
            } catch(std::out_of_range&) {}

            // Try finding an extern function
            if (!resolved) {
                try {
                    auto funcMangledName = mangledName + getFuncUniqueNameStr(argTypes);
                    auto externFuncIndex = addExternEntryIfNotExists(targetModule, funcMangledName);
                    auto func = targetedModule->functionTable[funcMangledName];

                    moduleContext->getIRBuilder().invokeOp(externFuncIndex, argTypes.size(), func->returnType, true);
                    resolved = true;
                } catch(std::out_of_range&) {}
            }

            if (!resolved) {
                panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not find extern function or struct constructor: " + wstring2string(mangledName));
            }
        } else { // first_term->isSubscript()
            visitExtern(subscriptExpr->id, targetModule, false);
        }

        if (first_term_handled) {
            it++;
        }

        while (it != end) {
            auto current_term = *it;
            bool is_last_term = (std::next(it) == end);

            auto object_on_stack_type = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (current_term->isSubscript()) {
                panic(current_term->getLine(), current_term->getColumn(), "TODO: Extern array access is not fully implemented yet.");
            } else if (current_term->isInvocation()) {
                panic(current_term->getLine(), current_term->getColumn(), "TODO: Extern operator '()' overloading is not implemented yet.");
            }
            it++;
        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

    yoi::indexT visitor::visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg, bool isStoreOp) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            // TODO: what the heck is this
            return {};
        } else {
            return visit(identifierWithTemplateArg->id, isStoreOp);
        }
    }

    yoi::indexT visitor::visit(yoi::useStmt *useStmt) {
        auto index = moduleContext->getCompilerContext()->compileModule(useStmt->path.strVal);
        irModule->moduleImports[useStmt->name->get().strVal] = index;
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    IRValueType visitor::parseTypeSpec(yoi::identifier *identifier) {
        auto &typeName = identifier->node.strVal;
        try {
            auto typeIndex = irModule->structTable.getIndex(typeName);
            return IRValueType{IRValueType::valueType::structObject, static_cast<yoi::indexT>(currentModuleIndex), typeIndex};
        } catch(std::out_of_range &e) {
            // let it go
        }
        try {
            auto incompleteType = getIncompleteType(typeName);
            return *incompleteType;
        } catch(std::out_of_range &e) {
            // let it go
        }
        if (typeName == L"int") {
            return *moduleContext->getCompilerContext()->getIntObjectType();
        } else if (typeName == L"bool") {
            return *moduleContext->getCompilerContext()->getBoolObjectType();
        } else if (typeName == L"deci") {
            return *moduleContext->getCompilerContext()->getDeciObjectType();
        } else if (typeName == L"string") {
            return *moduleContext->getCompilerContext()->getStrObjectType();
        } else if (typeName == L"none") {
            return *moduleContext->getCompilerContext()->getNoneObjectType();
        } else if (typeName == L"char") {
            return *moduleContext->getCompilerContext()->getCharObjectType();
        } else if (typeName == L"int32") {
            return *moduleContext->getCompilerContext()->getForeignInt32ObjectType();
        } else if (typeName == L"float") {
            return *moduleContext->getCompilerContext()->getForeignFloatObjectType();
        } else {
            panic(identifier->getLine(), identifier->getColumn(), "Unsupported type: " + wstring2string(typeName));
        }
    }

    yoi::indexT visitor::visit(yoi::funcDefStmt *funcDefStmt) {
        auto funcName = funcDefStmt->getId();

        if (funcName.hasDefTemplateArg()) {
            IRFunctionTemplate::Builder templateBuilder;
            IRFunctionDefinition::Builder builder;

            templateBuilder.templateArguments = getTemplateArgs(funcName.getArg());

            moduleContext->pushTemplateBuilder(templateBuilder);

            std::vector<std::shared_ptr<IRValueType>> argTypes;
            for (auto &i : funcDefStmt->getArgs().get()) {
                auto argName = i->getId().node.strVal;
                auto argType = managedPtr(parseTypeSpec(i->spec));
                argTypes.push_back(argType);
                builder.addArgument(argName, argType);
            }

            auto funcType = parseTypeSpec(&funcDefStmt->getResultType());

            builder.setReturnType(managedPtr(funcType));

            auto actualName = funcName.getId().node.strVal;
            builder.setName(actualName);

            auto func = builder.yield();

            templateBuilder.setTemplateDefinition(func);

            auto funcTemplate = templateBuilder.yield();
            irModule->functionTemplateTable.put_create(actualName, funcTemplate);
            irModule->funcTemplateAsts[actualName] = funcDefStmt;

            // Compilation of the body is deferred until specialization.
            moduleContext->popTemplateBuilder();
        } else {
            auto funcType = parseTypeSpec(&funcDefStmt->getResultType());
            IRFunctionDefinition::Builder builder;

            builder.setReturnType(managedPtr(funcType));
            std::vector<std::shared_ptr<IRValueType>> argTypes;
            for (auto &i : funcDefStmt->getArgs().get()) {
                auto argName = i->getId().node.strVal;
                auto argType = managedPtr(parseTypeSpec(i->spec));
                argTypes.push_back(argType);
                builder.addArgument(argName, argType);
            }

            builder.setName(funcName.getId().node.strVal + getFuncUniqueNameStr(argTypes));

            auto func = builder.yield();

            irModule->functionTable.put(builder.name, func);

            moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
            moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
            visit(funcDefStmt->block, true);
            moduleContext->getIRBuilder().yield();
            moduleContext->popIRBuilder();
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::interfaceDefStmt *interfaceDefStmt) {
        if (interfaceDefStmt->id->hasDefTemplateArg()) {
            // TODO: interface template
        }
        auto &interfaceName = interfaceDefStmt->id->getId().get().strVal;

        IRInterfaceInstanceDefinition::Builder builder;
        builder.setName(interfaceName);
        for (auto &i : interfaceDefStmt->getInner().getInner()) {
            yoi_assert(i->isMethod(), i->getLine(), i->getColumn(), "Interface member must be a method");

            auto methodName = i->getMethod().getName().get().strVal;
            auto methodResultType = managedPtr(parseTypeSpec(i->getMethod().resultType));
            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
            IRFunctionDefinition::Builder methodBuilder;
            methodBuilder.setReturnType(methodResultType);
            for (auto &arg : i->getMethod().getArgs().get()) {
                auto argName = arg->getId().get().strVal;
                auto argType = managedPtr(parseTypeSpec(arg->spec));
                methodBuilder.addArgument(argName, argType);
                argTypes.push_back(argType);
            }
            auto methodFuncName = L"interface#" + interfaceName + L"#" + methodName;
            auto uniq = getFuncUniqueNameStr(argTypes);
            methodBuilder.setName(methodFuncName + uniq);

            auto func = methodBuilder.yield();
            builder.addMethod(methodName + uniq, func);
        }
        auto interfaceType = builder.yield();
        irModule->interfaceTable.put(interfaceName, interfaceType);
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::structDefStmt *structDefStmt) {
        auto &structName = structDefStmt->id->getId().get().strVal;

        if (structDefStmt->getId().hasDefTemplateArg()) {
            IRStructTemplate::Builder templateBuilder;
            IRStructDefinition::Builder builder;
            
            templateBuilder.templateArguments = getTemplateArgs(structDefStmt->getId().getArg());
            moduleContext->pushTemplateBuilder(templateBuilder);

            builder.setName(structName);
            for (auto& field : structDefStmt->getInner().getInner()) {
                if (field->kind == 0) { // Var
                    auto memberName = field->getVar().getId().get().strVal;
                    auto memberType = managedPtr(parseTypeSpec(field->getVar().spec));
                    builder.addField(memberName, memberType);
                } else if (field->kind == 1) { // Constructor
                    IRFunctionTemplate::Builder constructorBuilder;
                    IRFunctionDefinition::Builder constructorDefBuilder;
                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                    moduleContext->pushTemplateBuilder(constructorBuilder);
                    
                    constructorBuilder.templateArguments = templateBuilder.templateArguments;
                    auto STRUCT_INCOMPLETE_TYPE = constructorBuilder.templateArguments.put_create(L"STRUCT", {{}, {-1, -1}});
                    constructorBuilder.templateArguments[STRUCT_INCOMPLETE_TYPE].templateType = managedPtr(IRValueType{IRValueType::valueType::incompleteTemplateType, currentModuleIndex, STRUCT_INCOMPLETE_TYPE});

                    constructorDefBuilder.setReturnType(constructorBuilder.templateArguments[STRUCT_INCOMPLETE_TYPE].templateType);

                    for (auto &arg : field->getConstructor().getArgs().get()) {
                        auto argName = arg->getId().get().strVal;
                        auto argType = managedPtr(parseTypeSpec(arg->spec));
                        constructorDefBuilder.addArgument(argName, argType);
                        argTypes.push_back(argType);
                    }

                    auto uniq = getFuncUniqueNameStr(argTypes);
                    auto mangledName = L"constructor" + uniq;
                    constructorDefBuilder.setName(structName + L"::" + mangledName);

                    auto func = constructorDefBuilder.yield();
                    constructorBuilder.setTemplateDefinition(func);

                    templateBuilder.setTemplateMethod(mangledName, constructorBuilder.yield());

                    moduleContext->popTemplateBuilder();
                } else if (field->kind == 2) { // Method
                    auto methodName = field->getMethod().getName().get().strVal;
                    IRFunctionTemplate::Builder methodBuilder;
                    IRFunctionDefinition::Builder methodDefBuilder;

                    moduleContext->pushTemplateBuilder(methodBuilder);

                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                    methodBuilder.templateArguments = templateBuilder.templateArguments;
                    auto STRUCT_INCOMPLETE_TYPE = methodBuilder.templateArguments.put_create(L"STRUCT", {{}, {-1, -1}});
                    methodBuilder.templateArguments[STRUCT_INCOMPLETE_TYPE].templateType = managedPtr(IRValueType{IRValueType::valueType::incompleteTemplateType, currentModuleIndex, STRUCT_INCOMPLETE_TYPE});

                    auto methodType = managedPtr(parseTypeSpec(field->getMethod().resultType));

                    methodDefBuilder.setReturnType(methodType);

                    for (auto &arg : field->getMethod().getArgs().get()) {
                        auto argName = arg->getId().get().strVal;
                        auto argType = managedPtr(parseTypeSpec(arg->spec));
                        methodDefBuilder.addArgument(argName, argType);
                        argTypes.push_back(argType);
                    }

                    auto uniq = getFuncUniqueNameStr(argTypes);
                    auto mangledName = methodName + uniq;
                    methodDefBuilder.setName(structName + L"::" + mangledName);

                    auto func = methodDefBuilder.yield();
                    methodBuilder.setTemplateDefinition(func);

                    templateBuilder.setTemplateMethod(mangledName, methodBuilder.yield());

                    moduleContext->popTemplateBuilder();
                }
            }
            templateBuilder.setTemplateDefinition(builder.yield());
            irModule->structTemplateTable.put_create(structName, templateBuilder.yield());
            irModule->structTemplateAsts[structName] = structDefStmt;

            moduleContext->popTemplateBuilder();
        } else {
            auto structIndex = irModule->structTable.put(structName, {});

            IRStructDefinition::Builder builder;
            builder.setName(structName);
            for (auto &i : structDefStmt->getInner().getInner()) {
                switch (i->kind) {
                    case 0: {
                        auto memberName = i->getVar().getId().get().strVal;
                        auto memberType = managedPtr(parseTypeSpec(i->getVar().spec));
                        builder.addField(memberName, memberType);
                        break;
                    }
                    case 1: {
                        IRFunctionDefinition::Builder constructorBuilder;
                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                        auto thisType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                        constructorBuilder.setReturnType(thisType);
                        constructorBuilder.addArgument(L"this", thisType);

                        for (auto &arg : i->getConstructor().getArgs().get()) {
                            auto argName = arg->getId().get().strVal;
                            auto argType = managedPtr(parseTypeSpec(arg->spec));
                            constructorBuilder.addArgument(argName, argType);
                            argTypes.push_back(argType);
                        }
                        auto uniq = getFuncUniqueNameStr(argTypes);
                        auto mangledName = L"constructor" + uniq;
                        constructorBuilder.setName(structName + L"::" + mangledName);
                        
                        auto func = constructorBuilder.yield();
                        builder.addMethod(mangledName, irModule->functionTable.put_create(func->name, func));
                        break;
                    }
                    case 2: {
                        auto methodName = i->getMethod().getName().get().strVal;
                        auto methodType = managedPtr(parseTypeSpec(i->getMethod().resultType));
                        IRFunctionDefinition::Builder methodBuilder;
                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                        methodBuilder.setReturnType(methodType);
                        auto thisType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                        methodBuilder.addArgument(L"this", thisType);

                        for (auto &arg : i->getMethod().getArgs().get()) {
                            auto argName = arg->getId().get().strVal;
                            auto argType = managedPtr(parseTypeSpec(arg->spec));
                            methodBuilder.addArgument(argName, argType);
                            argTypes.push_back(argType);
                        }

                        auto uniq = getFuncUniqueNameStr(argTypes);
                        auto mangledName = methodName + uniq;
                        methodBuilder.setName(structName + L"::" + mangledName);
                        auto func = methodBuilder.yield();
                        builder.addMethod(mangledName, irModule->functionTable.put_create(func->name, func));
                        break;
                    }
                }
            }
            auto structType = builder.yield();
            irModule->structTable[structIndex] = structType;
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::implStmt *implStmt) {
        auto& structIdNode = implStmt->getStructId();
        auto structBaseName = structIdNode.getId().get().strVal;

        if (structIdNode.hasTemplateArg()) { // Impl for a template struct
            yoi_assert(irModule->structTemplateTable.contains(structBaseName),
                implStmt->getLine(), implStmt->getColumn(), "Impl for undefined struct template: " + wstring2string(structBaseName));
            
            yoi::vec<std::shared_ptr<IRValueType>> concreteTemplateArgs = parseTemplateArgs(structIdNode.getArg());
            
            if (concreteTemplateArgs.empty()) {
                // pure template struct, store them and specialize when used
                irModule->templateImplAsts[structBaseName] = implStmt;
            } else {
                // specialize template struct
                auto structTemplateAst = irModule->structTemplateAsts[structBaseName];
                specializeStructTemplate(structBaseName, concreteTemplateArgs, implStmt);
            }
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

        if (implStmt->isImplForStmt()) {
            auto interfaceName = parseInterfaceName(implStmt->interfaceName);
            indexT structIndex;
            try {
                structIndex = irModule->structTable.getIndex(structBaseName);
            } catch(std::runtime_error &e) {
                panic(implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(structBaseName));
                return {};
            }
            auto interfaceImplName = getInterfaceImplName(interfaceName.first, std::make_pair(currentModuleIndex, structIndex));

            auto interfaceImplIndex = irModule->interfaceImplementationTable.put(interfaceImplName, {});
            IRInterfaceImplementationDefinition::Builder builder;
            builder.setName(interfaceImplName);
            builder.setImplStructIndex(structIndex);
            builder.setImplInterfaceIndex(interfaceName.first.second);

            for (auto &i : implStmt->getInner().getInner()) {
                yoi_assert(!i->isConstructor(), i->getLine(), i->getColumn(), "Constructor cannot be implemented for interface");
                
                auto methodName = i->getMethod().getName().get().strVal;
                IRFunctionDefinition::Builder methodBuilder;
                yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                
                auto thisType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                methodBuilder.addArgument(L"this", thisType);

                for (auto &arg : i->getMethod().getArgs().get()) {
                    auto argName = arg->getId().get().strVal;
                    auto argType = managedPtr(parseTypeSpec(arg->spec));
                    methodBuilder.addArgument(argName, argType);
                    argTypes.push_back(argType);
                }
                auto uniq = getFuncUniqueNameStr(argTypes);
                methodBuilder.setReturnType(managedPtr(parseTypeSpec(i->getMethod().resultType)));
                methodBuilder.setName(structBaseName + L"::" + methodName + uniq);

                auto func = methodBuilder.yield();
                auto funcIndex = irModule->functionTable.put_create(func->name, func);
                builder.addVirtualMethod(methodName + uniq, managedPtr(IRValueType{IRValueType::valueType::virtualMethod, currentModuleIndex, funcIndex}));

                moduleContext->pushIRBuilder(IRBuilder{moduleContext->getCompilerContext(), irModule, func});
                moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                visit(i->getMethod().block, true);
                moduleContext->getIRBuilder().yield();
                moduleContext->popIRBuilder();
            }
            irModule->interfaceImplementationTable[interfaceImplIndex] = builder.yield();
        } else {
            indexT structIndex;
            try {
                structIndex = irModule->structTable.getIndex(structBaseName);
            } catch(std::runtime_error &e) {
                panic(implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(structBaseName));
            }

            for (auto &i : implStmt->getInner().getInner()) {
                yoi::wstr mangledName;
                if (i->isConstructor()) {
                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                    for (auto &arg : i->getConstructor().getArgs().get()) {
                        argTypes.push_back(managedPtr(parseTypeSpec(arg->spec)));
                    }
                    mangledName = structBaseName + L"::constructor" + getFuncUniqueNameStr(argTypes);
                } else {
                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                    for (auto &arg : i->getMethod().getArgs().get()) {
                        argTypes.push_back(managedPtr(parseTypeSpec(arg->spec)));
                    }
                    mangledName = structBaseName + L"::" + i->getMethod().getName().get().strVal + getFuncUniqueNameStr(argTypes);
                }

                try {
                    auto funcIndex = irModule->functionTable.getIndex(mangledName);
                    auto func = irModule->functionTable[funcIndex];
                    moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
                    moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                    visit(i->isConstructor() ? i->getConstructor().block : i->getMethod().block, true);
                    moduleContext->getIRBuilder().yield();
                    moduleContext->popIRBuilder();
                } catch (std::out_of_range &e) {
                     panic(i->getLine(), i->getColumn(), "No matched constructor or method declaration found for impl: " + wstring2string(mangledName));
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
                visit(globalStmt->value.useStmtVal);
                break;
            }
            case globalStmt::vKind::implStmt: {
                visit(globalStmt->value.implStmtVal);
                break;
            }
            case globalStmt::vKind::letStmt: {
                visit(globalStmt->value.letStmtVal);
                break;
            }
            case globalStmt::vKind::funcDefStmt: {
                visit(globalStmt->value.funcDefStmtVal);
                break;
            }
            case globalStmt::vKind::structDefStmt: {
                visit(globalStmt->value.structDefStmtVal);
                break;
            }
            case globalStmt::vKind::interfaceDefStmt: {
                visit(globalStmt->value.interfaceDefStmtVal);
                break;
            }
            case globalStmt::vKind::importDecl: {
                visit(globalStmt->value.importDeclVal);
                break;
            }
            case globalStmt::vKind::exportDecl: {
                visit(globalStmt->value.exportDeclVal);
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
        auto outBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, ifBlock);
        auto back = moduleContext->getIRBuilder().switchCodeBlock(ifBlock);
        visit(ifStmt->getIfBlock().block, true);
        moduleContext->getIRBuilder().jumpOp(outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(back);

        for (auto &i : ifStmt->elifB) {
            visit(i.cond);
            auto elifCondType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            yoi_assert(elifCondType->type == IRValueType::valueType::booleanObject, i.cond->getLine(), i.cond->getColumn(), "The type in elif-condition must be boolean");

            auto elifBlock = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, elifBlock);
            auto back = moduleContext->getIRBuilder().switchCodeBlock(elifBlock);
            visit(i.block, true);
            moduleContext->getIRBuilder().jumpOp(outBlock);
            moduleContext->getIRBuilder().switchCodeBlock(back);
        }

        if (ifStmt->hasElseBlock()) {
            auto elseBlock = moduleContext->getIRBuilder().createCodeBlock();
            moduleContext->getIRBuilder().jumpOp(elseBlock);
            auto back = moduleContext->getIRBuilder().switchCodeBlock(elseBlock);
            visit(ifStmt->elseB, true);
            moduleContext->getIRBuilder().jumpOp(outBlock);
            moduleContext->getIRBuilder().switchCodeBlock(back);
        }
        moduleContext->getIRBuilder().switchCodeBlock(outBlock);
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
            // validate type
            auto returnType = moduleContext->getIRBuilder().irFuncDefinition()->returnType;
            emitBasicCastTo(returnType);
            moduleContext->getIRBuilder().retOp();
        } else {
            // TODO: return void
            moduleContext->getIRBuilder().retOp(true);
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
            auto baseName = identifierWithTemplateArg->getId().node.strVal;
            yoi_assert(irModule->structTemplateTable.contains(baseName),
                identifierWithTemplateArg->getLine(), identifierWithTemplateArg->getColumn(), "Unknown struct template: " + wstring2string(baseName));
            
            auto concreteTypes = parseTemplateArgs(identifierWithTemplateArg->getArg());
            try {
                auto pureTemplateAst = irModule->templateImplAsts.at(baseName);

                auto specializedIndex = specializeStructTemplate(baseName, concreteTypes, pureTemplateAst);
                
                return {IRValueType::valueType::structObject, currentModuleIndex, specializedIndex};
            } catch (std::out_of_range &e) {
                panic(identifierWithTemplateArg->getLine(), identifierWithTemplateArg->getColumn(), "No matched struct template found for: " + wstring2string(baseName));
            }

        } else {
            return parseTypeSpec(identifierWithTemplateArg->id);
        }
    }

    IRValueType visitor::parseTypeSpec(yoi::subscriptExpr *subscriptExpr) {
        if (!subscriptExpr->getSubscript().empty()) {
            // This is an array type like `int[10]`
            auto baseType = parseTypeSpec(subscriptExpr->id);
            yoi::vec<yoi::indexT> dimensions;
            for (auto& sub : subscriptExpr->getSubscript()) {
                yoi_assert(sub->isSubscript(), sub->getLine(), sub->getColumn(), "Expected dimension size for array type.");
                // For type parsing, we don't evaluate the expression, but we'd need a way
                // to represent the dimension. For now, let's assume it's a literal.
                // A full implementation would require constant expression evaluation here.
                panic(sub->getLine(), sub->getColumn(), "TODO: Array type specifier with non-literal size not implemented yet.");
            }
            // TODO: Create and return an array type.
            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "TODO: Array type parsing not fully implemented.");
            return {IRValueType::valueType::null};
        } else {
            return parseTypeSpec(subscriptExpr->id);
        }
    }


    IRValueType visitor::parseTypeSpecExtern(yoi::identifier *identifier, yoi::indexT targetModule) {
        auto mod = moduleContext->getCompilerContext()->getImportedModule(targetModule);
        auto exId = addExternEntryIfNotExists(targetModule, identifier->node.strVal);
        auto ex = irModule->externTable[exId];
        yoi_assert(ex->type == IRExternEntry::externType::structType, identifier->getLine(), identifier->getColumn(), "Invalid type specifier, expected struct type");
        return {IRValueType::valueType::structObject, static_cast<yoi::indexT>(currentModuleIndex), exId};
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
        if (!subscriptExpr->getSubscript().empty()) {
            auto baseType = parseTypeSpecExtern(subscriptExpr->id, targetModule);
            // TODO: Create and return an extern array type, creating extern entries for dimensions if necessary.
            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "TODO: Extern array type parsing not fully implemented.");
            return {IRValueType::valueType::null};
        } else {
            return parseTypeSpecExtern(subscriptExpr->id, targetModule);
        }
    }

    IRValueType visitor::parseTypeSpec(yoi::typeSpec *typeSpec) {
        switch (typeSpec->kind) {
            case 0: {
                // member
                auto it = typeSpec->member->getTerms().begin();
                yoi::indexT targetModule = -1;
                while (it + 1 != typeSpec->member->getTerms().end() && (targetModule = isModuleName(*it, targetModule)) != -1) {
                    it++;
                }

                IRValueType lhs{IRValueType::valueType::integerObject};
                if (targetModule == -1) {
                    lhs = parseTypeSpec(*it);
                } else {
                    lhs = parseTypeSpecExtern(*it, targetModule);
                }
                yoi_assert(it + 1 == typeSpec->member->getTerms().end(), typeSpec->getLine(), typeSpec->getColumn(), "Type specifier is not valid.");
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

    yoi::wstr visitor::parseIdentifierWithTemplateArg(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        yoi::wstr res = identifierWithTemplateArg->getId().node.strVal;
        if (identifierWithTemplateArg->hasTemplateArg()) {
            res += L"<";
            for (auto &i : identifierWithTemplateArg->getArg().get()) {
                res += parseTypeSpec(i->spec).to_string();
                res += L",";
            }
            res.pop_back();
            res += L">";
        }
        return std::move(res);
    }

    yoi::wstr visitor::getInterfaceImplName(const std::pair<yoi::indexT, yoi::indexT> &interfaceSrc,
        const std::pair<yoi::indexT, yoi::indexT> &structSrc) {
        return L"interfaceImpl#" + std::to_wstring(interfaceSrc.first) + L"#" + std::to_wstring(interfaceSrc.second) + L"#" + std::to_wstring(structSrc.first) + L"#" + std::to_wstring(structSrc.second);
    }

    std::pair<std::pair<yoi::indexT, yoi::indexT>, std::shared_ptr<IRInterfaceInstanceDefinition>> visitor::parseInterfaceName(
        yoi::externModuleAccessExpression *structDef) {
        // modules~
        auto it = structDef->getTerms().begin();
        yoi::indexT targetModule = -1;
        while (it + 1 != structDef->getTerms().end() && (targetModule = isModuleName(*it, targetModule)) != -1) {
            it++;
        }
        if (targetModule == -1) {
            targetModule = currentModuleIndex;
        }
        yoi_assert(it + 1 == structDef->getTerms().end(), structDef->getLine(), structDef->getColumn(), "Invalid interface name");
        auto interfaceName = parseIdentifierWithTemplateArg(*it);
        try {
            auto target = moduleContext->getCompilerContext()->getImportedModule(targetModule);
            auto interfaceIndex = target->interfaceTable.getIndex(interfaceName);
            return std::make_pair(std::make_pair(targetModule, interfaceIndex), target->interfaceTable[interfaceIndex]);
        } catch (std::out_of_range &) {
            panic(structDef->getLine(), structDef->getColumn(), "Undefined interface: " + wstring2string(interfaceName));
        }

    }

    yoi::indexT visitor::isModuleName(identifierWithTemplateArg *it, yoi::indexT currentModule) const {
        if(!it->hasTemplateArg()) {
            std::shared_ptr<yoi::IRModule> target = currentModule == -1 ? irModule : moduleContext->getCompilerContext()->getImportedModule(currentModule);
            if (auto x = target->moduleImports.find(it->getId().node.strVal); x != target->moduleImports.end() ) {
                return x->second;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    yoi::IRExternEntry visitor::getExternEntry(yoi::indexT moduleIndex, const yoi::wstr &identifier) const {
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->globalVariables.getIndex(identifier);
            return {IRExternEntry::externType::globalVar, identifier, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->functionTable.getIndex(identifier);
            return {IRExternEntry::externType::function, identifier, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->structTable.getIndex(identifier);
            return {IRExternEntry::externType::structType, identifier, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->interfaceTable.getIndex(identifier);
            return {IRExternEntry::externType::interfaceType, identifier, moduleIndex, res};
        } catch (std::runtime_error &) {}
        try {
            auto res = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->interfaceImplementationTable.getIndex(identifier);
            return {IRExternEntry::externType::interfaceImplType, identifier, moduleIndex, res};
        } catch (std::runtime_error &) {}

        panic(0, 0, "undefined identifier: not known global variable, function, struct or interface type");
        return {};
    }


    yoi::indexT visitor::addExternEntryIfNotExists(yoi::indexT moduleIndex, const yoi::wstr &identifier) {
        // extern entry format: moduleIndex#identifier
        yoi::wstr key = std::to_wstring(moduleIndex) + L"#" + identifier;
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

    void visitor::emitBasicCastInBasicArithOpByLhsAndRhs(yoi::indexT lhs, yoi::indexT rhs) {
        auto lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
        auto rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

        // if bool and char with other, upcast to other
        if (lhsType->is1ByteType() && !rhsType->is1ByteType()) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        }
        else if (!lhsType->is1ByteType() && rhsType->is1ByteType()) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        }
        // if int with deci, upcast to deci
        else if (lhsType->type == IRValueType::valueType::integerObject && rhsType->type == IRValueType::valueType::decimalObject) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        }
        else if (lhsType->type == IRValueType::valueType::decimalObject && rhsType->type == IRValueType::valueType::integerObject) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        }

    }

    void visitor::emitBasicCastTo(const std::shared_ptr<IRValueType> &toType) {
        auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        
        if (rhs->type == toType->type) {
            return;
        } else { 
            moduleContext->getIRBuilder().basicCast(toType, moduleContext->getIRBuilder().getCurrentInsertionPoint());
        }
    }

    yoi::wstr visitor::getInterfaceNameStr(const std::pair<yoi::indexT, yoi::indexT> &interfaceSrc) {
        return L"interface#" + std::to_wstring(interfaceSrc.first) + L"#" + std::to_wstring(interfaceSrc.second);
    }

    yoi::wstr visitor::getTypeSpecUniqueNameStr(const std::shared_ptr<IRValueType> &type) {
        yoi::wstr res;
        switch (type->type) {
        case IRValueType::valueType::integerObject:
            res = L"int";
            break;
        case IRValueType::valueType::decimalObject:
            res = L"deci";
            break;
        case IRValueType::valueType::booleanObject:
            res = L"bool";
            break;
        case IRValueType::valueType::stringObject:
            res = L"str";
            break;
        case IRValueType::valueType::structObject:
            res = L"struct#" + std::to_wstring(type->typeAffiliateModule) + L"#" + std::to_wstring(type->typeIndex);
            break;
        case IRValueType::valueType::null:
            res = L"null";
            break;
        case IRValueType::valueType::virtualMethod:
            res = L"virtual_method#" + std::to_wstring(type->typeAffiliateModule) + L"#" + std::to_wstring(type->typeIndex);
            break;
        case IRValueType::valueType::incompleteTemplateType:
            res = L"incomplete_template_type#" + std::to_wstring(type->typeIndex);
            break;
        default:
            panic(0, 0, "Invalid type");
            break;
        }
        return res;
    }

    yoi::wstr visitor::getFuncUniqueNameStr(const std::vector<std::shared_ptr<IRValueType>> &argumentTypes, bool whetherIgnoreFirstParam) {
        yoi::wstr res = L"#";
        bool first = false;
        for (auto &arg : argumentTypes) {
            if (whetherIgnoreFirstParam && !first || !whetherIgnoreFirstParam) res += getTypeSpecUniqueNameStr(arg) + L"#";
            else first = false;
        }
        if (!argumentTypes.empty()) {
            res.pop_back();
        }
        res.shrink_to_fit();
        return res;
    }

    yoi::indexT visitor::visitExtern(yoi::identifier *identifier, yoi::indexT targetModule, bool isStoreOp) {
        try {
            auto entryIndex = addExternEntryIfNotExists(targetModule, identifier->get().strVal);
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

    std::shared_ptr<IRValueType> visitor::getIncompleteType(const yoi::wstr &typeName) const {
        try {
            if (moduleContext->getTemplateBuilders().empty()) {
                throw std::out_of_range("No template builder found");
            }
            auto &templateBuilder = **moduleContext->getTemplateBuilders().rbegin();
            auto res = templateBuilder.templateArguments[typeName];
            return res.templateType;
        } catch (std::out_of_range &e) {
            throw std::out_of_range("Cannot find incomplete type: " + yoi::wstring2string(typeName));
        }
    }
    yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument>
    visitor::getTemplateArgs(const yoi::defTemplateArg &templateArgs) {
        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> res;
        for (auto &arg : templateArgs.spec) {
            yoi::wstr name = arg->getId().get().strVal;
            auto index = res.put_create(name, {{}, {-1, -1}});
            res[index].templateType = managedPtr(IRValueType{
                IRValueType::valueType::incompleteTemplateType, currentModuleIndex, index});

            if (arg->impl) {
                auto constraint = parseInterfaceName(arg->impl);
                res[index].interfaceType = constraint.first;
            }
        }
        return res;
    }
    yoi::vec<std::shared_ptr<IRValueType>>
    visitor::parseTemplateArgs(const yoi::templateArg &templateArgs) {
        yoi::vec<std::shared_ptr<IRValueType>> res;
        for (auto &arg : templateArgs.spec) {
            res.push_back(managedPtr(parseTypeSpec(arg->spec)));
        }
        return res;
    }
    yoi::indexT visitor::specializeFunctionTemplate(
        const std::shared_ptr<IRFunctionTemplate> &templateFunc,
        yoi::funcDefStmt* astNode,
        const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs) {
        
        yoi::wstr specializedName = getMangledTemplateName(templateFunc->templateDefinition->name, concreteTemplateArgs);

        if (irModule->functionTable.contains(specializedName)) {
            return irModule->functionTable.getIndex(specializedName);
        }

        // Create a new function definition by specializing the template
        IRFunctionDefinition::Builder builder;
        builder.setName(specializedName);

        // Create specialization context
        IRTemplateBuilder specializationContext;
        for (yoi::indexT i = 0; i < templateFunc->templateArguments.size(); ++i) {
            auto paramName = templateFunc->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        moduleContext->pushTemplateBuilder(specializationContext);

        // Specialize arguments and return type
        for (const auto& argPair : astNode->getArgs().get()) {
            auto argName = argPair->getId().get().strVal;
            auto argType = managedPtr(parseTypeSpec(&argPair->getSpec()));
            builder.addArgument(argName, argType);
        }
        builder.setReturnType(managedPtr(parseTypeSpec(&astNode->getResultType())));

        auto specializedFunc = builder.yield();
        auto funcIndex = irModule->functionTable.put_create(specializedName, specializedFunc);

        // Visit the body to generate IR
        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, specializedFunc});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(&astNode->getBlock(), true);
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();
        
        // Pop context
        moduleContext->popTemplateBuilder();
        
        return funcIndex;
    }

    yoi::wstr visitor::getMangledTemplateName(const yoi::wstr& baseName, const yoi::vec<std::shared_ptr<IRValueType>>& templateArgs) {
        yoi::wstr mangled = baseName + L"<";
        for (size_t i = 0; i < templateArgs.size(); ++i) {
            mangled += getTypeSpecUniqueNameStr(templateArgs[i]);
            if (i < templateArgs.size() - 1) {
                mangled += L",";
            }
        }
        mangled += L">";
        return mangled;
    }
    yoi::indexT visitor::specializeStructTemplate(
        const yoi::wstr& templateName,
        const yoi::vec<std::shared_ptr<IRValueType>>& concreteTemplateArgs,
        yoi::implStmt *pureTemplateImplAst
    ) {

        yoi::wstr specializedName = getMangledTemplateName(templateName, concreteTemplateArgs);

        if (irModule->structTable.contains(specializedName)) {
            return irModule->structTable.getIndex(specializedName);
        }

        yoi_assert(irModule->structTemplateTable.contains(templateName), 0, 0, "Unknown struct template: " + wstring2string(templateName));
        auto structTemplate = irModule->structTemplateTable[templateName];
        auto structAst = irModule->structTemplateAsts.at(templateName);

        IRTemplateBuilder specializationContext;
        printf("specializing struct template %s with args: %lu %llu\n", wstring2string(templateName).c_str(), concreteTemplateArgs.size(), structTemplate->templateArguments.size());
        yoi_assert(concreteTemplateArgs.size() == structTemplate->templateArguments.size(),
                0, 0, "Template argument count mismatch for struct " + wstring2string(templateName));
        
        for (yoi::indexT i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structTemplate->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        
        auto specializedStructIndex = irModule->structTable.put_create(specializedName, nullptr);
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, specializedStructIndex});

        specializationContext.addTemplateArgument(L"STRUCT", selfType);

        moduleContext->pushTemplateBuilder(specializationContext);

        IRStructDefinition::Builder builder;
        builder.setName(specializedName);

        for (auto& field : structAst->getInner().getInner()) {
            if (field->kind == 0) { // is a variable/field
                auto memberName = field->getVar().getId().get().strVal;
                auto memberType = managedPtr(parseTypeSpec(field->getVar().spec));
                builder.addField(memberName, memberType);
            }
        }
        
        auto specializedStruct = builder.yield();
        irModule->structTable[specializedStructIndex] = specializedStruct;

        for (auto& methodAst : pureTemplateImplAst->getInner().getInner()) {
            specializeStructMethod(structTemplate, specializedStruct, methodAst, specializedName, concreteTemplateArgs);
        }

        moduleContext->popTemplateBuilder();

        return specializedStructIndex;
    }

    void visitor::specializeStructMethod(
        const std::shared_ptr<IRStructTemplate>& structTemplate,
        const std::shared_ptr<IRStructDefinition>& specializedStruct,
        yoi::implInnerPair* methodAstNode,
        const yoi::wstr& specializedStructName,
        const yoi::vec<std::shared_ptr<IRValueType>>& concreteTemplateArgs) {

        
        // Push the template's own context to resolve generic types like 'T' to their placeholder 'incompleteTemplateType'.
        IRTemplateBuilder genericContext;
        genericContext.templateArguments = structTemplate->templateArguments;
        genericContext.addTemplateArgument(L"STRUCT", managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, irModule->structTable.getIndex(specializedStructName)}));
        moduleContext->pushTemplateBuilder(genericContext);

        yoi::wstr baseMethodName;
        yoi::wstr genericMethodKey;
        yoi::vec<std::shared_ptr<IRValueType>> genericArgTypes;

        if (methodAstNode->isConstructor()) {
            baseMethodName = L"constructor";
            for (auto& arg : methodAstNode->getConstructor().getArgs().get()) {
                genericArgTypes.push_back(managedPtr(parseTypeSpec(&arg->getSpec())));
            }
            genericMethodKey = baseMethodName + getFuncUniqueNameStr(genericArgTypes);
        } else {
            baseMethodName = methodAstNode->getMethod().getName().get().strVal;
            for (auto& arg : methodAstNode->getMethod().getArgs().get()) {
                genericArgTypes.push_back(managedPtr(parseTypeSpec(&arg->getSpec())));
            }
            genericMethodKey = baseMethodName + getFuncUniqueNameStr(genericArgTypes);
        }

        moduleContext->popTemplateBuilder(); // Done with generic context
        
        std::shared_ptr<IRFunctionTemplate> methodTemplate;
        // yoi_assert(, 0, 0, "Method implementation does not match any template method definition: " + wstring2string(genericMethodKey));
        for (auto &methodPair : structTemplate->templateMethods) {
            if (methodPair.first == genericMethodKey) {
                methodTemplate = methodPair.second;
            } else {
                if (getSpecializedMangledMethodName(genericContext.templateArguments, methodPair.first, concreteTemplateArgs) == genericMethodKey) {
                    methodTemplate = methodPair.second;
                }
            }
        }

        if (!methodTemplate) {
            throw std::runtime_error("Method implementation does not match any template method definition: " + wstring2string(genericMethodKey));
        }
        
        IRTemplateBuilder specializationContext;
        for(size_t i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structTemplate->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, irModule->structTable.getIndex(specializedStructName)});
        specializationContext.addTemplateArgument(L"STRUCT", selfType);
        moduleContext->pushTemplateBuilder(specializationContext);

        IRFunctionDefinition::Builder funcBuilder;
        yoi::vec<std::shared_ptr<IRValueType>> specializedArgTypes;
        
        funcBuilder.addArgument(L"this", selfType); // Specialized 'this'

        if (methodAstNode->isConstructor()) {
            for (auto& arg : methodAstNode->getConstructor().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                funcBuilder.addArgument(arg->getId().get().strVal, specializedType);
                specializedArgTypes.push_back(specializedType);
            }
            funcBuilder.setReturnType(selfType);
        } else {
            for (auto& arg : methodAstNode->getMethod().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                funcBuilder.addArgument(arg->getId().get().strVal, specializedType);
                specializedArgTypes.push_back(specializedType);
            }
            funcBuilder.setReturnType(managedPtr(parseTypeSpec(&methodAstNode->getMethod().getResultType())));
        }

        yoi::wstr specializedMethodName = specializedStructName + L"::" + baseMethodName + getFuncUniqueNameStr(specializedArgTypes);
        funcBuilder.setName(specializedMethodName);

        auto specializedFunc = funcBuilder.yield();
        auto funcIndex = irModule->functionTable.put_create(specializedMethodName, specializedFunc);
        
        irModule->structTable[specializedStructName]->nameIndexMap[baseMethodName + getFuncUniqueNameStr(specializedArgTypes)] = {IRStructDefinition::nameInfo::nameType::method, funcIndex};

        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, specializedFunc});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(methodAstNode->isConstructor() ? &methodAstNode->getConstructor().getBlock() : &methodAstNode->getMethod().getBlock(), true);
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();

        moduleContext->popTemplateBuilder(); 
    }
    
    yoi::wstr visitor::getSpecializedMangledMethodName(
        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArgs,
        const yoi::wstr &baseMethodName,
        const yoi::vec<std::shared_ptr<IRValueType>> &specializedArgTypes) {
        auto res = baseMethodName;
        for (yoi::indexT i = 0; i < specializedArgTypes.size(); ++i) {
            auto &arg = templateArgs[i];
            auto strRepl1 = templateArgs[i].templateType->to_string();
            auto strRepl2 = specializedArgTypes[i]->to_string();
            replace_all(res, strRepl1, strRepl2);
        }
        return res;
    }

    yoi::indexT visitor::visit(yoi::exportDecl *exportDecl) {
        auto exportIdentifier = exportDecl->as->node.strVal;
        try {
            auto parsedType = managedPtr(parseTypeSpec(exportDecl->from));

            moduleContext->getCompilerContext()->getIRFFITable()->addForeignType(exportIdentifier, parsedType);
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } catch (std::runtime_error &e) {
            // failed as type, try function
        }

        try {
            auto it = exportDecl->from->member->getTerms().begin();
            yoi::indexT targetModule = -1;
            while (it + 1 != exportDecl->from->member->getTerms().end() && (targetModule = isModuleName(*it, targetModule)) != -1) {
                it++;
            }
            yoi_assert(it + 1 == exportDecl->from->member->getTerms().end(), exportDecl->getLine(), exportDecl->getColumn(), "Expected a identifier after modules but this is not the final term of expression.");
            targetModule = targetModule == -1 ? currentModuleIndex : targetModule;
            

            yoi::indexT funcIndex = -1;

            if (!(*it)->hasTemplateArg()) {
                for (auto funcIt = moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable.begin(); funcIt != moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable.end(); funcIt++) {
                    if (funcIt->first.starts_with((*it)->id->get().strVal + L"#")) {
                        // an mangled name of target function
                        funcIndex = std::distance(moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable.begin(), funcIt);
                    }
                }
            }

            if (funcIndex != -1) {
                moduleContext->getCompilerContext()->getIRFFITable()->addExportedFunction(exportIdentifier, targetModule, funcIndex);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            } else {
                // try template
                auto templateName = (*it)->id->get().strVal;
                auto templateIndex = moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTemplateTable.getIndex(templateName);
                yoi_assert((*it)->hasTemplateArg(), exportDecl->getLine(), exportDecl->getColumn(), "Expected template arguments for template: " + wstring2string(templateName));

                auto templateArgs = parseTemplateArgs((*it)->getArg());
                
                auto funcIndex = specializeFunctionTemplate(
                    moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTemplateTable[templateName], 
                    moduleContext->getCompilerContext()->getImportedModule(targetModule)->funcTemplateAsts[templateName], 
                    templateArgs);

                moduleContext->getCompilerContext()->getIRFFITable()->addExportedFunction(exportIdentifier, targetModule, funcIndex);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            }
        } catch (std::out_of_range &e) {
            panic(exportDecl->getLine(), exportDecl->getColumn(), "None of the existing types and functions match the name: " + wstring2string(exportDecl->as->node.strVal));
        }
    }

    yoi::indexT visitor::visit(yoi::importDecl *importDecl) {
        yoi::wstr from{};

        if (importDecl->from_path.strVal == L"builtin") {
            from = L"builtin";
        } else {
            for (auto &prep : moduleContext->getCompilerContext()->getBuildConfig()->searchPaths) {
                try {
                    std::filesystem::path final = std::filesystem::path(prep) / importDecl->from_path.strVal;
                    from = realpath(final.wstring());
                    break;
                } catch (std::runtime_error &e) {
                    continue;
                }
            }
        }
        yoi_assert(!from.empty(), importDecl->getLine(), importDecl->getColumn(), "Cannot find the file: " + wstring2string(importDecl->from_path.strVal));


        // import method implementation
        // parse method signature and add to import table
        auto funcName = importDecl->inner->name->get().strVal;
        IRFunctionDefinition::Builder builder;
        builder.setReturnType(managedPtr(parseTypeSpec(importDecl->inner->resultType)));
        for (auto &arg : importDecl->inner->args->get()) {
            auto argType = managedPtr(parseTypeSpec(&arg->getSpec()));
            builder.addArgument(arg->getId().get().strVal, argType);
        }
        builder.setName(funcName);
        auto importedFunc = builder.yield();
        auto importedIndex = moduleContext->getCompilerContext()->getIRFFITable()->addImportedFunction(from, funcName, importedFunc);

        // add to extern table
        irModule->externTable.put_create(funcName, managedPtr(IRExternEntry{IRExternEntry::externType::importedFunction, funcName, moduleContext->getCompilerContext()->getIRFFITable()->importedLibraries.getIndex(from), importedIndex}));
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }
} // namespace yoi