//
// Created by XIaokang00010 on 2024/9/6.
//

#include "visitor.h"
#include "compiler/builtinModule.hpp"
#include "compiler/compilerContext.h"
#include "compiler/frontend/lexer.hpp"
#include "compiler/ir/IR.h"
#include "compiler/moduleContext.h"
#include "share/def.hpp"
#include "share/magic_enum.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace yoi {
    visitor::visitor(const std::shared_ptr<yoi::moduleContext> &moduleContext,
                     const std::shared_ptr<yoi::IRModule> &irModule,
                     yoi::indexT moduleIndex)
        : moduleContext(moduleContext), irModule(irModule), currentModuleIndex(moduleIndex) {}

    std::shared_ptr<yoi::IRModule> visitor::visit() {
        IRDebugInfo debugInfo{irModule->modulePath, 0, 0};
        auto globInitializer = managedPtr(IRFunctionDefinition{
            L"yoimiya_glob_initializer", {}, moduleContext->getCompilerContext()->getNoneObjectType(), {}, {}, debugInfo});
        irModule->functionTable.put(L"yoimiya_glob_initializer", globInitializer);
        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, globInitializer});
        moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, 0, 0});
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
                moduleContext->getIRBuilder().pushOp(
                    IR::Opcode::push_integer, {IROperand::operandType::integer, basicLiterals->node.basicVal.vInt});
                break;
            }
            case yoi::lexer::token::tokenKind::decimal: {
                moduleContext->getIRBuilder().pushOp(
                    IR::Opcode::push_decimal, {IROperand::operandType::decimal, basicLiterals->node.basicVal.vDeci});
                break;
            }
            case yoi::lexer::token::tokenKind::string: {
                auto literalIndex = irModule->stringLiteralPool.addStringLiteral(basicLiterals->node.strVal);
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_string,
                                                     {IROperand::operandType::stringLiteral, literalIndex});
                break;
            }
            case yoi::lexer::token::tokenKind::boolean: {
                moduleContext->getIRBuilder().pushOp(
                    IR::Opcode::push_boolean, {IROperand::operandType::boolean, basicLiterals->node.basicVal.vBool});
                break;
            }
            case yoi::lexer::token::tokenKind::character: {
                // TODO: add support for character literals
                panic(basicLiterals->node.line, basicLiterals->node.col, "Unsupported character literal");
                break;
            }
            case yoi::lexer::token::tokenKind::kNull: {
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_null, {});
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
                tryCastTo(valType);
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local,
                                                      {IROperand::operandType::localVar, yoi::indexT{index}});
            } else {
                moduleContext->getIRBuilder().loadOp(
                    IR::Opcode::load_local, {IROperand::operandType::localVar, yoi::indexT{index}}, valType);
            }
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } catch (std::runtime_error &e) {
            // let it go, try to find it in global variables
        }
        try {
            auto index = irModule->globalVariables.getIndex(id);
            auto valType = irModule->globalVariables[index];
            if (isStoreOp) {
                tryCastTo(valType);
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global,
                                                      {IROperand::operandType::globalVar, yoi::indexT{index}});
            } else {
                moduleContext->getIRBuilder().loadOp(
                    IR::Opcode::load_global, {IROperand::operandType::globalVar, yoi::indexT{index}}, valType);
            }
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } catch (std::out_of_range &e) {
            panic(identifier->node.line, identifier->node.col, "Undefined identifier: " + wstring2string(id));
        }
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
            case 3: {
                visit(primary->typeId);
                break;
            }
            case 4: {
                visit(primary->dynCast);
                break;
            }
            case 5: {
                visit(primary->newExpr);
                break;
            }
            default: {
                panic(primary->getLine(), primary->getColumn(), "Unexpected primary type");
            }
        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::abstractExpr *abstractExpr, bool isStoreOp) {
        if (abstractExpr->rhs) {
            yoi_assert(!isStoreOp, abstractExpr->getLine(), abstractExpr->getColumn(), "trying to apply store op on a boolean expression");
            moduleContext->getIRBuilder().saveState();
            visit(abstractExpr->lhs);
            auto lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            auto rhs = parseTypeSpec(abstractExpr->rhs);


            if (abstractExpr->op.kind == lexer::token::tokenKind::kImpl) {
                yoi_assert (rhs.type == IRValueType::valueType::interfaceObject, abstractExpr->rhs->getLine(), abstractExpr->rhs->getColumn(), "RHS of 'impl' operator must be an interface type");
                auto interfaceImplName = getInterfaceImplName({rhs.typeAffiliateModule, rhs.typeIndex}, lhs);
                moduleContext->getIRBuilder().restoreState();
                // push the boolean
                if (moduleContext->getCompilerContext()->getImportedModule(lhs->typeAffiliateModule)->interfaceImplementationTable.contains(interfaceImplName)) {
                    moduleContext->getIRBuilder().pushOp(IR::Opcode::push_boolean, {IROperand::operandType::boolean, IROperand::operandValue{true}});
                } else {
                    moduleContext->getIRBuilder().pushOp(IR::Opcode::push_boolean, {IROperand::operandType::boolean, IROperand::operandValue{false}});
                }
            } else if (abstractExpr->op.kind == lexer::token::tokenKind::kInterfaceOf) {
                yoi_assert(lhs->type == IRValueType::valueType::interfaceObject, abstractExpr->lhs->getLine(), abstractExpr->lhs->getColumn(), "LHS of 'interfaceof' operator must be an interface type");
                // cut off some possibilies here.
                auto key = std::make_tuple(rhs.type, rhs.typeAffiliateModule, rhs.typeIndex);
                auto &vec = moduleContext->getCompilerContext()->getImportedModule(lhs->typeAffiliateModule)->interfaceTable[lhs->typeIndex]->implementations;
                if (std::find(vec.begin(), vec.end(), key) != vec.end()) {
                    // emit typeid op and emit interfaceof
                    moduleContext->getIRBuilder().discardState();
                    moduleContext->getIRBuilder().typeIdOp(managedPtr(rhs));
                    moduleContext->getIRBuilder().interfaceOfOp();
                } else {
                    // push the boolean
                    moduleContext->getIRBuilder().restoreState();
                    moduleContext->getIRBuilder().pushOp(IR::Opcode::push_boolean, {IROperand::operandType::boolean, IROperand::operandValue{false}});

                }
            }
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        } else {
            return visit(abstractExpr->lhs, isStoreOp);
        }
    }

    yoi::indexT visitor::visit(yoi::uniqueExpr *uniqueExpr, bool isStoreOp) {
        visit(uniqueExpr->lhs, isStoreOp);

        switch (uniqueExpr->getOp().kind) {
            case lexer::token::tokenKind::incrementSign: {
                // TODO: add support for operator overloading
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();

                if (lhs->isBasicType()) {
                    moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::increment);
                } else {
                    handleUnaryOperatorOverload(L"operator++");
                }
                break;
            }
            case lexer::token::tokenKind::decrementSign: {
                // TODO: add support for operator overloading
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                if (lhs->isBasicType()) {
                    moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::decrement);
                } else {
                    handleUnaryOperatorOverload(L"operator--");
                }
                break;
            }
            case lexer::token::tokenKind::binaryNot: {
                // TODO: add support for operator overloading
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                if (lhs->isBasicType()) {
                    moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::bitwise_not);
                } else {
                    handleUnaryOperatorOverload(L"operator~");
                }
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
                    visit(leftExpr->lhs);
                    break;
                }
                case lexer::token::tokenKind::directAssignSign: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    tryCastTo(lhs);
                    moduleContext->getIRBuilder().insert({IR::Opcode::direct_assign, {}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
                    moduleContext->getIRBuilder().popFromTempVarStack();
                    moduleContext->getIRBuilder().popFromTempVarStack();
                    moduleContext->getIRBuilder().pushTempVar(lhs);
                    break;
                }
                case lexer::token::tokenKind::additionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        if (lhs->type == IRValueType::valueType::decimalObject &&
                            rhs->type == IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                        } else if (lhs->type == IRValueType::valueType::integerObject &&
                                rhs->type == IRValueType::valueType::decimalObject) {
                            moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                        }
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                    } else {
                        handleBinaryOperatorOverload(L"operator+=");
                    }
                    break;
                }
                case lexer::token::tokenKind::subtractionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        if (lhs->type == IRValueType::valueType::decimalObject &&
                            rhs->type == IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                        } else if (lhs->type == IRValueType::valueType::integerObject &&
                                rhs->type == IRValueType::valueType::decimalObject) {
                            moduleContext->getIRBuilder().basicCast(rhs, lhsPos);
                        }
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                    } else {
                        handleBinaryOperatorOverload(L"operator-=");
                    }
                    break;
                }
                case lexer::token::tokenKind::multiplicationAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        if (lhs->type == IRValueType::valueType::decimalObject &&
                            rhs->type == IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                        } else if (lhs->type == IRValueType::valueType::integerObject &&
                                rhs->type == IRValueType::valueType::decimalObject) {
                            moduleContext->getIRBuilder().basicCast(rhs, lhsPos);
                        }
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                    } else {
                        handleBinaryOperatorOverload(L"operator*=");
                    }
                    break;
                }
                case lexer::token::tokenKind::divisionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        if (lhs->type == IRValueType::valueType::decimalObject &&
                            rhs->type == IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(lhs, rhsPos);
                        } else if (lhs->type == IRValueType::valueType::integerObject &&
                                rhs->type == IRValueType::valueType::decimalObject) {
                            moduleContext->getIRBuilder().basicCast(rhs, lhsPos);
                        }
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::div);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                    } else {
                        handleBinaryOperatorOverload(L"operator/=");
                    }
                    break;
                }
                default: {
                    panic(leftExpr->getOp().line,
                          leftExpr->getOp().col,
                          "Unexpected left expression operator, received: " + wstring2string(leftExpr->getOp().strVal));
                }
            }
        } else {
            visit(leftExpr->lhs);
        }
        
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
            
            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::asterisk: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul); break;
                    case lexer::token::tokenKind::slash: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::div); break;
                    case lexer::token::tokenKind::percentSign: moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mod); break;                    
                    default: panic(op->line, op->col, "Unexpected multiplication expression operator");
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::asterisk: handleBinaryOperatorOverload(L"operator*"); break;
                    case lexer::token::tokenKind::slash: handleBinaryOperatorOverload(L"operator/"); break;
                    case lexer::token::tokenKind::percentSign: handleBinaryOperatorOverload(L"operator%"); break;
                    default: panic(op->line, op->col, "Unexpected multiplication expression operator");
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

            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::plus: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add); break;
                    case lexer::token::tokenKind::minus: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub); break;
                    default: panic(op->line, op->col, "Unexpected addition expression operator");
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::plus: handleBinaryOperatorOverload(L"operator+"); break;
                    case lexer::token::tokenKind::minus: handleBinaryOperatorOverload(L"operator-"); break;
                    default: panic(op->line, op->col, "Unexpected addition expression operator");
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

            if (lhsType->isBasicType() && rhsType->type == IRValueType::valueType::integerObject) {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryShiftLeft: moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::left_shift); break;
                    case lexer::token::tokenKind::binaryShiftRight: moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::right_shift); break;
                    default:  panic(op->line, op->col, "Unexpected shift expression operator"); break;
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryShiftLeft: handleBinaryOperatorOverload(L"operator<<"); break;
                    case lexer::token::tokenKind::binaryShiftRight: handleBinaryOperatorOverload(L"operator>>"); break;
                    default:  panic(op->line, op->col, "Unexpected shift expression operator"); break;
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

            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::lessThan: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_than); break;
                    case lexer::token::tokenKind::greaterThan: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_than); break;
                    case lexer::token::tokenKind::lessEqual: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::less_equal); break;
                    case lexer::token::tokenKind::greaterEqual: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::greater_equal); break;
                    default: panic(op->line, op->col, "Unexpected relational expression operator");
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::lessThan: handleBinaryOperatorOverload(L"operator<"); break;
                    case lexer::token::tokenKind::greaterThan: handleBinaryOperatorOverload(L"operator>"); break;
                    case lexer::token::tokenKind::lessEqual: handleBinaryOperatorOverload(L"operator<="); break;
                    case lexer::token::tokenKind::greaterEqual: handleBinaryOperatorOverload(L"operator>="); break;
                    default: panic(op->line, op->col, "Unexpected relational expression operator");
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

            if (lhsType->isBasicType() && rhsType->isBasicType() || lhsType->type == IRValueType::valueType::pointerObject || rhsType->type == IRValueType::valueType::pointerObject) {
                switch (op->kind) {
                    case lexer::token::tokenKind::equal: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::equal); break;
                    case lexer::token::tokenKind::notEqual: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::not_equal); break;
                    default: panic(op->line, op->col, "Unexpected equality expression operator"); return {};
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::equal: handleBinaryOperatorOverload(L"operator=="); break;
                    case lexer::token::tokenKind::notEqual: handleBinaryOperatorOverload(L"operator!="); break;
                    default: panic(op->line, op->col, "Unexpected equality expression operator"); return {};
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
            
            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryAnd: {
                        if (lhsType->isBasicType() && lhsType->type != IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(
                                moduleContext->getCompilerContext()->getIntObjectType(), lhsPos, true);
                        }
                        if (rhsType->isBasicType() && rhsType->type != IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(moduleContext->getCompilerContext()->getIntObjectType(),
                                                                    rhsPos);
                        }

                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_and);
                        break;
                    }
                    default: {
                        panic(op->line, op->col, "Unexpected and expression operator");
                        return {};
                    }
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryAnd: {
                        handleBinaryOperatorOverload(L"operator&");
                        break;
                    }
                    default: {
                        panic(op->line, op->col, "Unexpected and expression operator");
                        return {};
                    }
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

            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryXor: {
                        if (lhsType->isBasicType() && lhsType->type != IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(
                                (moduleContext->getCompilerContext()->getIntObjectType()), lhs, true);
                        }
                        if (rhsType->isBasicType() && rhsType->type != IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(
                                (moduleContext->getCompilerContext()->getIntObjectType()), rhs);
                        }

                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_xor);
                        break;
                    }
                    default: {
                        panic(op->line, op->col, "Unexpected exclusive expression operator");
                        return {};
                    }
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryXor: {
                        handleBinaryOperatorOverload(L"operator^");
                        break;
                    }
                    default: {
                        panic(op->line, op->col, "Unexpected exclusive expression operator");
                    }
                }
            }
            lhs = rhs;
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

            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryOr: {
                        if (lhsType->isBasicType() && lhsType->type != IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(
                                (moduleContext->getCompilerContext()->getIntObjectType()), lhs, true);
                        }
                        if (rhsType->isBasicType() && rhsType->type != IRValueType::valueType::integerObject) {
                            moduleContext->getIRBuilder().basicCast(
                                (moduleContext->getCompilerContext()->getIntObjectType()), rhs);
                        }

                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::bitwise_or);
                        break;
                    }
                    default: {
                        panic(op->line, op->col, "Unexpected inclusive expression operator");
                        return {};
                    }
                }
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryOr: {
                        handleBinaryOperatorOverload(L"operator|");
                        break;
                    }
                    default: {
                        panic(op->line, op->col, "Unexpected inclusive expression operator");
                        return {};
                    }
                }
            }
            lhs = rhs;
        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::logicalAndExpr *logicalAndExpr) {
        auto term = logicalAndExpr->getTerms().begin();
        auto op = logicalAndExpr->getOp().begin();
        auto lhs = visit(*term);
        // The type of the first term on the left
        auto lhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

        // Loop through chained operators, e.g., a && b && c
        for (; op != logicalAndExpr->getOp().end(); ++op) {
            auto exitWithTrueBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitWithFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithTrueBlock)
                .insert({IR::Opcode::push_boolean, {{IROperand::operandType::boolean, true}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithTrueBlock)
                .insert({IR::Opcode::jump, {IROperand{IROperand::operandType::codeBlock, exitBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});

            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithFalseBlock)
                .insert({IR::Opcode::push_boolean,
                        {IROperand{IROperand::operandType::boolean, IROperand::operandValue{false}}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithFalseBlock)
                .insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});

            switch (op->kind) {
                case lexer::token::tokenKind::logicAnd: {
                    // On the first iteration, this is the original LHS.
                    // On subsequent iterations, it's the boolean result of the previous operation.
                    yoi_assert(lhsType->isBasicType(), op->line, op->col, "Not basic type for logical and");
                    if (lhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast(
                            (moduleContext->getCompilerContext()->getBoolObjectType()), lhs, true);
                    }

                    // Short-circuit if the LHS (or intermediate result) is false
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, exitWithFalseBlock);

                    // If not short-circuited, evaluate the RHS
                    auto rhs = visit(*++term);
                    auto rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    yoi_assert(rhsType->isBasicType(), op->line, op->col, "Not basic type for logical and");
                    if (rhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast(
                            (moduleContext->getCompilerContext()->getBoolObjectType()), rhs);
                    }

                    // The result of the expression is now on the stack
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, exitWithFalseBlock);
                    moduleContext->getIRBuilder().jumpOp(exitWithTrueBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(exitBlock);

                    lhsType = moduleContext->getCompilerContext()->getBoolObjectType();
                    // because before the rhs became lhs taken into evaluation, jumpIf inst has consumed the rhs on the stack so we must push it back
                    moduleContext->getIRBuilder().pushTempVar(lhsType);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected logical and expression operator");
                    return {};
                }
            }
        }

        if (!logicalAndExpr->getOp().empty()) {
            // not single term, push a boolean result onto the stack
            moduleContext->getIRBuilder().pushTempVar(lhsType);
        }
        
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::logicalOrExpr *logicalOrExpr) {
        auto term = logicalOrExpr->getTerms().begin();
        auto op = logicalOrExpr->getOp().begin();
        auto lhs = visit(*term);
        // The type of the first term on the left
        auto lhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

        // Loop through chained operators, e.g., a || b || c
        for (; op != logicalOrExpr->getOp().end(); ++op) {
            auto exitWithTrueBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitWithFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
            auto exitBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithTrueBlock)
                .insert({IR::Opcode::push_boolean, {{IROperand::operandType::boolean, true}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithTrueBlock)
                .insert({IR::Opcode::jump, {IROperand{IROperand::operandType::codeBlock, exitBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});

            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithFalseBlock)
                .insert({IR::Opcode::push_boolean,
                        {IROperand{IROperand::operandType::boolean, IROperand::operandValue{false}}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
            moduleContext->getIRBuilder()
                .getCodeBlock(exitWithFalseBlock)
                .insert({IR::Opcode::jump, {{IROperand::operandType::codeBlock, exitBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});

            switch (op->kind) {
                case lexer::token::tokenKind::logicOr: {
                    yoi_assert(lhsType->isBasicType(), op->line, op->col, "Not basic type for logical or");
                    if (lhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast(
                            (moduleContext->getCompilerContext()->getBoolObjectType()), lhs, true);
                    }

                    // Short-circuit if the LHS (or intermediate result) is true
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, exitWithTrueBlock);

                    // If not short-circuited, evaluate the RHS
                    auto rhs = visit(*++term);
                    auto rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    yoi_assert(rhsType->isBasicType(), op->line, op->col, "Not basic type for logical or");
                    if (rhsType->type != IRValueType::valueType::booleanObject) {
                        moduleContext->getIRBuilder().basicCast(
                            (moduleContext->getCompilerContext()->getBoolObjectType()), rhs);
                    }

                    // The result of the expression is now on the stack
                    moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, exitWithTrueBlock);
                    moduleContext->getIRBuilder().jumpOp(exitWithFalseBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(exitBlock);

                    lhsType = moduleContext->getCompilerContext()->getBoolObjectType();
                    // same as before
                    moduleContext->getIRBuilder().pushTempVar(lhsType);
                    break;
                }
                default: {
                    panic(op->line, op->col, "Unexpected logical or expression operator");
                    return {};
                }
            }
        }

        if (!logicalOrExpr->getOp().empty()) {
            // not single term, push a boolean result onto the stack
            moduleContext->getIRBuilder().pushTempVar(lhsType);
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
        yoi::indexT targetModule = -1, lastModule = -1;
        while (it + 1 != memberExpr->getTerms().end() && (targetModule = isModuleName((*it)->id, lastModule)) != lastModule) {
            it++;
            lastModule = targetModule;
        }

        bool whetherLastTerm = it + 1 == memberExpr->getTerms().end();

        // find the possibility of static method call
        bool isStaticMethod = false;
        if (it + 1 != memberExpr->getTerms().end()) {
            std::shared_ptr<IRValueType> type{};
            try {
                type = managedPtr(parseTypeSpec(*it));
            } catch (std::runtime_error &e) {
            }
            if (type != nullptr && type->type == IRValueType::valueType::structObject) {
                auto memberName = *(++it);
                yoi_assert(!memberName->id->hasTemplateArg(),
                           memberName->getLine(),
                           memberName->getColumn(),
                           "Unexpected template argument for static method call");
                yoi_assert(!memberName->getSubscript().empty(),
                           memberName->getLine(),
                           memberName->getColumn(),
                           "Expected subscript for static method call");

                auto &sub = memberName->getSubscript().front();
                moduleContext->getIRBuilder().saveState();
                auto argTypes = evaluateArguments(sub->args);
                auto overload = resolveOverloadExtern(memberName->id->getId().node.strVal, argTypes, type->typeAffiliateModule, moduleContext->getCompilerContext()
                                             ->getImportedModule(type->typeAffiliateModule)
                                             ->structTable[type->typeIndex]);

                yoi_assert(overload.found(), memberExpr->getLine(), memberExpr->getColumn(), "Unresolved static method call");
                if (overload.isVariadic) {
                    for (size_t i = 0; i < overload.fixedArgCount; ++i) {
                        visit(sub->args->get()[i]);
                    }
                    auto variadicArgCount = argTypes.size() - overload.fixedArgCount;
                    if (variadicArgCount > 0) {
                        for (size_t i = 0; i < variadicArgCount; ++i) {
                            visit(sub->args->get()[i + overload.fixedArgCount]);
                            tryCastTo(overload.variadicElementType);
                        }
                        moduleContext->getIRBuilder().newArrayOp(overload.variadicElementType, {static_cast<yoi::indexT>(variadicArgCount)});
                    } else {
                        moduleContext->getIRBuilder().newArrayOp(overload.variadicElementType, {0});
                    }
                } else {
                    moduleContext->getIRBuilder().discardState();
                }
                moduleContext->getIRBuilder().invokeMethodOp(
                            overload.functionIndex, overload.function->argumentTypes.size() - 1, overload.function->returnType, false, true, type->typeAffiliateModule);
                isStaticMethod = true;
            } else {
            }
        }

        if (!isStaticMethod) {
            if (targetModule == -1) {
                visit(*it, isStoreOp && whetherLastTerm);
            } else {
                visitExtern(*it, targetModule, isStoreOp && whetherLastTerm);
            }
        }

        for (; it != memberExpr->getTerms().end();) {
            ++it;
            if (it == memberExpr->getTerms().end()) {
                break;
            }

            auto rhsIt = *it;
            auto termType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            for (auto &sub : rhsIt->getSubscript()) {
                if (sub->isInvocation()) {
                    if (termType->type == IRValueType::valueType::structObject) {
                        auto memberName = rhsIt->id;
                        handleInvocationExtern(
                            memberName->getId().get().strVal, 
                            sub->args, 
                            termType->typeAffiliateModule, 
                            termType);
                    } else if (termType->type == IRValueType::valueType::interfaceObject) {
                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                        for (auto &arg : sub->args->get()) {
                            visit(arg);
                            argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
                        }

                        auto methodName = rhsIt->id->getId().get().strVal + getFuncUniqueNameStr(argTypes);
                        auto methodIdx = moduleContext->getCompilerContext()
                                             ->getImportedModule(termType->typeAffiliateModule)
                                             ->interfaceTable[termType->typeIndex]
                                             ->methodMap.getIndex(methodName);
                        auto method = moduleContext->getCompilerContext()
                                          ->getImportedModule(termType->typeAffiliateModule)
                                          ->interfaceTable[termType->typeIndex]
                                          ->methodMap[methodIdx];
                        yoi_assert(method->argumentTypes.size() == sub->args->get().size(),
                                   rhsIt->getLine(),
                                   rhsIt->getColumn(),
                                   "Argument count does not match");
                        moduleContext->getIRBuilder().invokeVirtualOp(
                            methodIdx, sub->args->get().size(), method->returnType);
                    }
                } else if (sub->isSubscript()) {
                    panic(rhsIt->getLine(),
                          rhsIt->getColumn(),
                          "TODO: Subscript on member expression not implemented yet.");
                }
            }

            if (rhsIt->getSubscript().empty()) {
                if (termType->isArrayType() || termType->isDynamicArrayType()) {
                    if (rhsIt->id->getId().get().strVal == L"length") {
                        moduleContext->getIRBuilder().arrayLengthOp();
                    } else {
                        panic(rhsIt->getLine(),
                              rhsIt->getColumn(),
                              "expected `length` when member expression applied to array type");
                    }
                } else if (termType->type == IRValueType::valueType::structObject) {
                    auto memberName = rhsIt->id;
                    if (memberName->hasTemplateArg()) {
                        // TODO: what the heck is this
                    } else {
                        try {
                            auto nameInfo = moduleContext->getCompilerContext()
                                                ->getImportedModule(termType->typeAffiliateModule)
                                                ->structTable[termType->typeIndex]
                                                ->lookupName(memberName->getId().get().strVal);
                            switch (nameInfo.type) {
                                case IRStructDefinition::nameInfo::nameType::field: {
                                    auto tempVarType =
                                        moduleContext->getCompilerContext()->getImportedModule(termType->typeAffiliateModule)->structTable[termType->typeIndex]->fieldTypes[nameInfo.index];
                                    if (isStoreOp) {
                                        moduleContext->getIRBuilder().storeMemberOp(
                                            {IROperand::operandType::index, nameInfo.index});
                                    } else {
                                        moduleContext->getIRBuilder().loadMemberOp(
                                            {IROperand::operandType::index, nameInfo.index}, tempVarType);
                                    }
                                    break;
                                }
                                case IRStructDefinition::nameInfo::nameType::method: {
                                    panic(
                                        rhsIt->getLine(), rhsIt->getColumn(), "Method cannot be parsed without invocation");
                                }
                            }
                        } catch (std::out_of_range &e) {
                            panic(rhsIt->getLine(),
                                rhsIt->getColumn(),
                                "Undefined field or function: " + yoi::wstring2string(rhsIt->id->getId().get().strVal));
                        }
                        
                    }
                } else {
                    panic(rhsIt->getLine(),
                          rhsIt->getColumn(),
                          "member expression applied to non-struct, non-array type");
                }
                
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::visit(yoi::inCodeBlockStmt *inCodeBlockStmt) {
        moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, inCodeBlockStmt->getLine(), inCodeBlockStmt->getColumn()});
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
                moduleContext->getIRBuilder().popOp();
                break;
        }
    }

    yoi::indexT visitor::visit(yoi::subscriptExpr *subscriptExpr, bool isStoreOp) {
        if (subscriptExpr->getSubscript().empty()) {
            return visit(subscriptExpr->id, isStoreOp);
        }

        auto it = subscriptExpr->getSubscript().begin();
        auto end = subscriptExpr->getSubscript().end();
        auto &first_term = *it;

        bool isType = false;
        std::shared_ptr<IRValueType> baseType;
        try {
            baseType = managedPtr(parseTypeSpec(subscriptExpr->id));
            isType = true;
        } catch (const std::runtime_error &) {
            isType = false;
        }

        bool firstTermHandled = false;

        // Case 1: Array initializer like `int[2](...)`
        if (isType && first_term->isSubscript()) {
            yoi::vec<yoi::indexT> dimensions;
            yoi::indexT size = 1;
            auto dim_it = it;
            while (dim_it != end && (*dim_it)->isSubscript()) {
                yoi_assert((*dim_it)->expr->getToken().kind == lexer::token::tokenKind::integer, (*dim_it)->expr->getLine(), (*dim_it)->expr->getColumn(), "Array dimension must be an integer.");
                dimensions.push_back((*dim_it)->expr->getToken().basicVal.vInt);
                size = size * dimensions.back();
                dim_it++;
            }
            yoi::indexT actualSize = 0;
            if (dim_it != end && (*dim_it)->isInvocation()) {
                for (auto &val : (*dim_it)->args->get()) {
                    visit(val);
                    tryCastTo(baseType);
                    actualSize++;
                }
                it = ++dim_it;
            } else {
                it = dim_it;
            }
            
            yoi_assert(size == actualSize, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Array size (" + std::to_string(size) + ") does not match the initializer size (" + std::to_string(actualSize) + ").");
            moduleContext->getIRBuilder().newArrayOp(baseType, dimensions);
        } 
        // Case 2: Invocation `id<...>(...)` or `id(...)`
        else if (first_term->isInvocation()) {
            firstTermHandled = true;
            auto baseName = subscriptExpr->id->getId().get().strVal;
            auto args = first_term->args;
            bool resolved = false;

            // --- Step 1: Handle explicit template specialization if present ---
            if (subscriptExpr->id->hasTemplateArg()) {
                auto concreteTemplateArgs = parseTemplateArgs(subscriptExpr->id->getArg());
                if (irModule->functionTemplateTable.contains(baseName)) {
                    auto funcTemplate = irModule->functionTemplateTable[baseName];
                    auto astNode = irModule->funcTemplateAsts.at(baseName);
                    specializeFunctionTemplate(funcTemplate, astNode, concreteTemplateArgs, currentModuleIndex);
                } else {
                    try {
                        auto baseType = managedPtr(parseTypeSpec(subscriptExpr->id));
                        switch (baseType->type) {
                            case IRValueType::valueType::structObject: baseName = moduleContext->getCompilerContext()->getImportedModule(baseType->typeAffiliateModule)->structTable.getKey(baseType->typeIndex); break;
                            case IRValueType::valueType::interfaceObject: baseName = moduleContext->getCompilerContext()->getImportedModule(baseType->typeAffiliateModule)->interfaceTable.getKey(baseType->typeIndex); break;
                            default: panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Cannot specialize type: except structObject or interfaceObject but got: " + yoi::wstring2string(baseType->to_string())); break;
                        }
                    } catch (const std::runtime_error &) {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not resolve template specialization for function '" + wstring2string(baseName) + "'.");
                    }
                }
            }

            // --- Step 2: Unified Invocation Logic with correct precedence ---

            // Attempt 0: Check whether is a accessible variable
            moduleContext->getIRBuilder().saveState();
            {
                bool isAccessible = false;
                try {
                    visit(subscriptExpr->id, false);
                    isAccessible = true;
                } catch (const std::runtime_error &) {
                    moduleContext->getIRBuilder().restoreState();
                }
                if (isAccessible) {
                    auto structObject = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if(structObject->type == IRValueType::valueType::structObject || structObject->type == IRValueType::valueType::interfaceObject) {
                        handleInvocationExtern(L"operator()", args, currentModuleIndex, structObject);
                        resolved = true;
                    } else {
                        // TODO: handle interface objects
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "TODO: Interface objects are not implemented yet.");
                    }
                }
            }

            moduleContext->getIRBuilder().saveState();
            // Attempt 1: Struct Constructor (handles regular, variadic, and specialized template structs)
            if (irModule->structTable.contains(baseName)) {
                auto structIndex = irModule->structTable.getIndex(baseName);
                auto structType = irModule->structTable[structIndex];
                moduleContext->getIRBuilder().newStructOp(structIndex);
                auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                if (handleInvocationExtern(L"constructor", args, currentModuleIndex, rhs)) {
                    resolved = true;
                    moduleContext->getIRBuilder().discardState();
                } else {
                    moduleContext->getIRBuilder().restoreState();
                }
            } else {
                moduleContext->getIRBuilder().restoreState();
            }

            // Attempt 2: Free Function (handles regular, variadic, and implicit template functions)
            if (!resolved) {
                resolved = handleInvocationExtern(baseName, args, currentModuleIndex);
            }

            // Attempt 3: Interface Constructor
            if (!resolved) {
                if (irModule->interfaceTable.contains(baseName)) {
                    moduleContext->getIRBuilder().saveState();
                    try {
                        auto interfaceIndex = irModule->interfaceTable.getIndex(baseName);
                        auto argTypes = evaluateArguments(args);
                        yoi_assert(argTypes.size() == 1, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Interface constructor expects exactly one argument.");
                        moduleContext->getIRBuilder().newInterfaceOp(interfaceIndex);
                        auto interfaceImplName = getInterfaceImplName({currentModuleIndex, interfaceIndex}, argTypes[0]);
                        auto interfaceImplIndex = irModule->interfaceImplementationTable.getIndex(interfaceImplName);
                        moduleContext->getIRBuilder().constructInterfaceImplOp(interfaceImplIndex);
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::exception &) {
                        moduleContext->getIRBuilder().restoreState();
                    }
                }
            }

            // Attempt 4: Imported Function
            if (!resolved) {
                if (irModule->externTable.contains(baseName)) {
                    moduleContext->getIRBuilder().saveState();
                    try {
                        auto importedFunctionIndex = irModule->externTable.getIndex(baseName);
                        yoi_assert(irModule->externTable[importedFunctionIndex]->type == IRExternEntry::externType::importedFunction,
                                subscriptExpr->getLine(), subscriptExpr->getColumn(), "This is not an imported function.");
                        
                        auto argTypes = evaluateArguments(args);
                        auto importedFunc =
                            moduleContext->getCompilerContext()
                                ->getIRFFITable()
                                ->importedLibraries[irModule->externTable[importedFunctionIndex]->affiliateModule]
                                .importedFunctionTable[irModule->externTable[importedFunctionIndex]->itemIndex];
                        moduleContext->getIRBuilder().invokeImportedOp(
                            irModule->externTable[importedFunctionIndex]->affiliateModule,
                            irModule->externTable[importedFunctionIndex]->itemIndex,
                            argTypes.size(),
                            importedFunc->returnType);
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::exception &) {
                        moduleContext->getIRBuilder().restoreState();
                    }
                }
            }

            if (!resolved) {
                panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not resolve call to '" + wstring2string(baseName) + "'. No matching function, constructor, or template found for the given arguments.");
            }
        } 
        // Case 3: First term is a variable access for subsequent subscript `var[...]`
        else {
            visit(subscriptExpr->id, false); // Load the variable
        }

        if (firstTermHandled) {
            it++;
        }

        // --- Loop for subsequent terms (e.g., chained array access) ---
        while (it != end) {
            auto currentTerm = *it;
            bool isLastTerm = (std::next(it) == end);
            auto objectOnStackType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (currentTerm->isSubscript()) {
                if (objectOnStackType->isArrayType() || objectOnStackType->isDynamicArrayType()) {
                    const auto& dimensions = objectOnStackType->dimensions;
                    yoi::vec<yoi::indexT> strides(dimensions.size());
                    strides.back() = 1;
                    for (long long i = static_cast<long long>(dimensions.size()) - 2; i >= 0; --i) {
                        strides[i] = strides[i + 1] * dimensions[i + 1];
                    }

                    moduleContext->getIRBuilder().pushOp(IR::Opcode::push_integer, {IROperand::operandType::integer, IROperand::operandValue{static_cast<int64_t>(0)}});

                    yoi::indexT currentDim = 0;
                    while (it != end && (*it)->isSubscript()) {
                        yoi_assert(currentDim < dimensions.size(), (*it)->getLine(), (*it)->getColumn(), "Too many indices for array dimension.");
                        visit((*it)->expr);
                        yoi_assert(moduleContext->getIRBuilder().getRhsFromTempVarStack()->type == IRValueType::valueType::integerObject, (*it)->getLine(), (*it)->getColumn(), "Array subscript index must be an integer.");
                        moduleContext->getIRBuilder().pushOp(IR::Opcode::push_integer, {IROperand::operandType::integer, strides[currentDim]});
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                        it++;
                        currentDim++;
                    }

                    yoi_assert(currentDim == dimensions.size() || (isStoreOp && isLastTerm), currentTerm->getLine(), currentTerm->getColumn(), "Partial array access is not a loadable value. Not enough indices provided.");

                    if (isStoreOp && isLastTerm) {
                        moduleContext->getIRBuilder().storeOp(IR::Opcode::store_element, {});
                    } else {
                        auto elementType = managedPtr(objectOnStackType->getElementType());
                        moduleContext->getIRBuilder().loadOp(IR::Opcode::load_element, {}, elementType);
                    }
                    continue;
                } else {
                    if (isStoreOp) {
                        handleBinaryOperatorOverload(L"operator[]");
                    } else {
                        handleUnaryOperatorOverload(L"operator[]");
                    }
                }
            } else if (currentTerm->isInvocation()) {
                panic(currentTerm->getLine(), currentTerm->getColumn(), "TODO: Operator '()' overloading is not implemented yet.");
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
        auto &firstTerm = *it;

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);

        bool isType = false;
        std::shared_ptr<IRValueType> baseType;
        try {
            baseType = managedPtr(parseTypeSpecExtern(subscriptExpr->id, targetModule));
            isType = true;
        } catch (const std::exception &) {
            isType = false;
        }

        bool firstTermHandled = false;

        // Case 1: Extern Array Initializer
        if (isType && firstTerm->isSubscript()) {
            yoi::vec<yoi::indexT> dimensions;
            yoi::indexT size = 1;
            auto dim_it = it;
            while (dim_it != end && (*dim_it)->isSubscript()) {
                yoi_assert((*dim_it)->expr->getToken().kind == lexer::token::tokenKind::integer, (*dim_it)->expr->getLine(), (*dim_it)->expr->getColumn(), "Array dimension must be an integer.");
                dimensions.push_back((*dim_it)->expr->getToken().basicVal.vInt);
                size = size * dimensions.back();
                dim_it++;
            }
            yoi::indexT actualSize = 0;
            if (dim_it != end && (*dim_it)->isInvocation()) {
                for (auto &val : (*dim_it)->args->get()) {
                    visit(val);
                    tryCastTo(baseType);
                    actualSize++;
                }
                it = ++dim_it;
            } else {
                it = dim_it;
            }
            yoi_assert(size == actualSize, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Array size (" + std::to_string(size) + ") does not match the initializer size (" + std::to_string(actualSize) + ").");
            moduleContext->getIRBuilder().newArrayOp(baseType, dimensions);
        } 
        // Case 2: Extern Invocation
        else if (firstTerm->isInvocation()) {
            firstTermHandled = true;
            auto baseName = subscriptExpr->id->getId().get().strVal;
            auto args = firstTerm->args;
            bool resolved = false;

            // If the call has explicit template args, mangle the base name before resolution.
            yoi::wstr mangledBaseName = baseName;
            if (subscriptExpr->id->hasTemplateArg()) {
                auto concreteTemplateArgs = parseTemplateArgs(subscriptExpr->id->getArg());
                mangledBaseName = getMangledTemplateName(baseName, concreteTemplateArgs);
            }

            // --- Unified Invocation Logic for Extern Calls ---

            // Attempt 0: Check whether is a accessible variable
            moduleContext->getIRBuilder().saveState();
            {
                bool isAccessible = false;
                try {
                    visit(subscriptExpr->id, false);
                    isAccessible = true;
                } catch (const std::runtime_error &) {
                    moduleContext->getIRBuilder().restoreState();
                }
                if (isAccessible) {
                    auto structObject = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if(structObject->type == IRValueType::valueType::structObject || structObject->type == IRValueType::valueType::interfaceObject) {
                        auto structIndex = structObject->typeIndex;
                        auto structType = moduleContext->getCompilerContext()->getImportedModule(structObject->typeAffiliateModule)->structTable[structIndex];
                        
                        handleInvocationExtern(L"operator()", args, currentModuleIndex, structObject);
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } else {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Cannot call operator() on non-struct object or interface object.");
                    }
                }
            }

            // Attempt 1: Extern Struct Constructor (regular or variadic)
            if (targetedModule->structTable.contains(mangledBaseName)) {
                auto externStructEntry = getExternEntry(targetModule, mangledBaseName);
                auto targetedStruct = targetedModule->structTable[mangledBaseName];
                
                moduleContext->getIRBuilder().newStructOp(externStructEntry.itemIndex, true, externStructEntry.affiliateModule);
                auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                if (handleInvocationExtern(L"constructor", args, targetModule, rhs)) {
                    resolved = true;
                } else {
                    moduleContext->getIRBuilder().popFromTempVarStack(); // Pop unused extern struct
                }
            }

            // Attempt 2: Extern Free Function (regular or variadic)
            if (!resolved) {
                resolved = handleInvocationExtern(mangledBaseName, args, targetModule);
            }

            // Attempt 3: Extern Interface Constructor
            if (!resolved) {
                if (targetedModule->interfaceTable.contains(mangledBaseName)) {
                    moduleContext->getIRBuilder().saveState();
                    try {
                        auto argTypes = evaluateArguments(args);
                        yoi_assert(argTypes.size() == 1, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Interface constructor expects exactly one argument.");

                        auto externInterface = getExternEntry(targetModule, mangledBaseName);
                        moduleContext->getIRBuilder().newInterfaceOp(externInterface.itemIndex, true, externInterface.affiliateModule);

                        auto interfaceImplName = getInterfaceImplName({externInterface.affiliateModule, externInterface.itemIndex}, argTypes[0]);
                        auto interfaceImplIndex = irModule->interfaceImplementationTable.getIndex(interfaceImplName);
                        moduleContext->getIRBuilder().constructInterfaceImplOp(interfaceImplIndex);
                        
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::exception &) {
                        moduleContext->getIRBuilder().restoreState();
                    }
                }
            }

            // Attempt 4: FFI Imported Function (via an extern module)
            if (!resolved) {
                if (targetedModule->externTable.contains(mangledBaseName)) {
                    moduleContext->getIRBuilder().saveState();
                    try {
                        auto argTypes = evaluateArguments(args);
                        auto externEntry = targetedModule->externTable[mangledBaseName];
                        auto externFunc = moduleContext->getCompilerContext()->getIRFFITable()->importedLibraries[externEntry->affiliateModule].importedFunctionTable[externEntry->itemIndex];
                        moduleContext->getIRBuilder().invokeImportedOp(externEntry->affiliateModule, externEntry->itemIndex, argTypes.size(), externFunc->returnType);
                        
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::exception &) {
                        moduleContext->getIRBuilder().restoreState();
                    }
                }
            }

            if (!resolved) {
                panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not find extern function, struct constructor, or interface in module: " + wstring2string(mangledBaseName));
            }
        } else { // First term is a variable access
            visitExtern(subscriptExpr->id, targetModule, false);
        }

        if (firstTermHandled) {
            it++;
        }

        // --- Loop for subsequent terms ---
        while (it != end) {
            auto current_term = *it;
            auto object_on_stack_type = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (current_term->isSubscript()) {
                panic(current_term->getLine(),
                    current_term->getColumn(),
                    "TODO: Extern array access is not fully implemented yet.");
            } else if (current_term->isInvocation()) {
                panic(current_term->getLine(),
                    current_term->getColumn(),
                    "TODO: Extern operator '()' overloading is not implemented yet.");
            }
            it++;
        }

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }


    yoi::indexT visitor::visit(yoi::identifierWithTemplateArg *identifierWithTemplateArg, bool isStoreOp) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            panic(identifierWithTemplateArg->getLine(), identifierWithTemplateArg->getColumn(), "Invalid visit of identifier with template arg.");
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
            return IRValueType{
                IRValueType::valueType::structObject, static_cast<yoi::indexT>(currentModuleIndex), typeIndex};
        } catch (std::out_of_range &e) {
            // let it go
        }
        try {
            auto typeIndex = irModule->interfaceTable.getIndex(typeName);
            return IRValueType{
                IRValueType::valueType::interfaceObject, static_cast<yoi::indexT>(currentModuleIndex), typeIndex};
        } catch (std::out_of_range &e) {
            // let it go
        }
        try {
            auto incompleteType = getIncompleteType(typeName);
            return *incompleteType;
        } catch (std::out_of_range &e) {
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
            bool isVaridic = false;

            IRFunctionTemplate::Builder templateBuilder;
            IRFunctionDefinition::Builder builder;

            builder.setDebugInfo({irModule->modulePath, funcDefStmt->getLine(), funcDefStmt->getColumn()});

            templateBuilder.templateArguments = getTemplateArgs(funcName.getArg());

            moduleContext->pushTemplateBuilder(templateBuilder);

            
            std::vector<std::shared_ptr<IRValueType>> argTypes;
            for (auto &i : funcDefStmt->getArgs().get()) {
                if (&i == &funcDefStmt->getArgs().get().back() && i->spec->kind == 3 /* elipsis */) {
                    isVaridic = true;
                    builder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                    auto argName = i->getId().node.strVal;
                    auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                    builder.addArgument(argName, argType);
                    argTypes.push_back(argType);
                    break;
                }
                auto argName = i->getId().node.strVal;
                auto argType = managedPtr(parseTypeSpec(i->spec));
                argTypes.push_back(argType);
                builder.addArgument(argName, argType);
            }

            auto funcType = parseTypeSpec(&funcDefStmt->getResultType());

            builder.setReturnType(managedPtr(funcType));

            builder.attrs = getFunctionAttributes(funcDefStmt->attrs);

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
            bool isVaridic = false;

            auto funcType = parseTypeSpec(&funcDefStmt->getResultType());
            IRFunctionDefinition::Builder builder;

            builder.setDebugInfo({irModule->modulePath, funcDefStmt->getLine(), funcDefStmt->getColumn()});

            builder.attrs = getFunctionAttributes(funcDefStmt->attrs);

            builder.setReturnType(managedPtr(funcType));
            std::vector<std::shared_ptr<IRValueType>> argTypes;
            for (auto &i : funcDefStmt->getArgs().get()) {
                if (&i == &funcDefStmt->getArgs().get().back() && i->spec->kind == 3 /* elipsis */) {
                    isVaridic = true;
                    builder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                    auto argName = i->getId().node.strVal;
                    auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                    builder.addArgument(argName, argType);
                    argTypes.push_back(argType);
                    break;
                }

                auto argName = i->getId().node.strVal;
                auto argType = managedPtr(parseTypeSpec(i->spec));
                argTypes.push_back(argType);
                builder.addArgument(argName, argType);
            }

            builder.setName(funcName.getId().node.strVal + getFuncUniqueNameStr(argTypes));

            auto func = builder.yield();

            irModule->functionTable.put(builder.name, func);

            moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
            moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, funcDefStmt->getLine(), funcDefStmt->getColumn()});
            moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
            visit(funcDefStmt->block, true);
            moduleContext->getIRBuilder().yield();
            moduleContext->popIRBuilder();
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::interfaceDefStmt *interfaceDefStmt) {
        if (interfaceDefStmt->id->hasDefTemplateArg()) {
            auto &interfaceName = interfaceDefStmt->id->getId().get().strVal;

            IRInterfaceInstanceTemplate::Builder templateBuilder;
            templateBuilder.templateArguments = getTemplateArgs(interfaceDefStmt->id->getArg());

            moduleContext->pushTemplateBuilder(templateBuilder);

            IRInterfaceInstanceDefinition::Builder builder;
            builder.setName(interfaceName);
            for (auto &i : interfaceDefStmt->getInner().getInner()) {
                bool isVaridic = false;
                yoi_assert(i->isMethod(), i->getLine(), i->getColumn(), "Interface member must be a method");

                auto methodName = i->getMethod().getName().get().strVal;
                auto methodResultType = managedPtr(parseTypeSpec(i->getMethod().resultType));
                yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                IRFunctionDefinition::Builder methodBuilder;

                methodBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});

                methodBuilder.setReturnType(methodResultType);
                for (auto &arg : i->getMethod().getArgs().get()) {
                    if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                        isVaridic = true;
                        methodBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                        auto argName = arg->getId().node.strVal;
                        auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                        methodBuilder.addArgument(argName, argType);
                        argTypes.push_back(argType);
                        break;
                    }

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
            
            auto templateDef = builder.yield();
            templateBuilder.setTemplateDefinition(templateDef);
            irModule->interfaceInstanceTemplateTable.put_create(interfaceName, templateBuilder.yield());
            irModule->templateInterfaceAsts[interfaceName] = interfaceDefStmt;

            moduleContext->popTemplateBuilder();

            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

        auto &interfaceName = interfaceDefStmt->id->getId().get().strVal;

        IRInterfaceInstanceDefinition::Builder builder;
        builder.setName(interfaceName);
        for (auto &i : interfaceDefStmt->getInner().getInner()) {
            bool isVaridic = false;
            yoi_assert(i->isMethod(), i->getLine(), i->getColumn(), "Interface member must be a method");

            auto methodName = i->getMethod().getName().get().strVal;
            auto methodResultType = managedPtr(parseTypeSpec(i->getMethod().resultType));
            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
            IRFunctionDefinition::Builder methodBuilder;

            methodBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});

            methodBuilder.setReturnType(methodResultType);
            for (auto &arg : i->getMethod().getArgs().get()) {
                if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                    isVaridic = true;
                    methodBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                    auto argName = arg->getId().node.strVal;
                    auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                    methodBuilder.addArgument(argName, argType);
                    argTypes.push_back(argType);
                    break;
                }

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
            for (auto &field : structDefStmt->getInner().getInner()) {
                if (field->kind == 0) { // Var
                    auto memberName = field->getVar().getId().get().strVal;
                    auto memberType = managedPtr(parseTypeSpec(field->getVar().spec));
                    builder.addField(memberName, memberType);
                } else if (field->kind == 1) { // Constructor
                    bool isVaridic = false;
                    IRFunctionTemplate::Builder constructorBuilder;
                    IRFunctionDefinition::Builder constructorDefBuilder;

                    constructorDefBuilder.setDebugInfo({irModule->modulePath, field->getLine(), field->getColumn()});

                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                    moduleContext->pushTemplateBuilder(constructorBuilder);

                    constructorBuilder.templateArguments = templateBuilder.templateArguments;
                    auto STRUCT_INCOMPLETE_TYPE =
                        constructorBuilder.templateArguments.put_create(L"STRUCT", {{}, {-1, -1}});
                    constructorBuilder.templateArguments[STRUCT_INCOMPLETE_TYPE].templateType = managedPtr(IRValueType{
                        IRValueType::valueType::incompleteTemplateType, currentModuleIndex, STRUCT_INCOMPLETE_TYPE});

                    constructorDefBuilder.setReturnType(
                        constructorBuilder.templateArguments[STRUCT_INCOMPLETE_TYPE].templateType);

                    for (auto &arg : field->getConstructor().getArgs().get()) {
                        if (&arg == &field->getConstructor().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                            isVaridic = true;
                            constructorDefBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                            auto argName = arg->getId().node.strVal;
                            auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                            constructorDefBuilder.addArgument(argName, argType);
                            argTypes.push_back(argType);
                            break;
                        }
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
                    bool isVaridic = false;
                    auto methodName = field->getMethod().getName().get().strVal;
                    IRFunctionTemplate::Builder methodBuilder;
                    IRFunctionDefinition::Builder methodDefBuilder;

                    methodDefBuilder.setDebugInfo({irModule->modulePath, field->getLine(), field->getColumn()});

                    moduleContext->pushTemplateBuilder(methodBuilder);

                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                    methodBuilder.templateArguments = templateBuilder.templateArguments;
                    auto STRUCT_INCOMPLETE_TYPE = methodBuilder.templateArguments.put_create(L"STRUCT", {{}, {-1, -1}});
                    methodBuilder.templateArguments[STRUCT_INCOMPLETE_TYPE].templateType = managedPtr(IRValueType{
                        IRValueType::valueType::incompleteTemplateType, currentModuleIndex, STRUCT_INCOMPLETE_TYPE});

                    auto methodType = managedPtr(parseTypeSpec(field->getMethod().resultType));

                    methodDefBuilder.setReturnType(methodType);

                    for (auto &arg : field->getMethod().getArgs().get()) {
                        if (&arg == &field->getConstructor().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                            isVaridic = true;
                            methodDefBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                            auto argName = arg->getId().node.strVal;
                            auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                            methodDefBuilder.addArgument(argName, argType);
                            argTypes.push_back(argType);
                            break;
                        }
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

            generateNullInterfaceImplementation(managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex}));

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
                        bool isVaridic = false;
                        IRFunctionDefinition::Builder constructorBuilder;

                        constructorBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});

                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                        auto thisType = managedPtr(
                            IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                        constructorBuilder.setReturnType(thisType);
                        constructorBuilder.addArgument(L"this", thisType);

                        for (auto &arg : i->getConstructor().getArgs().get()) {
                            if (&arg == &i->getConstructor().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                                isVaridic = true;
                                constructorBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                                auto argName = arg->getId().node.strVal;
                                auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                                constructorBuilder.addArgument(argName, argType);
                                argTypes.push_back(argType);
                                break;
                            }
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
                        bool isVaridic = false;
                        auto methodName = i->getMethod().getName().get().strVal;
                        auto methodType = managedPtr(parseTypeSpec(i->getMethod().resultType));
                        IRFunctionDefinition::Builder methodBuilder;

                        methodBuilder.attrs = getFunctionAttributes(i->getMethod().attrs);
                        methodBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});

                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                        methodBuilder.setReturnType(methodType);
                        auto thisType = managedPtr(
                            IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                        
                        if (std::find(methodBuilder.attrs.begin(), methodBuilder.attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) == methodBuilder.attrs.end())
                            methodBuilder.addArgument(L"this", thisType);

                        for (auto &arg : i->getMethod().getArgs().get()) {
                            if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                                isVaridic = true;
                                methodBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                                auto argName = arg->getId().node.strVal;
                                auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                                methodBuilder.addArgument(argName, argType);
                                argTypes.push_back(argType);
                                break;
                            }
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
        auto &structIdNode = implStmt->getStructId();
        auto structBaseName = structIdNode.getId().get().strVal;

        if (structIdNode.hasTemplateArg()) {
            // Impl for a template struct
            yoi_assert(irModule->structTemplateTable.contains(structBaseName),
                       implStmt->getLine(),
                       implStmt->getColumn(),
                       "Impl for undefined struct template: " + wstring2string(structBaseName));

            yoi::vec<std::shared_ptr<IRValueType>> concreteTemplateArgs = parseTemplateArgs(structIdNode.getArg());

            if (implStmt->isImplForStmt()) {
                // interface implementation for a template struct
                if (concreteTemplateArgs.empty()) {
                    irModule->templateInterfaceImplAsts[structBaseName].push_back(implStmt);
                } else {
                    // specialize template interface
                    auto interfaceTemplateAst = irModule->templateInterfaceAsts[structBaseName];
                    auto concreteStructName = getMangledTemplateName(structBaseName, concreteTemplateArgs);
                    auto concreteStructType = managedPtr(parseTypeSpec(implStmt->structName)); // quick specialization check
                    yoi_assert(concreteStructType->type == IRValueType::valueType::structObject, implStmt->getLine(), implStmt->getColumn(), "Invalid struct name for struct specialization: " + wstring2string(concreteStructName) + " (except structObject but got " + wstring2string(concreteStructType->to_string()) + ")");                    
                    specializeInterfaceImplementation(implStmt, concreteStructType, concreteStructName, concreteTemplateArgs);
                }
            } else {
                if (concreteTemplateArgs.empty()) {
                    // pure template struct, store them and specialize when used
                    irModule->templateImplAsts[structBaseName] = implStmt;
                } else {
                    // specialize template struct
                    auto structTemplateAst = irModule->structTemplateAsts[structBaseName];
                    specializeStructTemplate(structBaseName, concreteTemplateArgs, implStmt, currentModuleIndex);
                }
            }

            
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

        if (implStmt->isImplForStmt()) {
            auto interfaceName = parseInterfaceName(implStmt->interfaceName);
            std::shared_ptr<IRValueType> srcType;
            try {
                srcType = managedPtr(parseTypeSpec(implStmt->structName));
            } catch (std::runtime_error &e) {
                panic(
                    implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(structBaseName));
                return {};
            }
            moduleContext->getCompilerContext()->getImportedModule(interfaceName.first.first)->interfaceTable[interfaceName.first.second]->implementations.emplace_back(
                srcType->type, srcType->typeAffiliateModule, srcType->typeIndex
            );
            auto interfaceImplName =
                getInterfaceImplName(interfaceName.first, srcType);

            auto interfaceImplIndex = irModule->interfaceImplementationTable.put(interfaceImplName, {});
            IRInterfaceImplementationDefinition::Builder builder;
            builder.setName(interfaceImplName);
            builder.setImplStructIndex({srcType->type, srcType->typeAffiliateModule, srcType->typeIndex});
            builder.setImplInterfaceIndex(interfaceName.first.second);

            for (auto &i : implStmt->getInner().getInner()) {
                yoi_assert(!i->isConstructor(),
                           i->getLine(),
                           i->getColumn(),
                           "Constructor cannot be implemented for interface");

                bool isVaridic = false;

                auto methodName = i->getMethod().getName().get().strVal;
                IRFunctionDefinition::Builder methodBuilder;

                methodBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
                methodBuilder.attrs = getFunctionAttributes(i->getMethod().attrs);

                yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                const auto& thisType = srcType;
                if (std::find(methodBuilder.attrs.begin(), methodBuilder.attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) == methodBuilder.attrs.end())
                    methodBuilder.addArgument(L"this", thisType);

                for (auto &arg : i->getMethod().getArgs().get()) {
                    if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                        isVaridic = true;
                        methodBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                        auto argName = arg->getId().node.strVal;
                        auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                        methodBuilder.addArgument(argName, argType);
                        argTypes.push_back(argType);
                        break;
                    }
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
                builder.addVirtualMethod(
                    methodName + uniq,
                    managedPtr(IRValueType{IRValueType::valueType::virtualMethod, currentModuleIndex, funcIndex}));

                moduleContext->pushIRBuilder(IRBuilder{moduleContext->getCompilerContext(), irModule, func});
                moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
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
            } catch (std::runtime_error &e) {
                panic(
                    implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(structBaseName));
            }

            for (auto &i : implStmt->getInner().getInner()) {
                yoi::wstr mangledName;
                if (i->isConstructor()) {
                    bool isVaridic = false;
                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                    for (auto &arg : i->getConstructor().getArgs().get()) {
                        if (&arg == &i->getConstructor().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                            isVaridic = true;
                            auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                            argTypes.push_back(argType);
                            break;
                        }
                        argTypes.push_back(managedPtr(parseTypeSpec(arg->spec)));
                    }
                    mangledName = structBaseName + L"::constructor" + getFuncUniqueNameStr(argTypes);
                } else {
                    // whole bunch of shit doin' here is to get the mangled name of the method, no actual modification of original func def here.
                    bool isVaridic = false;
                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                    for (auto &arg : i->getMethod().getArgs().get()) {
                        if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                            isVaridic = true;
                            auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                            argTypes.push_back(argType);
                            break;
                        }
                        argTypes.push_back(managedPtr(parseTypeSpec(arg->spec)));
                    }
                    mangledName =
                        structBaseName + L"::" + i->getMethod().getName().get().strVal + getFuncUniqueNameStr(argTypes);
                }

                try {
                    auto funcIndex = irModule->functionTable.getIndex(mangledName);
                    auto func = irModule->functionTable[funcIndex];
                    moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
                    moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
                    moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                    visit(i->isConstructor() ? i->getConstructor().block : i->getMethod().block, true);
                    moduleContext->getIRBuilder().yield();
                    moduleContext->popIRBuilder();
                } catch (std::out_of_range &e) {
                    panic(i->getLine(),
                          i->getColumn(),
                          "No matched constructor or method declaration found for impl: " +
                              wstring2string(mangledName));
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
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global,
                                                      {IROperand::operandType::globalVar, index});

            } else {
                auto index =
                    moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().put(i->lhs->node.strVal, type);
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local,
                                                      {IROperand::operandType::localVar, index});
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
        yoi_assert(condType->type == IRValueType::valueType::booleanObject,
                   ifStmt->getLine(),
                   ifStmt->getColumn(),
                   "The type in if-condition must be boolean");

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
            yoi_assert(elifCondType->type == IRValueType::valueType::booleanObject,
                       i.cond->getLine(),
                       i.cond->getColumn(),
                       "The type in elif-condition must be boolean");

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
        } else {
            moduleContext->getIRBuilder().jumpOp(outBlock);
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
        yoi_assert(condType->type == IRValueType::valueType::booleanObject,
                   whileStmt->getLine(),
                   whileStmt->getColumn(),
                   "The type in while-condition must be boolean");

        auto whileBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto outBlock = moduleContext->getIRBuilder().createCodeBlock();

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, whileBlock);
        moduleContext->getIRBuilder().jumpOp(outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(whileBlock);
        visit(whileStmt->block, true);

        // replace dummy_break and dummy_continue with jump to the cond block
        for (auto &i : moduleContext->getIRBuilder().getCurrentCodeBlock().getIRArray()) {
            if (i.opcode == IR::Opcode::dummy_break) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, outBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()};
            } else if (i.opcode == IR::Opcode::dummy_continue) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, condBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()};
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
        yoi_assert(condType->type == IRValueType::valueType::booleanObject,
                   forStmt->getLine(),
                   forStmt->getColumn(),
                   "The type in for-condition must be boolean");

        moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_true, codeBlock);
        moduleContext->getIRBuilder().jumpOp(outBlock);
        moduleContext->getIRBuilder().switchCodeBlock(codeBlock);
        visit(forStmt->block, true);

        // replace dummy_break and dummy_continue with jump to the cond block
        for (auto &i : moduleContext->getIRBuilder().getCurrentCodeBlock().getIRArray()) {
            if (i.opcode == IR::Opcode::dummy_break) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, outBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()};
            } else if (i.opcode == IR::Opcode::dummy_continue) {
                i = {IR::Opcode::jump, {{IROperand::operandType::index, codeBlock}}, moduleContext->getIRBuilder().getCurrentDebugInfo()};
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
            tryCastTo(returnType);
            moduleContext->getIRBuilder().retOp();
        } else {
            // TODO: return void
            moduleContext->getIRBuilder().retOp(true);
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::continueStmt *continueStmt) {
        moduleContext->getIRBuilder().insert({IR::Opcode::dummy_continue, {}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::breakStmt *breakStmt) {
        moduleContext->getIRBuilder().insert({IR::Opcode::dummy_break, {}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    IRValueType visitor::parseTypeSpec(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            auto baseName = identifierWithTemplateArg->getId().node.strVal;
            auto concreteTypes = parseTemplateArgs(identifierWithTemplateArg->getArg());

            if (irModule->structTemplateTable.contains(baseName)) {
                try {
                    auto pureTemplateAst = irModule->templateImplAsts.at(baseName);
                    auto specializedIndex = specializeStructTemplate(baseName, concreteTypes, pureTemplateAst, currentModuleIndex);
                    return {IRValueType::valueType::structObject, currentModuleIndex, specializedIndex};
                } catch (std::out_of_range &e) {
                    panic(identifierWithTemplateArg->getLine(),
                        identifierWithTemplateArg->getColumn(),
                        "No implementation block found for struct template: " + wstring2string(baseName));
                }
            } else if (irModule->interfaceInstanceTemplateTable.contains(baseName)) {
                auto specializedIndex = specializeInterfaceTemplate(baseName, concreteTypes);
                return {IRValueType::valueType::interfaceObject, currentModuleIndex, specializedIndex};
            } else {
                panic(identifierWithTemplateArg->getLine(),
                    identifierWithTemplateArg->getColumn(),
                    "Unknown template: " + wstring2string(baseName));
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
            for (auto &sub : subscriptExpr->getSubscript()) {
                yoi_assert(
                    sub->isSubscript() && sub->expr->getToken().kind == lexer::token::tokenKind::integer, sub->getLine(), sub->getColumn(), "Expected dimension size for array type.");
                dimensions.push_back(sub->expr->getToken().basicVal.vInt);
            }
            return baseType.getArrayType(dimensions);
        } else {
            return parseTypeSpec(subscriptExpr->id);
        }
    }

    IRValueType visitor::parseTypeSpecExtern(yoi::identifier *identifier, yoi::indexT targetModule) {
        auto mod = moduleContext->getCompilerContext()->getImportedModule(targetModule);
        auto ex = getExternEntry(targetModule, identifier->node.strVal);
        if (ex.type == IRExternEntry::externType::structType)
            return {IRValueType::valueType::structObject, ex.affiliateModule, ex.itemIndex};
        else if (ex.type == IRExternEntry::externType::interfaceType)
            return {IRValueType::valueType::interfaceObject, ex.affiliateModule, ex.itemIndex};
        else
            panic(identifier->getLine(), identifier->getColumn(), "Unsupported extern type: " + wstring2string(identifier->node.strVal));
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
            panic(subscriptExpr->getLine(),
                  subscriptExpr->getColumn(),
                  "TODO: Extern array type parsing not fully implemented.");
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
                yoi::indexT targetModule = -1, lastModule = -1;
                while (it + 1 != typeSpec->member->getTerms().end() &&
                       (targetModule = isModuleName(*it, targetModule)) != lastModule) {
                    it++;
                    lastModule = targetModule;
                }

                IRValueType lhs{IRValueType::valueType::integerObject};
                if (targetModule == -1) {
                    lhs = parseTypeSpec(*it);
                } else {
                    lhs = parseTypeSpecExtern(*it, targetModule);
                }
                yoi_assert(it + 1 == typeSpec->member->getTerms().end(),
                           typeSpec->getLine(),
                           typeSpec->getColumn(),
                           "Type specifier is not valid.");
                
                return typeSpec->hasArrayTypeSpec ? lhs.getDynamicArrayType() : lhs;
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
            case 3: {
                // elipsis
                // let it fallback to invalid
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
                                            const std::shared_ptr<IRValueType> &typeSrc) {
        return L"interfaceImpl#" + std::to_wstring(interfaceSrc.first) + L"#" + std::to_wstring(interfaceSrc.second) +
               L"#" + typeSrc->to_string();
    }

    std::pair<std::pair<yoi::indexT, yoi::indexT>, std::shared_ptr<IRInterfaceInstanceDefinition>>
    visitor::parseInterfaceName(yoi::externModuleAccessExpression *structDef) {
        // modules~
        auto it = structDef->getTerms().begin();
        yoi::indexT targetModule = -1, lastModule = -1;
        while (it + 1 != structDef->getTerms().end() && (targetModule = isModuleName(*it, targetModule)) != lastModule) {
            it++;
            lastModule = targetModule;
        }
        if (targetModule == -1) {
            targetModule = currentModuleIndex;
        }
        yoi_assert(it + 1 == structDef->getTerms().end(),
                   structDef->getLine(),
                   structDef->getColumn(),
                   "Invalid interface name");
        auto interfaceName = parseIdentifierWithTemplateArg(*it);
        try {
            auto target = moduleContext->getCompilerContext()->getImportedModule(targetModule);
            auto interfaceIndex = target->interfaceTable.getIndex(interfaceName);
            return std::make_pair(std::make_pair(targetModule, interfaceIndex), target->interfaceTable[interfaceIndex]);
        } catch (std::out_of_range &) {
            panic(
                structDef->getLine(), structDef->getColumn(), "Undefined interface: " + wstring2string(interfaceName));
        }
    }

    yoi::indexT visitor::isModuleName(identifierWithTemplateArg *it, yoi::indexT currentModule) const {
        if (!it->hasTemplateArg()) {
            return isModuleName(it->id, currentModule);
        } else {
            return currentModule;
        }
    }

    yoi::IRExternEntry visitor::getExternEntry(yoi::indexT moduleIndex, const yoi::wstr &identifier) const {
        try {
            auto res = moduleContext->getCompilerContext()
                           ->getImportedModule(moduleIndex)
                           ->globalVariables.getIndex(identifier);
            return {IRExternEntry::externType::globalVar, identifier, moduleIndex, res};
        } catch (std::out_of_range &) {
        }
        try {
            auto res =
                moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->functionTable.getIndex(identifier);
            return {IRExternEntry::externType::function, identifier, moduleIndex, res};
        } catch (std::out_of_range &) {
        }
        try {
            auto res =
                moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->structTable.getIndex(identifier);
            return {IRExternEntry::externType::structType, identifier, moduleIndex, res};
        } catch (std::out_of_range &) {
        }
        try {
            auto res = moduleContext->getCompilerContext()
                           ->getImportedModule(moduleIndex)
                           ->interfaceTable.getIndex(identifier);
            return {IRExternEntry::externType::interfaceType, identifier, moduleIndex, res};
        } catch (std::out_of_range &) {
        }
        try {
            auto res = moduleContext->getCompilerContext()
                           ->getImportedModule(moduleIndex)
                           ->interfaceImplementationTable.getIndex(identifier);
            return {IRExternEntry::externType::interfaceImplType, identifier, moduleIndex, res};
        } catch (std::out_of_range &) {
        }

        throw std::out_of_range("undefined identifier: not known global variable, function, struct or interface type: " + yoi::wstring2string(identifier));
    }

    yoi::indexT visitor::addExternEntryIfNotExists(yoi::indexT moduleIndex, const yoi::wstr &identifier) {
        // extern entry format: moduleIndex#identifier
        yoi::wstr key = std::to_wstring(moduleIndex) + L"#" + identifier;
        try {
            auto it = irModule->externTable.getIndex(key);
            return it;
        } catch (std::runtime_error &) {
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
        } else if (!lhsType->is1ByteType() && rhsType->is1ByteType()) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        }
        // if int with deci, upcast to deci
        else if (lhsType->type == IRValueType::valueType::integerObject &&
                 rhsType->type == IRValueType::valueType::decimalObject) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        } else if (lhsType->type == IRValueType::valueType::decimalObject &&
                   rhsType->type == IRValueType::valueType::integerObject) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        }
        // if left or right is pointer, cast the other to pointer
        else if (lhsType->type == IRValueType::valueType::pointerObject) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        } else if (rhsType->type == IRValueType::valueType::pointerObject) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
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
                res = L"virtual_method#" + std::to_wstring(type->typeAffiliateModule) + L"#" +
                      std::to_wstring(type->typeIndex);
                break;
            case IRValueType::valueType::incompleteTemplateType:
                res = L"incomplete_template_type#" + std::to_wstring(type->typeIndex);
                break;
            case IRValueType::valueType::interfaceObject:
                res = L"interfaceObject#" + std::to_wstring(type->typeAffiliateModule) + L"#" + std::to_wstring(type->typeIndex);
                break;
            default:
                panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Invalid type");
                break;
        }
        if (type->isArrayType()) {
            yoi::indexT arraySize = 1;
            for (auto &dim : type->dimensions) {
                arraySize *= dim;
            }
            res += L"[" + std::to_wstring(arraySize) + L"]";
        } else if (type->isDynamicArrayType()) {
            res += L"[]";
        }
        return res;
    }

    yoi::wstr visitor::getFuncUniqueNameStr(const std::vector<std::shared_ptr<IRValueType>> &argumentTypes,
                                            bool whetherIgnoreFirstParam) {
        yoi::wstr res = L"#";
        bool first = false;
        for (auto &arg : argumentTypes) {
            if (whetherIgnoreFirstParam && !first || !whetherIgnoreFirstParam)
                res += getTypeSpecUniqueNameStr(arg) + L"#";
            else
                first = false;
        }
        if (!argumentTypes.empty()) {
            res.pop_back();
        }
        res.shrink_to_fit();
        return res;
    }

    yoi::indexT visitor::visitExtern(yoi::identifier *identifier, yoi::indexT targetModule, bool isStoreOp) {
        try {
            yoi::IRExternEntry entry = getExternEntry(targetModule, identifier->get().strVal);
            yoi_assert(entry.type == IRExternEntry::externType::globalVar,
                       identifier->getLine(),
                       identifier->getColumn(),
                       "Invalid type specifier, expected global variable");
            auto valType = moduleContext->getCompilerContext()
                               ->getImportedModule(targetModule)
                               ->globalVariables[identifier->node.strVal];
            if (isStoreOp) {
                tryCastTo(valType);
                moduleContext->getIRBuilder().storeOp(
                    IR::Opcode::store_global, {IROperand::operandType::externVar, entry.itemIndex}, entry.affiliateModule);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            } else {
                moduleContext->getIRBuilder().loadOp(
                    IR::Opcode::load_global, {IROperand::operandType::externVar, entry.itemIndex}, valType, entry.affiliateModule);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            }
        } catch (std::runtime_error &) {
            // not found, panic
            panic(identifier->getLine(),
                  identifier->getColumn(),
                  "undefined identifier: " + wstring2string(identifier->node.strVal));
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }
    }

    yoi::indexT visitor::visitExtern(yoi::identifierWithTemplateArg *identifierWithTemplateArg,
                                     yoi::indexT targetModule,
                                     bool isStoreOp) {
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
            res[index].templateType =
                managedPtr(IRValueType{IRValueType::valueType::incompleteTemplateType, currentModuleIndex, index});

            if (arg->impl) {
                auto constraint = parseInterfaceName(arg->impl);
                res[index].interfaceType = constraint.first;
            }
        }
        return res;
    }

    yoi::vec<std::shared_ptr<IRValueType>> visitor::parseTemplateArgs(const yoi::templateArg &templateArgs) {
        yoi::vec<std::shared_ptr<IRValueType>> res;
        for (auto &arg : templateArgs.spec) {
            res.push_back(managedPtr(parseTypeSpec(arg->spec)));
        }
        return res;
    }

    yoi::indexT
    visitor::specializeFunctionTemplate(const std::shared_ptr<IRFunctionTemplate> &templateFunc,
                                        yoi::funcDefStmt *astNode,
                                        const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                        yoi::indexT moduleIndex) {
        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);

        yoi::wstr specializedName =
            getMangledTemplateName(templateFunc->templateDefinition->name, concreteTemplateArgs);

        if (targetedModule->functionTable.contains(specializedName)) {
            return targetedModule->functionTable.getIndex(specializedName);
        }

        // Create a new function definition by specializing the template
        IRFunctionDefinition::Builder builder;

        builder.setDebugInfo({targetedModule->modulePath, astNode->getLine(), astNode->getColumn()});
        builder.setName(specializedName);

        // Create specialization context
        IRTemplateBuilder specializationContext;
        for (yoi::indexT i = 0; i < templateFunc->templateArguments.size(); ++i) {
            auto paramName = templateFunc->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }

        
        pushModuleContext(moduleIndex);
        moduleContext->pushTemplateBuilder(specializationContext);

        // Specialize arguments and return type
        for (const auto &argPair : astNode->getArgs().get()) {
            auto argName = argPair->getId().get().strVal;
            auto argType = managedPtr(parseTypeSpec(&argPair->getSpec()));
            builder.addArgument(argName, argType);
        }
        builder.setReturnType(managedPtr(parseTypeSpec(&astNode->getResultType())));

        auto specializedFunc = builder.yield();
        auto funcIndex = targetedModule->functionTable.put_create(specializedName, specializedFunc);

        // Visit the body to generate IR
        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), targetedModule, specializedFunc});
        moduleContext->getIRBuilder().setDebugInfo({targetedModule->modulePath, astNode->getLine(), astNode->getColumn()});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(&astNode->getBlock(), true);
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();

        // Pop context
        moduleContext->popTemplateBuilder();
        popModuleContext();

        return funcIndex;
    }

    yoi::wstr visitor::getMangledTemplateName(const yoi::wstr &baseName,
                                              const yoi::vec<std::shared_ptr<IRValueType>> &templateArgs) {
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

    yoi::indexT visitor::specializeStructTemplate(const yoi::wstr &templateName,
                                              const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                              yoi::implStmt *pureTemplateImplAst, yoi::indexT moduleIndex) {

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);

        yoi::wstr specializedName = getMangledTemplateName(templateName, concreteTemplateArgs);

        if (targetedModule->structTable.contains(specializedName)) {
            return targetedModule->structTable.getIndex(specializedName);
        }
        auto structTemplate = targetedModule->structTemplateTable[templateName];
        auto structAst = targetedModule->structTemplateAsts.at(templateName);

        IRTemplateBuilder specializationContext;
        yoi_assert(concreteTemplateArgs.size() == structTemplate->templateArguments.size(),
                0, 0, "Template argument count mismatch for struct " + wstring2string(templateName));
        for (yoi::indexT i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structTemplate->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }

        auto specializedStructIndex = targetedModule->structTable.put_create(specializedName, nullptr);

        generateNullInterfaceImplementation(managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, specializedStructIndex}));
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, specializedStructIndex});
        specializationContext.addTemplateArgument(L"STRUCT", selfType);

        pushModuleContext(moduleIndex);
        moduleContext->pushTemplateBuilder(specializationContext);

        IRStructDefinition::Builder builder;
        builder.setName(specializedName);
        for (auto &field : structAst->getInner().getInner()) {
            if (field->kind == 0) {
                auto memberName = field->getVar().getId().get().strVal;
                auto memberType = managedPtr(parseTypeSpec(field->getVar().spec));
                builder.addField(memberName, memberType);
            }
        }
        auto specializedStruct = builder.yield();
        targetedModule->structTable[specializedStructIndex] = specializedStruct;

        // Specialize methods defined in `impl MyStruct<T> { ... }`
        if (pureTemplateImplAst) {
            for (auto &methodAst : pureTemplateImplAst->getInner().getInner()) {
                specializeStructMethod(structTemplate, specializedStruct, methodAst, specializedName, concreteTemplateArgs, moduleIndex);
            }
        }

        if (targetedModule->templateInterfaceImplAsts.count(templateName)) {
            for (auto& implAst : targetedModule->templateInterfaceImplAsts.at(templateName)) {
                specializeInterfaceImplementation(implAst, selfType, specializedName, concreteTemplateArgs);
            }
        }

        moduleContext->popTemplateBuilder();
        popModuleContext();

        return specializedStructIndex;
    }

    void visitor::specializeStructMethod(const std::shared_ptr<IRStructTemplate> &structTemplate,
                                         const std::shared_ptr<IRStructDefinition> &specializedStruct,
                                         yoi::implInnerPair *methodAstNode,
                                         const yoi::wstr &specializedStructName,
                                         const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                         yoi::indexT moduleIndex) {

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);

        // Push the template's own context to resolve generic types like 'T' to their placeholder
        // 'incompleteTemplateType'.
        IRTemplateBuilder genericContext;
        genericContext.templateArguments = structTemplate->templateArguments;
        genericContext.addTemplateArgument(
            L"STRUCT",
            managedPtr(IRValueType{IRValueType::valueType::structObject,
                                   currentModuleIndex,
                                   irModule->structTable.getIndex(specializedStructName)}));
        moduleContext->pushTemplateBuilder(genericContext);

        yoi::wstr baseMethodName;
        yoi::wstr genericMethodKey;
        yoi::vec<std::shared_ptr<IRValueType>> genericArgTypes;

        if (methodAstNode->isConstructor()) {
            baseMethodName = L"constructor";
            for (auto &arg : methodAstNode->getConstructor().getArgs().get()) {
                genericArgTypes.push_back(managedPtr(parseTypeSpec(&arg->getSpec())));
            }
            genericMethodKey = baseMethodName + getFuncUniqueNameStr(genericArgTypes);
        } else {
            baseMethodName = methodAstNode->getMethod().getName().get().strVal;
            for (auto &arg : methodAstNode->getMethod().getArgs().get()) {
                genericArgTypes.push_back(managedPtr(parseTypeSpec(&arg->getSpec())));
            }
            genericMethodKey = baseMethodName + getFuncUniqueNameStr(genericArgTypes);
        }

        moduleContext->popTemplateBuilder(); // Done with generic context

        std::shared_ptr<IRFunctionTemplate> methodTemplate;
        // yoi_assert(, 0, 0, "Method implementation does not match any template method definition: " +
        // wstring2string(genericMethodKey));
        for (auto &methodPair : structTemplate->templateMethods) {
            if (methodPair.first == genericMethodKey) {
                methodTemplate = methodPair.second;
            } else {
                if (getSpecializedMangledMethodName(
                        genericContext.templateArguments, methodPair.first, concreteTemplateArgs) == genericMethodKey) {
                    methodTemplate = methodPair.second;
                }
            }
        }

        if (!methodTemplate) {
            throw std::runtime_error("Method implementation does not match any template method definition: " +
                                     wstring2string(genericMethodKey));
        }

        IRTemplateBuilder specializationContext;
        for (size_t i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structTemplate->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject,
                                               currentModuleIndex,
                                               targetedModule->structTable.getIndex(specializedStructName)});
        specializationContext.addTemplateArgument(L"STRUCT", selfType);
        moduleContext->pushTemplateBuilder(specializationContext);

        IRFunctionDefinition::Builder funcBuilder;

        funcBuilder.setDebugInfo({targetedModule->modulePath, methodAstNode->getLine(), methodAstNode->getColumn()});

        yoi::vec<std::shared_ptr<IRValueType>> specializedArgTypes;

        funcBuilder.addArgument(L"this", selfType); // Specialized 'this'

        if (methodAstNode->isConstructor()) {
            for (auto &arg : methodAstNode->getConstructor().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                funcBuilder.addArgument(arg->getId().get().strVal, specializedType);
                specializedArgTypes.push_back(specializedType);
            }
            funcBuilder.setReturnType(selfType);
        } else {
            for (auto &arg : methodAstNode->getMethod().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                funcBuilder.addArgument(arg->getId().get().strVal, specializedType);
                specializedArgTypes.push_back(specializedType);
            }
            funcBuilder.setReturnType(managedPtr(parseTypeSpec(&methodAstNode->getMethod().getResultType())));
        }

        yoi::wstr specializedMethodName =
            specializedStructName + L"::" + baseMethodName + getFuncUniqueNameStr(specializedArgTypes);
        funcBuilder.setName(specializedMethodName);

        auto specializedFunc = funcBuilder.yield();
        auto funcIndex = targetedModule->functionTable.put_create(specializedMethodName, specializedFunc);

        targetedModule->structTable[specializedStructName]
            ->nameIndexMap[baseMethodName + getFuncUniqueNameStr(specializedArgTypes)] = {
            IRStructDefinition::nameInfo::nameType::method, funcIndex};

        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), targetedModule, specializedFunc});
        moduleContext->getIRBuilder().setDebugInfo({targetedModule->modulePath, methodAstNode->getLine(), methodAstNode->getColumn()});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(methodAstNode->isConstructor() ? &methodAstNode->getConstructor().getBlock()
                                             : &methodAstNode->getMethod().getBlock(),
              true);
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();

        moduleContext->popTemplateBuilder();
    }

    yoi::wstr
    visitor::getSpecializedMangledMethodName(yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArgs,
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
            yoi::indexT targetModule = -1, lastModule = -1;
            while (it + 1 != exportDecl->from->member->getTerms().end() &&
                   (targetModule = isModuleName(*it, targetModule)) != lastModule) {
                it++;
                lastModule = targetModule;
            }
            yoi_assert(it + 1 == exportDecl->from->member->getTerms().end(),
                       exportDecl->getLine(),
                       exportDecl->getColumn(),
                       "Expected a identifier after modules but this is not the final term of expression.");
            targetModule = targetModule == -1 ? currentModuleIndex : targetModule;

            yoi::indexT funcIndex = -1;

            if (!(*it)->hasTemplateArg()) {
                for (auto funcIt =
                         moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable.begin();
                     funcIt !=
                     moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable.end();
                     funcIt++) {
                    if (funcIt->first.starts_with((*it)->id->get().strVal + L"#")) {
                        // an mangled name of target function
                        funcIndex = std::distance(
                            moduleContext->getCompilerContext()->getImportedModule(targetModule)->functionTable.begin(),
                            funcIt);
                    }
                }
            }

            if (funcIndex != -1) {
                auto attrs = getFunctionAttributes(exportDecl->attrs);
                moduleContext->getCompilerContext()->getIRFFITable()->addExportedFunction(
                    exportIdentifier, targetModule, funcIndex, attrs);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            } else {
                // try template
                auto templateName = (*it)->id->get().strVal;
                auto templateIndex = moduleContext->getCompilerContext()
                                         ->getImportedModule(targetModule)
                                         ->functionTemplateTable.getIndex(templateName);
                yoi_assert((*it)->hasTemplateArg(),
                           exportDecl->getLine(),
                           exportDecl->getColumn(),
                           "Expected template arguments for template: " + wstring2string(templateName));

                auto templateArgs = parseTemplateArgs((*it)->getArg());

                auto funcIndex = specializeFunctionTemplate(moduleContext->getCompilerContext()
                                                                ->getImportedModule(targetModule)
                                                                ->functionTemplateTable[templateName],
                                                            moduleContext->getCompilerContext()
                                                                ->getImportedModule(targetModule)
                                                                ->funcTemplateAsts[templateName],
                                                            templateArgs, targetModule);

                auto attrs = getFunctionAttributes(exportDecl->attrs);

                moduleContext->getCompilerContext()->getIRFFITable()->addExportedFunction(
                    exportIdentifier, targetModule, funcIndex, attrs);
                return moduleContext->getIRBuilder().getCurrentInsertionPoint();
            }
        } catch (std::out_of_range &e) {
            panic(exportDecl->getLine(),
                  exportDecl->getColumn(),
                  "None of the existing types and functions match the name: " +
                      wstring2string(exportDecl->as->node.strVal));
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
        yoi_assert(!from.empty(),
                   importDecl->getLine(),
                   importDecl->getColumn(),
                   "Cannot find the file: " + wstring2string(importDecl->from_path.strVal));

        // import method implementation
        // parse method signature and add to import table
        auto funcName = importDecl->inner->name->get().strVal;
        IRFunctionDefinition::Builder builder;

        builder.attrs = getFunctionAttributes(importDecl->inner->attrs);
        builder.setDebugInfo({irModule->modulePath, importDecl->inner->getLine(), importDecl->inner->getColumn()});

        builder.setReturnType(managedPtr(parseTypeSpec(importDecl->inner->resultType)));
        for (auto &arg : importDecl->inner->args->get()) {
            auto argType = managedPtr(parseTypeSpec(&arg->getSpec()));
            builder.addArgument(arg->getId().get().strVal, argType);
        }
        builder.setName(funcName);
        auto importedFunc = builder.yield();
        auto importedIndex =
            moduleContext->getCompilerContext()->getIRFFITable()->addImportedFunction(from, funcName, importedFunc);

        // add to extern table
        irModule->externTable.put_create(
            funcName,
            managedPtr(
                IRExternEntry{IRExternEntry::externType::importedFunction,
                              funcName,
                              moduleContext->getCompilerContext()->getIRFFITable()->importedLibraries.getIndex(from),
                              importedIndex}));

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::tryCastTo(const std::shared_ptr<IRValueType> &toType) {
        auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        if (*rhs == *toType) {
            return;
        } else if (rhs->isArrayType() && toType->isDynamicArrayType()) {
            return; // two type of array is ABI compatible
        } else if (rhs->isBasicType() && toType->isBasicType() && !rhs->isArrayType() && !toType->isArrayType()) {
            emitBasicCastTo(toType);
        } else if (toType->type == IRValueType::valueType::interfaceObject) {
            // check implemented interfaces
            try {
                auto implName = getInterfaceImplName({toType->typeAffiliateModule, toType->typeIndex}, rhs);
                auto implIndex = moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->interfaceImplementationTable.getIndex(implName);
                // construct interface object
                moduleContext->getIRBuilder().newInterfaceOp(toType->typeIndex, toType->typeAffiliateModule != currentModuleIndex, toType->typeAffiliateModule);
                moduleContext->getIRBuilder().constructInterfaceImplOp(implIndex, rhs->typeAffiliateModule != currentModuleIndex, rhs->typeAffiliateModule);
            } catch (std::out_of_range &e) {
                panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to interface " + yoi::wstring2string((toType->to_string())) + ": no implementation found.");
            }
        } else if (toType->type == IRValueType::valueType::structObject) {
            // check whether owns the constructor
            auto structType = moduleContext->getCompilerContext()->getImportedModule(toType->typeAffiliateModule)->structTable[toType->typeIndex];
            auto constructorName = L"constructor" + getFuncUniqueNameStr({rhs});
            if (structType->nameIndexMap.contains(constructorName)) {
                panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to struct " + yoi::wstring2string((toType->to_string())) + ": target type contains a constructor with corresponding params but inexplicit conversion is not allowed.");
            }
        } else {
            panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to " + yoi::wstring2string((toType->to_string())) + ": no viable conversion found.");
        }
    }

    yoi::indexT visitor::visit(yoi::typeIdExpression *typeIdExpression) {
        if (typeIdExpression->type) {
            auto parsedType = managedPtr(parseTypeSpec(typeIdExpression->type));
            moduleContext->getIRBuilder().typeIdOp(parsedType);
        } else {
            moduleContext->getIRBuilder().saveState();
            visit(typeIdExpression->expr);
            auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            moduleContext->getIRBuilder().restoreState();
            moduleContext->getIRBuilder().typeIdOp(rhs);
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::dynCastExpression *dynCastExpression) {
        visit(dynCastExpression->expr);
        auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        auto toType = managedPtr(parseTypeSpec(dynCastExpression->type));
        yoi_assert(rhs->type == IRValueType::valueType::interfaceObject, dynCastExpression->expr->getLine(), dynCastExpression->expr->getColumn(), "dynamic cast can only be applied to interface objects to struct objects.");

        auto &impls = moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->interfaceTable[rhs->typeIndex]->implementations;

        if (auto it = std::find(impls.begin(), impls.end(), std::make_tuple(toType->type, toType->typeAffiliateModule, toType->typeIndex)); it != impls.end())
            moduleContext->getIRBuilder().dynCastOp(toType);
        else
            panic(dynCastExpression->getLine(), dynCastExpression->getColumn(), "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to " + yoi::wstring2string((toType->to_string())) + ": no implementation found.");

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::generateNullInterfaceImplementation(const std::shared_ptr<IRValueType> &structType) {
        auto nullInterface = std::make_pair(HOSHI_COMPILER_CTX_GLOB_ID_CONST, 0);
        auto nullImplName = getInterfaceImplName(nullInterface, structType);
        moduleContext->getCompilerContext()->getImportedModule(HOSHI_COMPILER_CTX_GLOB_ID_CONST)->interfaceTable[0]->implementations.emplace_back(
            structType->type, structType->typeAffiliateModule, structType->typeIndex);
        try {
            return irModule->interfaceImplementationTable.getIndex(nullImplName);
        } catch (std::out_of_range &e) {
            auto nullImpl = managedPtr(IRInterfaceImplementationDefinition{nullImplName, {structType->type, structType->typeAffiliateModule, structType->typeIndex}, 0, {}, {}});
            return irModule->interfaceImplementationTable.put_create(nullImplName, nullImpl);
        }
    }

    yoi::vec<IRFunctionDefinition::FunctionAttrs> visitor::getFunctionAttributes(const yoi::vec<lexer::token> &attrs) {
        yoi::vec<IRFunctionDefinition::FunctionAttrs> res;
        for (auto &attr : attrs) {
            switch (attr.kind) {
                case lexer::token::tokenKind::kAlwaysInline:
                    res.push_back(IRFunctionDefinition::FunctionAttrs::AlwaysInline);
                    break;
                case lexer::token::tokenKind::kNoFFI:
                    res.push_back(IRFunctionDefinition::FunctionAttrs::NoFFI);
                    break;
                case lexer::token::tokenKind::kStatic:
                    res.push_back(IRFunctionDefinition::FunctionAttrs::Static);
                    break;
                default:
                    break;
            }
        }
        return std::move(res);
    }

    yoi::indexT visitor::visit(yoi::newExpression *newExpression) {
        auto baseType = parseTypeSpec(newExpression->type);
        for (auto &i : newExpression->args->get()) {
            visit(i);
            tryCastTo(managedPtr(baseType));
        }
        visit(newExpression->length->expr);
        moduleContext->getIRBuilder().newDynamicArrayOp(managedPtr(baseType), newExpression->args->get().size());
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::isModuleName(identifier *it, yoi::indexT currentModule) const {
        std::shared_ptr<yoi::IRModule> target =
            currentModule == -1 ? irModule : moduleContext->getCompilerContext()->getImportedModule(currentModule);
        if (auto x = target->moduleImports.find(it->node.strVal); x != target->moduleImports.end()) {
            return x->second;
        } else {
            return currentModule;
        }
    }

    IRValueType visitor::parseTypeSpec(yoi::externModuleAccessExpression *emaExpression) {
        auto it = emaExpression->getTerms().begin();
        yoi::indexT targetModule = -1, lastModule = -1;
        while (it + 1 != emaExpression->getTerms().end() && (targetModule = isModuleName((*it)->id, lastModule)) != lastModule) {
            it++;
            lastModule = targetModule;
        }

        bool whetherLastTerm = it + 1 == emaExpression->getTerms().end();
        if (targetModule == -1 || targetModule == currentModuleIndex)
            return parseTypeSpec((*it));
        else
            return parseTypeSpecExtern((*it), targetModule);
    }

    bool visitor::OverloadResult::found() const {
        return functionIndex != -1;
    }

    yoi::vec<std::shared_ptr<IRValueType>> visitor::evaluateArguments(yoi::invocationArguments *args) {
        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
        for (auto& arg : args->get()) {
            visit(arg);
            argTypes.push_back(moduleContext->getIRBuilder().getRhsFromTempVarStack());
        }
        return argTypes;
    }

    visitor::OverloadResult visitor::resolveOverloadExtern(const yoi::wstr &baseName,
                                                           const yoi::vec<std::shared_ptr<IRValueType>> &argTypes,
                                                           yoi::indexT targetModule,
                                                           const std::shared_ptr<IRStructDefinition> &structContext) {
        OverloadResult result;
        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);

        // Pass 1: Look for an exact, non-variadic match in the target module.
        auto exactMangledName = baseName + getFuncUniqueNameStr(argTypes);
        yoi::wstr lookupName = structContext ? structContext->name + L"::" + exactMangledName : exactMangledName;

        if (targetedModule->functionTable.contains(lookupName)) {
            result.functionIndex = targetedModule->functionTable.getIndex(lookupName);
            result.function = targetedModule->functionTable[result.functionIndex];
            // if (result.function->isVariadic) {
            if (std::find(result.function->attrs.begin(),
                          result.function->attrs.end(),
                          IRFunctionDefinition::FunctionAttrs::Variadic) != result.function->attrs.end()) {
                result.isVariadic = true;
                result.fixedArgCount = result.function->argumentTypes.size() - 1;
                result.variadicElementType = managedPtr(result.function->argumentTypes.back()->getElementType());
            }
            return result;
        }

        // Pass 2: Look for a compatible variadic match in the target module.
        auto findVariadicMatch = [&](const yoi::wstr &funcKey, bool skipFirstParam = false) {
            auto func = targetedModule->functionTable[funcKey];
            if (std::find(func->attrs.begin(), func->attrs.end(), IRFunctionDefinition::FunctionAttrs::Variadic) !=
                func->attrs.end()) {
                const auto &paramTypes = func->argumentTypes;
                size_t fixedParamCount = paramTypes.size() - 1 - (skipFirstParam && paramTypes.size() > 1 ? 1 : 0);
                if (argTypes.size() >= fixedParamCount) {
                    bool fixedMatch = true;
                    for (size_t i = skipFirstParam ? 1 : 0; i < fixedParamCount; ++i) {
                        if (*paramTypes[i] != *argTypes[i]) {
                            fixedMatch = false;
                            break;
                        }
                    }
                    if (fixedMatch) {
                        result.functionIndex = targetedModule->functionTable.getIndex(funcKey);
                        result.isVariadic = true;
                        result.fixedArgCount = fixedParamCount;
                        result.variadicElementType = managedPtr(paramTypes.back()->getElementType());
                        result.function = func;
                        return true;
                    }
                }
            }
            return false;
        };

        yoi::wstr prefix = structContext ? structContext->name + L"::" + baseName : baseName;
        for (const auto &[key, value] : targetedModule->functionTable) {
            if (key.starts_with(prefix)) {
                if (findVariadicMatch(key, structContext != nullptr))
                    return result;
            }
        }

        if (!structContext && targetedModule->functionTemplateTable.contains(baseName)) {
            try {
                auto funcTemplate = targetedModule->functionTemplateTable[baseName];
                auto astNode = targetedModule->funcTemplateAsts.at(baseName);
                yoi::vec<std::shared_ptr<IRValueType>> deducedArgs(funcTemplate->templateArguments.size());
                
                // --- FULL TEMPLATE ARGUMENT DEDUCTION LOGIC ---
                for (yoi::indexT i = 0; i < argTypes.size(); i++) {
                    if (i < funcTemplate->templateDefinition->argumentTypes.size() &&
                        funcTemplate->templateDefinition->argumentTypes[i]->type == IRValueType::valueType::incompleteTemplateType) {
                        auto &srcTypeToPlace = argTypes[i];
                        auto incompleteTypeIndex = funcTemplate->templateDefinition->argumentTypes[i]->typeIndex;
                        if (deducedArgs[incompleteTypeIndex] && *deducedArgs[incompleteTypeIndex] != *srcTypeToPlace) {
                             throw std::runtime_error("Template argument type mismatch during deduction.");
                        }
                        deducedArgs[incompleteTypeIndex] = srcTypeToPlace;
                    }
                }
                for (yoi::indexT i = 0; i < deducedArgs.size(); i++) {
                    if (deducedArgs[i] == nullptr) {
                        throw std::runtime_error("Cannot deduce all template arguments for: " + yoi::wstring2string(funcTemplate->templateArguments.getKey(i)));
                    }
                }
                
                auto specializedFuncIndex = specializeFunctionTemplate(funcTemplate, astNode, deducedArgs, targetModule);
                result.functionIndex = specializedFuncIndex;
                result.function = irModule->functionTable[specializedFuncIndex];

                // The newly specialized function might itself be variadic
                if(std::find(result.function->attrs.begin(), result.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Variadic) != result.function->attrs.end()) {
                    result.isVariadic = true;
                    result.fixedArgCount = result.function->argumentTypes.size() - 1;
                    result.variadicElementType = managedPtr(result.function->argumentTypes.back()->getElementType());
                }
                return result;
            } catch (const std::exception&) {
                result.functionIndex = -1; 
            }
        }

        return result; // Not found
    }

    bool visitor::handleInvocationExtern(const yoi::wstr &baseName,
                                         yoi::invocationArguments *args,
                                         yoi::indexT targetModule,
                                         const std::shared_ptr<IRValueType> &structContext) {
        moduleContext->getIRBuilder().saveState();
        auto argTypes = evaluateArguments(args);
        OverloadResult overload;
        if (structContext && structContext->type == IRValueType::valueType::structObject) {
            auto structType = moduleContext->getCompilerContext()->getImportedModule(structContext->typeAffiliateModule)->structTable[structContext->typeIndex];
            overload = resolveOverloadExtern(baseName, argTypes, targetModule, structType);
        } else if (structContext && structContext->type == IRValueType::valueType::interfaceObject) {
            auto interfaceType = moduleContext->getCompilerContext()->getImportedModule(structContext->typeAffiliateModule)->interfaceTable[structContext->typeIndex];
            overload = resolveOverloadInterface(baseName, argTypes, targetModule, interfaceType);
        } else {
            overload = resolveOverloadExtern(baseName, argTypes, targetModule, nullptr);
        }

        if (!overload.found()) {
            moduleContext->getIRBuilder().restoreState();
            return false;
        }

        auto fullMangledName = overload.function->name;

        if (overload.isVariadic) {
            moduleContext->getIRBuilder().restoreState();
            for (size_t i = 0; i < overload.fixedArgCount; ++i) {
                visit(args->get()[i]);
            }
            auto variadicArgCount = argTypes.size() - overload.fixedArgCount;
            if (variadicArgCount > 0) {
                for (size_t i = 0; i < variadicArgCount; ++i) {
                    visit(args->get()[i + overload.fixedArgCount]);
                    tryCastTo(overload.variadicElementType);
                }
                moduleContext->getIRBuilder().newArrayOp(overload.variadicElementType,
                                                         {static_cast<yoi::indexT>(variadicArgCount)});
            } else {
                moduleContext->getIRBuilder().newArrayOp(overload.variadicElementType, {0});
            }
        } else {
            moduleContext->getIRBuilder().discardState();
        }

        size_t finalParamCount = overload.function->argumentTypes.size();
        if (structContext && structContext->type == IRValueType::valueType::structObject) {
            auto externEntry = getExternEntry(targetModule, fullMangledName);
            auto isStaticMethod = std::find(overload.function->attrs.begin(), overload.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) != overload.function->attrs.end();
            moduleContext->getIRBuilder().invokeMethodOp(externEntry.itemIndex,
                                                         finalParamCount - !isStaticMethod,
                                                         overload.function->returnType,
                                                         isStaticMethod,
                                                         true,
                                                         externEntry.affiliateModule);
        } else if (structContext && structContext->type == IRValueType::valueType::interfaceObject) {
            moduleContext->getIRBuilder().invokeVirtualOp(overload.functionIndex,
                                                          finalParamCount,
                                                          overload.function->returnType,
                                                          true,
                                                          structContext->typeAffiliateModule);
        } else {
            auto externEntry = getExternEntry(targetModule, fullMangledName);
            moduleContext->getIRBuilder().invokeOp(externEntry.itemIndex,
                                                   finalParamCount,
                                                   overload.function->returnType,
                                                   true,
                                                   externEntry.affiliateModule);
        }
        return true;
    }

    yoi::indexT visitor::handleBinaryOperatorOverload(const yoi::wstr &overloadName) {
        auto lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
        auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        bool isResolved = false;

        if (lhs->type == IRValueType::valueType::structObject)  {
            auto resolved = resolveOverloadExtern(overloadName, {lhs, rhs}, lhs->typeAffiliateModule, moduleContext->getCompilerContext()->getImportedModule(lhs->typeAffiliateModule)->structTable[lhs->typeIndex]);
            if (resolved.found()) {
                yoi_assert(resolved.isVariadic == false, 0, 0, "Binary operator overloading with variadic functions is not supported.");
                yoi_assert(std::find(resolved.function->attrs.begin(), resolved.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) != resolved.function->attrs.end(), moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Binary operator overloading with non-static functions is not supported.");

                // same as below
                moduleContext->getIRBuilder().invokeMethodOp(
                    resolved.functionIndex, 1, resolved.function->returnType, false, true, lhs->typeAffiliateModule);
                isResolved = true;
            }
        } 
        if (!isResolved && rhs->type == IRValueType::valueType::structObject) {
            auto resolved = resolveOverloadExtern(overloadName, {lhs, rhs}, rhs->typeAffiliateModule, moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->structTable[rhs->typeIndex]);
            if (resolved.found()) {
                yoi_assert(resolved.isVariadic == false, moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Binary operator overloading with variadic functions is not supported.");
                yoi_assert(std::find(resolved.function->attrs.begin(), resolved.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) != resolved.function->attrs.end(), moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Binary operator overloading with non-static functions is not supported.");

                // trick here: since when we set isStatic to true, we need 3 elements on the stack, which this ptr should also be present.
                // however, we only have 2 elements on the stack which is lhs and rhs, so, we set isStatic to false here.
                // to trick the invoke method op into generating the correct code
                // this way, this method would take two elements from the stack and push the result to the stack.
                moduleContext->getIRBuilder().invokeMethodOp(
                    resolved.functionIndex, 1, resolved.function->returnType, false, true, rhs->typeAffiliateModule);
                isResolved = true;
            }
        } 
        if (!isResolved && lhs->type == IRValueType::valueType::interfaceObject) {
            const auto& baseName = overloadName;
            auto mangledName = getFuncUniqueNameStr({rhs});

            try {
                auto methodIdx = moduleContext->getCompilerContext()
                                    ->getImportedModule(lhs->typeAffiliateModule)
                                    ->interfaceTable[lhs->typeIndex]
                                    ->methodMap.getIndex(baseName + mangledName);
                auto method = moduleContext->getCompilerContext()
                                ->getImportedModule(lhs->typeAffiliateModule)
                                ->interfaceTable[lhs->typeIndex]
                                ->methodMap[methodIdx];
                yoi_assert(method->argumentTypes.size() == 1,
                        moduleContext->getIRBuilder().getCurrentDebugInfo().line,
                        moduleContext->getIRBuilder().getCurrentDebugInfo().column,
                        "Argument count does not match");
                moduleContext->getIRBuilder().invokeVirtualOp(methodIdx, 1, method->returnType);
                isResolved = true;
            } catch (const std::out_of_range &) {

            }
        }
        yoi_assert(isResolved, moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Binary operator overloading not found for " + yoi::wstring2string(overloadName));

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::handleUnaryOperatorOverload(const yoi::wstr &overloadName) {
        auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        bool isResolved = false;

        if (rhs->type == IRValueType::valueType::structObject) {
            auto resolved = resolveOverloadExtern(overloadName, {rhs}, rhs->typeAffiliateModule, moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->structTable[rhs->typeIndex]);
            if (resolved.found()) {
                yoi_assert(resolved.isVariadic == false, moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Unary operator overloading with variadic functions is not supported.");
                yoi_assert(std::find(resolved.function->attrs.begin(), resolved.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) != resolved.function->attrs.end(), moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Unary operator overloading with non-static functions is not supported.");
                // why isStatic = false? check the comment in handleBinaryOperatorOverload
                moduleContext->getIRBuilder().invokeMethodOp(
                    resolved.functionIndex, 0, resolved.function->returnType, false, true, rhs->typeAffiliateModule);
                isResolved = true;
            }
        } 
        if (rhs->type == IRValueType::valueType::interfaceObject) {
            const auto &baseName = overloadName;
            auto mangledName = getFuncUniqueNameStr({});

            auto methodIdx = moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->interfaceTable[rhs->typeIndex]
                                 ->methodMap.getIndex(baseName + mangledName);
            auto method = moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->interfaceTable[rhs->typeIndex]->methodMap[methodIdx];
            yoi_assert(method->argumentTypes.size() == 1,
                       moduleContext->getIRBuilder().getCurrentDebugInfo().line,
                       moduleContext->getIRBuilder().getCurrentDebugInfo().column,
                       "Argument count does not match");
            moduleContext->getIRBuilder().invokeVirtualOp(methodIdx, 0, method->returnType);
            isResolved = true;
        }
        yoi_assert(isResolved, moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Unary operator overloading not found for " + yoi::wstring2string(overloadName));

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::specializeInterfaceTemplate(const yoi::wstr &templateName,
                                             const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs) {
        yoi::wstr specializedName = getMangledTemplateName(templateName, concreteTemplateArgs);
        if (irModule->interfaceTable.contains(specializedName)) {
            return irModule->interfaceTable.getIndex(specializedName);
        }
        
        yoi_assert(irModule->interfaceInstanceTemplateTable.contains(templateName), 0, 0, "Unknown interface template: " + wstring2string(templateName));
        
        auto interfaceTemplate = irModule->interfaceInstanceTemplateTable[templateName];
        auto interfaceAst = irModule->templateInterfaceAsts.at(templateName);

        IRTemplateBuilder specializationContext;
        yoi_assert(concreteTemplateArgs.size() == interfaceTemplate->templateArguments.size(), 0, 0, "Template argument count mismatch for interface " + wstring2string(templateName));
        for (yoi::indexT i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = interfaceTemplate->templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        
        moduleContext->pushTemplateBuilder(specializationContext);
        
        IRInterfaceInstanceDefinition::Builder builder;
        builder.setName(specializedName);

        for (auto &i : interfaceAst->getInner().getInner()) {
            bool isVaridic = false;
            yoi_assert(i->isMethod(), i->getLine(), i->getColumn(), "Interface member must be a method");
            auto methodName = i->getMethod().getName().get().strVal;
            auto methodResultType = managedPtr(parseTypeSpec(i->getMethod().resultType));
            yoi::vec<std::shared_ptr<IRValueType>> argTypes;
            IRFunctionDefinition::Builder methodBuilder;
            methodBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
            methodBuilder.setReturnType(methodResultType);
            for (auto &arg : i->getMethod().getArgs().get()) {
                if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3) {
                    isVaridic = true;
                    methodBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                    auto argName = arg->getId().node.strVal;
                    auto argType = managedPtr(moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType());
                    methodBuilder.addArgument(argName, argType);
                    argTypes.push_back(argType);
                    break;
                }
                auto argName = arg->getId().get().strVal;
                auto argType = managedPtr(parseTypeSpec(arg->spec));
                methodBuilder.addArgument(argName, argType);
                argTypes.push_back(argType);
            }
            auto uniq = getFuncUniqueNameStr(argTypes);
            methodBuilder.setName(L"interface#" + specializedName + L"#" + methodName + uniq);
            builder.addMethod(methodName + uniq, methodBuilder.yield());
        }

        auto specializedInterface = builder.yield();
        auto interfaceIndex = irModule->interfaceTable.put_create(specializedName, specializedInterface);

        moduleContext->popTemplateBuilder();
        return interfaceIndex;
    }

    void visitor::specializeInterfaceImplementation(yoi::implStmt *implAst,
                                                const std::shared_ptr<IRValueType> &concreteStructType,
                                                const yoi::wstr& specializedStructName,
                                                const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs) {
        
        yoi_assert(implAst->isImplForStmt(), implAst->getLine(), implAst->getColumn(), "Expected 'impl for' AST node for interface implementation specialization.");

        // The active specialization context (from specializeStructTemplate) resolves types like `T` to concrete types.
        auto concreteInterfaceType = managedPtr(parseTypeSpec(implAst->interfaceName));
        yoi_assert(concreteInterfaceType->type == IRValueType::valueType::interfaceObject, implAst->getLine(), implAst->getColumn(), "Expected an interface type.");

        auto interfaceSrcPair = std::make_pair(concreteInterfaceType->typeAffiliateModule, concreteInterfaceType->typeIndex);

        moduleContext->getCompilerContext()->getImportedModule(interfaceSrcPair.first)
            ->interfaceTable[interfaceSrcPair.second]
            ->implementations.emplace_back(
                concreteStructType->type, concreteStructType->typeAffiliateModule, concreteStructType->typeIndex
            );
        
        auto implName = getInterfaceImplName(interfaceSrcPair, concreteStructType);
        if (irModule->interfaceImplementationTable.contains(implName)) {
            return; // Already specialized and created.
        }

        auto implIndex = irModule->interfaceImplementationTable.put_create(implName, nullptr);

        IRInterfaceImplementationDefinition::Builder builder;
        builder.setName(implName);
        builder.setImplStructIndex({concreteStructType->type, concreteStructType->typeAffiliateModule, concreteStructType->typeIndex});
        builder.setImplInterfaceIndex(interfaceSrcPair.second);

        for (auto &methodNode : implAst->getInner().getInner()) {
            yoi_assert(!methodNode->isConstructor(), methodNode->getLine(), methodNode->getColumn(), "Only methods are allowed in interface implementations.");
            auto &methodAst = methodNode->getMethod();

            IRFunctionDefinition::Builder methodBuilder;
            methodBuilder.setDebugInfo({irModule->modulePath, methodAst.getLine(), methodAst.getColumn()});
            methodBuilder.attrs = getFunctionAttributes(methodAst.attrs);
            
            yoi::vec<std::shared_ptr<IRValueType>> specializedArgTypes;
            
            if (std::find(methodBuilder.attrs.begin(), methodBuilder.attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) == methodBuilder.attrs.end()) {
                methodBuilder.addArgument(L"this", concreteStructType);
            }

            for (auto &arg : methodAst.getArgs().get()) {
                auto specializedArgType = managedPtr(parseTypeSpec(arg->spec));
                methodBuilder.addArgument(arg->getId().get().strVal, specializedArgType);
                specializedArgTypes.push_back(specializedArgType);
            }
            
            auto uniq = getFuncUniqueNameStr(specializedArgTypes);
            auto baseMethodName = methodAst.getName().get().strVal;
            
            methodBuilder.setReturnType(managedPtr(parseTypeSpec(methodAst.resultType)));
            methodBuilder.setName(specializedStructName + L"::" + baseMethodName + uniq);

            auto func = methodBuilder.yield();
            auto funcIndex = irModule->functionTable.put_create(func->name, func);
            builder.addVirtualMethod(
                baseMethodName + uniq,
                managedPtr(IRValueType{IRValueType::valueType::virtualMethod, currentModuleIndex, funcIndex}));

            moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
            moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, methodAst.getLine(), methodAst.getColumn()});
            moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
            visit(methodAst.block, true);
            moduleContext->getIRBuilder().yield();
            moduleContext->popIRBuilder();
        }
        
        irModule->interfaceImplementationTable[implIndex] = builder.yield();
    }

    visitor::OverloadResult
    visitor::resolveOverloadInterface(const yoi::wstr &baseName,
                                      const yoi::vec<std::shared_ptr<IRValueType>> &argTypes,
                                      yoi::indexT targetModule,
                                      const std::shared_ptr<IRInterfaceInstanceDefinition> &interfaceContext) {
        OverloadResult result;
        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);

        auto exactMangledName = baseName + getFuncUniqueNameStr(argTypes);

        if (interfaceContext->methodMap.contains(exactMangledName)) {
            result.isVirtual = true;
            result.functionIndex = interfaceContext->methodMap.getIndex(exactMangledName);
            result.function = interfaceContext->methodMap[result.functionIndex];
            if (std::find(result.function->attrs.begin(),
                          result.function->attrs.end(),
                          IRFunctionDefinition::FunctionAttrs::Variadic) != result.function->attrs.end()) {
                result.isVariadic = true;
                result.fixedArgCount = result.function->argumentTypes.size() - 1;
                result.variadicElementType = managedPtr(result.function->argumentTypes.back()->getElementType());
            }
            return result;
        }

        auto findVariadicMatch = [&](const yoi::wstr &funcKey) {
            auto func = interfaceContext->methodMap[funcKey];
            if (std::find(func->attrs.begin(), func->attrs.end(), IRFunctionDefinition::FunctionAttrs::Variadic) !=
                func->attrs.end()) {
                const auto &paramTypes = func->argumentTypes;
                size_t fixedParamCount = paramTypes.size() - 1;
                if (argTypes.size() >= fixedParamCount) {
                    bool fixedMatch = true;
                    for (size_t i = 0; i < fixedParamCount; ++i) {
                        if (*paramTypes[i] != *argTypes[i]) {
                            fixedMatch = false;
                            break;
                        }
                    }
                    if (fixedMatch) {
                        result.functionIndex = interfaceContext->methodMap.getIndex(funcKey);
                        result.isVariadic = true;
                        result.isVirtual = true;
                        result.fixedArgCount = fixedParamCount;
                        result.variadicElementType = managedPtr(paramTypes.back()->getElementType());
                        result.function = func;
                        return true;
                    }
                }
            }
            return false;
        };

        for (const auto &[key, value] : targetedModule->functionTable) {
            if (key.starts_with(baseName)) {
                if (findVariadicMatch(key))
                    return result;
            }
        }

        return result; // Not found
    }

    void visitor::pushModuleContext(yoi::indexT moduleIndex) {
        moduleContextStack.emplace(moduleContext, currentModuleIndex);
        this->moduleContext = moduleContext->getCompilerContext()->getModuleContext(moduleIndex);
        currentModuleIndex = moduleIndex;
    }

    void visitor::popModuleContext() {
        this->moduleContext = moduleContextStack.top().first;
        currentModuleIndex = moduleContextStack.top().second;
        moduleContextStack.pop();
    }
} // namespace yoi