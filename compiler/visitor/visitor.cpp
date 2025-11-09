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
#include <exception>
#include <iterator>
#include <memory>
#include <sstream>
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
                moduleContext->getIRBuilder().pushOp(
                    IR::Opcode::push_character, {IROperand::operandType::character, basicLiterals->node.strVal[0]});
                break;
            }
            case yoi::lexer::token::tokenKind::shortInt: {
                moduleContext->getIRBuilder().pushOp(
                    IR::Opcode::push_short, {IROperand::operandType::shortInt, basicLiterals->node.basicVal.vShort});
                break;
            }
            case yoi::lexer::token::tokenKind::unsignedInt: {
                moduleContext->getIRBuilder().pushOp(
                    IR::Opcode::push_unsigned, {IROperand::operandType::unsignedInt, basicLiterals->node.basicVal.vUint});
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
        } catch (std::out_of_range &e) {
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
            case 6: {
                auto lambdaStructIndex = createLambdaUnnamedStruct(primary->lambda);
                auto [lambdaCallableIndex, callableInterface] = createCallableImplementationForLambda(irModule->structTable[lambdaStructIndex], lambdaStructIndex, currentModuleIndex);
                // moduleContext->getIRBuilder().newInterfaceOp(callableInterface.second, true, callableInterface.first);
                moduleContext->getIRBuilder().constructInterfaceImplOp(callableInterface, lambdaCallableIndex);
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
            } else if (abstractExpr->op.kind == lexer::token::tokenKind::kAs) {
                moduleContext->getIRBuilder().discardState();
                auto typeSpec = managedPtr(rhs);
                tryCastTo(typeSpec);
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
            case lexer::token::tokenKind::minus: {
                auto &lhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                if (lhs->isBasicType()) {
                    moduleContext->getIRBuilder().uniqueArithmeticOp(IR::Opcode::negate);
                } else {
                    handleUnaryOperatorOverload(L"operator-");
                }
                break;
            }
            case lexer::token::tokenKind::unknown: {
                // means no op
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
                    moduleContext->getIRBuilder().saveState();
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        tryCastTo(lhs);
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                        moduleContext->getIRBuilder().discardState();
                    } else {
                        handleBinaryOperatorOverload(L"operator+=", leftExpr->rhs);
                    }
                    break;
                }
                case lexer::token::tokenKind::subtractionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    moduleContext->getIRBuilder().saveState();
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        tryCastTo(lhs);
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                        moduleContext->getIRBuilder().discardState();
                    } else {
                        handleBinaryOperatorOverload(L"operator-=", leftExpr->rhs);
                    }
                    break;
                }
                case lexer::token::tokenKind::multiplicationAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    moduleContext->getIRBuilder().saveState();
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        tryCastTo(lhs);
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                        moduleContext->getIRBuilder().discardState();
                    } else {
                        handleBinaryOperatorOverload(L"operator*=", leftExpr->rhs);
                    }
                    break;
                }
                case lexer::token::tokenKind::divisionAssignment: {
                    auto lhsPos = visit(leftExpr->lhs);
                    moduleContext->getIRBuilder().saveState();
                    auto rhsPos = visit(leftExpr->rhs);
                    auto &lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                    auto &rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (lhs->isBasicType() && rhs->isBasicType()) {
                        tryCastTo(lhs);
                        moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::div);
                        visit(leftExpr->lhs, true);
                        visit(leftExpr->lhs);
                        moduleContext->getIRBuilder().discardState();
                    } else {
                        handleBinaryOperatorOverload(L"operator/=", leftExpr->rhs);
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
            moduleContext->getIRBuilder().saveState();
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
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::asterisk: handleBinaryOperatorOverload(L"operator*", *term); break;
                    case lexer::token::tokenKind::slash: handleBinaryOperatorOverload(L"operator/", *term); break;
                    case lexer::token::tokenKind::percentSign: handleBinaryOperatorOverload(L"operator%", *term); break;
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
            moduleContext->getIRBuilder().saveState();
            auto rhsPos = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (lhsType->isBasicType() && rhsType->isBasicType()) {
                switch (op->kind) {
                    case lexer::token::tokenKind::plus: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add); break;
                    case lexer::token::tokenKind::minus: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::sub); break;
                    default: panic(op->line, op->col, "Unexpected addition expression operator");
                }
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::plus: handleBinaryOperatorOverload(L"operator+", *term); break;
                    case lexer::token::tokenKind::minus: handleBinaryOperatorOverload(L"operator-", *term); break;
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
            moduleContext->getIRBuilder().saveState();

            emitBasicCastTo(moduleContext->getCompilerContext()->getIntObjectType());
            
            visit(*++term);

            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (lhsType->isBasicType() && rhsType->type == IRValueType::valueType::integerObject) {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryShiftLeft: moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::left_shift); break;
                    case lexer::token::tokenKind::binaryShiftRight: moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::right_shift); break;
                    default:  panic(op->line, op->col, "Unexpected shift expression operator"); break;
                }
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryShiftLeft: handleBinaryOperatorOverload(L"operator<<", *term); break;
                    case lexer::token::tokenKind::binaryShiftRight: handleBinaryOperatorOverload(L"operator>>", *term); break;
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
            moduleContext->getIRBuilder().saveState();
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
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::lessThan: handleBinaryOperatorOverload(L"operator<", *term); break;
                    case lexer::token::tokenKind::greaterThan: handleBinaryOperatorOverload(L"operator>", *term); break;
                    case lexer::token::tokenKind::lessEqual: handleBinaryOperatorOverload(L"operator<=", *term); break;
                    case lexer::token::tokenKind::greaterEqual: handleBinaryOperatorOverload(L"operator>=", *term); break;
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
            moduleContext->getIRBuilder().saveState();
            auto rhsPos = visit(*++term);
            auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
            auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (lhsType->isBasicType() && rhsType->isBasicType() || lhsType->type == IRValueType::valueType::pointerObject || rhsType->type == IRValueType::valueType::pointerObject) {
                switch (op->kind) {
                    case lexer::token::tokenKind::equal: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::equal); break;
                    case lexer::token::tokenKind::notEqual: emitBasicCastInBasicArithOpByLhsAndRhs(lhsPos, rhsPos); moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::not_equal); break;
                    default: panic(op->line, op->col, "Unexpected equality expression operator"); return {};
                }
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::equal: handleBinaryOperatorOverload(L"operator==", *term); break;
                    case lexer::token::tokenKind::notEqual: handleBinaryOperatorOverload(L"operator!=", *term); break;
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
            moduleContext->getIRBuilder().saveState();
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
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryAnd: {
                        handleBinaryOperatorOverload(L"operator&", *term);
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
            moduleContext->getIRBuilder().saveState();
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
                moduleContext->getIRBuilder().discardState();
            } else {
                switch (op->kind) {
                    case lexer::token::tokenKind::binaryXor: {
                        handleBinaryOperatorOverload(L"operator^", *term);
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
                        handleBinaryOperatorOverload(L"operator|", *term);
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
        if (logicalAndExpr->getOp().empty()) {
            return visit(logicalAndExpr->getTerms().front());
        }

        auto finalFalseBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto endBlock = moduleContext->getIRBuilder().createCodeBlock();
        auto terms = logicalAndExpr->getTerms().begin();
        auto ops = logicalAndExpr->getOp().begin();

        visit(*terms);
        auto termType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        if (termType->type != IRValueType::valueType::booleanObject) {
            moduleContext->getIRBuilder().basicCast(
                moduleContext->getCompilerContext()->getBoolObjectType(), {}, true);
        }
        
        for (; ops != logicalAndExpr->getOp().end(); ++ops) {
            auto nextTermBlock = moduleContext->getIRBuilder().createCodeBlock();

            moduleContext->getIRBuilder().jumpIfOp(IR::Opcode::jump_if_false, finalFalseBlock);
            moduleContext->getIRBuilder().jumpOp(nextTermBlock);

            moduleContext->getIRBuilder().switchCodeBlock(nextTermBlock);
            visit(*++terms);
            
            termType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (termType->type != IRValueType::valueType::booleanObject) {
                moduleContext->getIRBuilder().basicCast(
                    moduleContext->getCompilerContext()->getBoolObjectType(), {}, true);
            }
        }
        
        moduleContext->getIRBuilder().jumpOp(endBlock);

        moduleContext->getIRBuilder().switchCodeBlock(finalFalseBlock);
        moduleContext->getIRBuilder().insert({IR::Opcode::push_boolean, {{IROperand::operandType::boolean, IROperand::operandValue{false}}}, moduleContext->getIRBuilder().getCurrentDebugInfo()});
        moduleContext->getIRBuilder().jumpOp(endBlock);

        moduleContext->getIRBuilder().switchCodeBlock(endBlock);

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
            auto nextConditionBlock = moduleContext->getIRBuilder().createCodeBlock();
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
                    moduleContext->getIRBuilder().jumpOp(nextConditionBlock);
                    moduleContext->getIRBuilder().switchCodeBlock(nextConditionBlock);

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

    yoi::indexT visitor::visit(yoi::memberExpr* memberExpr, bool isStoreOp) {
        // take a snapshot for builder state
        auto snapshotIndex = moduleContext->getIRBuilder().saveState();

        auto it = memberExpr->getTerms().begin();
        yoi::indexT targetModule = -1, lastModule = -1;
        // 1. Resolve module prefixes (e.g., std::io)
        while (it + 1 != memberExpr->getTerms().end() && (targetModule = isModuleName((*it)->id, lastModule)) != lastModule) {
            it++;
            lastModule = targetModule;
        }

        // 2. Handle enumerations
        if (it + 2 == memberExpr->getTerms().end()) {
            if ((*it)->isIdentifier() && !(*it)->id->hasTemplateArg() && (*(it + 1))->isIdentifier() && !(*(it + 1))->id->hasTemplateArg() && irModule->enumerationTable.contains((*it)->id->id->node.strVal)) {
                try {
                    auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule == -1 ? currentModuleIndex : targetModule);
                    auto v = targetedModule->enumerationTable[(*it)->id->id->node.strVal]->valueToIndexMap[(*(it + 1))->id->id->node.strVal];
                    IR::Opcode op = IR::Opcode::push_character;
                    IROperand operand;
                    switch (targetedModule->enumerationTable[(*it)->id->id->node.strVal]->getUnderlyingType()) {
                        case IREnumerationType::UnderlyingType::I8: {
                            op = IR::Opcode::push_character;
                            operand = {IROperand::operandType::character, (yoi::wchar)v};
                            break;
                        }
                        case IREnumerationType::UnderlyingType::I16: {
                            op = IR::Opcode::push_short;
                            operand = {IROperand::operandType::character, (short)v};
                            break;
                        }
                        case IREnumerationType::UnderlyingType::I64: {
                            op = IR::Opcode::push_unsigned;
                            operand = {IROperand::operandType::unsignedInt, (yoi::indexT)v};
                            break;
                        }
                    }
                    moduleContext->getIRBuilder().pushOp(op, {operand});

                    moduleContext->getIRBuilder().discardStateUntil(snapshotIndex);
                    return moduleContext->getIRBuilder().getCurrentInsertionPoint();
                } catch(std::out_of_range &e) {
                    panic((*(it + 1))->getLine(), (*(it + 1))->getColumn(), "Enumeration value not found: " + yoi::wstring2string((*(it + 1))->id->id->node.strVal));
                }
            }
        }

        // 3. Handle static method calls (e.g., MyType::staticFunc())
        // This is a special case where the base is a type name, not an instance.
        std::shared_ptr<IRValueType> staticTypeBase{};
        try {
            if (it + 1 != memberExpr->getTerms().end()) {
                staticTypeBase = managedPtr(parseTypeSpecExtern(*it, targetModule == -1 ? currentModuleIndex : targetModule));
            }
        } catch (std::runtime_error&) {
            // Not a type name, so it's an instance member expression.
        } catch (std::out_of_range&) {
            // Not a type name, so it's an instance member expression.
        }

        if (staticTypeBase && staticTypeBase->type == IRValueType::valueType::structObject) {
            auto memberNameNode = *(++it);
            yoi_assert(!memberNameNode->getSubscript().empty() && memberNameNode->getSubscript().front()->isInvocation(),
                    memberNameNode->getLine(), memberNameNode->getColumn(), "Static member access must be a method call.");
            
            auto& invocation = memberNameNode->getSubscript().front();
            if(!handleInvocationExtern(memberNameNode->id->getId().node.strVal, invocation->args, staticTypeBase->typeAffiliateModule, staticTypeBase, true))
                panic(memberNameNode->getLine(), memberNameNode->getColumn(), "No matching static method found for: " + wstring2string(memberNameNode->id->getId().get().strVal));
            // After a static call, subsequent member accesses operate on its return value.
        } else {
            // 3. It's an instance member expression. Visit the base instance.
            bool isFinalTerm = (it + 1 == memberExpr->getTerms().end());
            if (targetModule == -1) {
                visit(*it, isStoreOp && isFinalTerm);
            } else {
                visitExtern(*it, targetModule, isStoreOp && isFinalTerm);
            }
        }

        // 4. Loop through the rest of the terms (.b, .c(), .d[i], etc.)
        for (it ++; it != memberExpr->getTerms().end();it ++) {
            if (it == memberExpr->getTerms().end()) {
                break;
            }

            auto currentTermNode = *it;
            bool isFinalTerm = (std::next(it) == memberExpr->getTerms().end());

            // The type of the object we are operating on (result of the previous term)
            auto objectType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            // A. Handle the base of the current term (the identifier itself).
            // It's either a field access or a method name.
            if (currentTermNode->getSubscript().empty()) {
                // Case: Simple field access like `obj.field`
                if (objectType->isArrayType() || objectType->isDynamicArrayType()) {
                    if (currentTermNode->id->getId().get().strVal == L"length") {
                        moduleContext->getIRBuilder().arrayLengthOp();
                    } else {
                        panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Only 'length' member is valid on array types.");
                    }
                } else if (objectType->type == IRValueType::valueType::structObject) {
                    auto structDef = moduleContext->getCompilerContext()->getImportedModule(objectType->typeAffiliateModule)->structTable[objectType->typeIndex];
                    auto& memberName = currentTermNode->id->getId().get().strVal;
                    try {
                        auto nameInfo = structDef->lookupName(memberName);
                        if (nameInfo.type != IRStructDefinition::nameInfo::nameType::field) {
                            panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Cannot access method '" + wstring2string(memberName) + "' as a field.");
                        }
                        auto fieldType = structDef->fieldTypes[nameInfo.index];
                        
                        // A store operation can only happen on the very last term of the expression.
                        if (isStoreOp && isFinalTerm) {
                            moduleContext->getIRBuilder().restoreStateTemporarily();
                            tryCastTo(fieldType);
                            moduleContext->getIRBuilder().commitState();
                            moduleContext->getIRBuilder().storeMemberOp({IROperand::operandType::index, nameInfo.index});
                        } else {
                            moduleContext->getIRBuilder().loadMemberOp({IROperand::operandType::index, nameInfo.index}, fieldType);
                        }
                    } catch (std::out_of_range&) {
                        panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Struct '" + wstring2string(structDef->name) + "' has no field named '" + wstring2string(memberName) + "'.");
                    }
                } else {
                    panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Member access on a non-struct or non-array type is not allowed.");
                }
            } else {
                // Case: Field access followed by operations `obj.field[i]()`, or a method call `obj.method()`
                // We need to resolve if the identifier is a field or method. We prioritize method calls.
                auto firstOp = currentTermNode->getSubscript().front();
                bool isMethodCall = firstOp->isInvocation();

                if (isMethodCall) {
                    if (objectType->type == IRValueType::valueType::structObject) {
                        if(!handleInvocationExtern(currentTermNode->id->getId().get().strVal, firstOp->args, objectType->typeAffiliateModule, objectType))
                            panic(currentTermNode->getLine(), currentTermNode->getColumn(), "No matching method found for: " + wstring2string(currentTermNode->id->getId().get().strVal));
                    } else if (objectType->type == IRValueType::valueType::interfaceObject) {
                        // Interface method call logic... (was already correct)
                        if(!handleInvocationExtern(currentTermNode->id->getId().get().strVal, firstOp->args, objectType->typeAffiliateModule, objectType))
                            panic(currentTermNode->getLine(), currentTermNode->getColumn(), "No matching method found for: " + wstring2string(currentTermNode->id->getId().get().strVal));
                    }
                } else {
                    // It's a field access followed by subscript, e.g., `obj.data[i]`
                    // First, load the field itself.
                    if (objectType->type == IRValueType::valueType::structObject) {
                        auto structDef = moduleContext->getCompilerContext()->getImportedModule(objectType->typeAffiliateModule)->structTable[objectType->typeIndex];
                        auto& fieldName = currentTermNode->id->getId().get().strVal;
                        try {
                            auto nameInfo = structDef->lookupName(fieldName);
                            if (nameInfo.type != IRStructDefinition::nameInfo::nameType::field) {
                                panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Cannot subscript method '" + wstring2string(fieldName) + "'.");
                            }
                            auto fieldType = structDef->fieldTypes[nameInfo.index];
                            moduleContext->getIRBuilder().loadMemberOp({IROperand::operandType::index, nameInfo.index}, fieldType);
                        } catch (std::out_of_range&) {
                            panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Struct '" + wstring2string(structDef->name) + "' has no field named '" + wstring2string(fieldName) + "'.");
                        }
                    } else {
                        panic(currentTermNode->getLine(), currentTermNode->getColumn(), "Member access on a non-struct type is not allowed here.");
                    }
                }

                // B. Now, process the chain of `()` and `[]` that follow the identifier.
                auto subIt = isMethodCall ? std::next(currentTermNode->getSubscript().begin()) : currentTermNode->getSubscript().begin();
                for (; subIt != currentTermNode->getSubscript().end(); ++subIt) {
                    auto sub = *subIt;
                    bool isFinalOperation = isFinalTerm && (std::next(subIt) == currentTermNode->getSubscript().end());
                    auto currentObjectType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

                    if (sub->isInvocation()) {
                        // This handles `obj.field[i]()` where `field[i]` returns a callable.
                        // Or `obj.method()()` where `method()` returns a callable.
                        panic(sub->getLine(), sub->getColumn(), "Chained function calls are not yet supported in this context.");
                    } else if (sub->isSubscript()) {
                        if (currentObjectType->isArrayType() || currentObjectType->isDynamicArrayType()) {
                            visit(sub->expr); // Evaluate the index and push it.
                            tryCastTo(moduleContext->getCompilerContext()->getUnsignedObjectType());
                            auto indexType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                            yoi_assert(indexType->type == IRValueType::valueType::unsignedObject, sub->getLine(), sub->getColumn(), "Array/subscript index must be an integer or unsigned integer.");

                            if (isStoreOp && isFinalOperation) {
                                // This handles `... = obj.field[i]`
                                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_element, {});
                            } else {
                                // This handles `let x = obj.field[i]`
                                moduleContext->getIRBuilder().loadOp(IR::Opcode::load_element, {}, managedPtr(currentObjectType->getElementType()));
                            }
                        } else {
                            // Overloaded operator[]
                            if (isFinalOperation && isStoreOp) {
                                // current stack: [..., value, array, index]
                                auto value = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                                auto array = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                                visit(sub->expr);
                                auto index = moduleContext->getIRBuilder().getRhsFromTempVarStack();

                                OverloadResult overload;
                                if (array->type == IRValueType::valueType::structObject)
                                    overload = resolveOverloadExtern(L"operator[]",
                                                                    {value, array, index},
                                                                    array->typeAffiliateModule,
                                                                    moduleContext->getCompilerContext()
                                                                        ->getImportedModule(array->typeAffiliateModule)
                                                                        ->structTable[array->typeIndex]);
                                /*else if(array->type == IRValueType::valueType::interfaceObject)
                                    overload = resolveOverloadInterface(L"operator[]", {value, array, index},
                                array->typeAffiliateModule,
                                        moduleContext->getCompilerContext()->getImportedModule(array->typeAffiliateModule)->interfaceTable[array->typeIndex]);*/

                                yoi_assert(overload.found(),
                                        sub->getLine(),
                                        sub->getColumn(),
                                        "No matching overload found for operator[].");
                                yoi_assert(!overload.isVariadic,
                                        sub->getLine(),
                                        sub->getColumn(),
                                        "Variadic operator[] overloading is not supported.");

                                moduleContext->getIRBuilder().invokeMethodOp(
                                    overload.functionIndex, 2, overload.function->returnType, false, true, array->typeAffiliateModule);
                            } else {
                                moduleContext->getIRBuilder().saveState();
                                visit(sub->expr);
                                handleBinaryOperatorOverload(L"operator[]", sub->expr);
                            }
                        }
                    }
                }
            }
        }

        moduleContext->getIRBuilder().discardStateUntil(snapshotIndex);
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::visit(yoi::inCodeBlockStmt *inCodeBlockStmt) {
        moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, inCodeBlockStmt->getLine(), inCodeBlockStmt->getColumn()});

        if (!checkMarcoSatisfaction(inCodeBlockStmt->marco))
            return;

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
            generateNullInterfaceImplementation(managedPtr(baseType->getArrayType(dimensions)));
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
                if (irModule->funcTemplateAsts.contains(baseName)) {
                    auto astNode = irModule->funcTemplateAsts.at(baseName);
                    specializeFunctionTemplate(astNode, concreteTemplateArgs, currentModuleIndex);
                } else {
                    try {
                        auto baseType = managedPtr(parseTypeSpec(subscriptExpr->id));
                        switch (baseType->type) {
                            case IRValueType::valueType::structObject: baseName = moduleContext->getCompilerContext()->getImportedModule(baseType->typeAffiliateModule)->structTable.getKey(baseType->typeIndex); break;
                            case IRValueType::valueType::interfaceObject: baseName = moduleContext->getCompilerContext()->getImportedModule(baseType->typeAffiliateModule)->interfaceTable.getKey(baseType->typeIndex); break;
                            default: panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Cannot specialize type: except structObject or interfaceObject but got: " + yoi::wstring2string(baseType->to_string())); break;
                        }
                    } catch (const std::runtime_error &e) {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not resolve template specialization for function '" + wstring2string(baseName) + ": \n" + e.what());
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
                    moduleContext->getIRBuilder().discardState();
                } catch (const std::runtime_error &) {
                    moduleContext->getIRBuilder().restoreState();
                }
                if (isAccessible) {
                    auto structObject = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if(structObject->type == IRValueType::valueType::structObject || structObject->type == IRValueType::valueType::interfaceObject) {
                        if(!handleInvocationExtern(L"operator()", args, currentModuleIndex, structObject))
                            panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "No matching method found for: " + wstring2string(baseName));
                        resolved = true;
                    } else {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Cannot call operator() on non-struct object or interface object.");
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
                    panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not resolve constructor for struct '" + wstring2string(baseName) + "'.");
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
                        // moduleContext->getIRBuilder().newInterfaceOp(interfaceIndex);
                        auto interfaceImplName = getInterfaceImplName({currentModuleIndex, interfaceIndex}, argTypes[0]);
                        auto targetModule = moduleContext->getCompilerContext()->getImportedModule(argTypes[0]->typeAffiliateModule);
                        auto interfaceImplIndex = targetModule->interfaceImplementationTable.getIndex(interfaceImplName);
                        moduleContext->getIRBuilder().constructInterfaceImplOp({currentModuleIndex, interfaceIndex}, interfaceImplIndex, true, targetModule->identifier);
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
                        auto importedFunc =
                            moduleContext->getCompilerContext()
                                ->getIRFFITable()
                                ->importedLibraries[irModule->externTable[importedFunctionIndex]->affiliateModule]
                                .importedFunctionTable[irModule->externTable[importedFunctionIndex]->itemIndex];
                        
                        auto desiredArgTypes = importedFunc->argumentTypes;
                        yoi_assert(desiredArgTypes.size() == args->arg.size(), args->getLine(), args->getColumn(), "Number of arguments does not match the function signature.");
                        for (yoi::indexT i = 0;i < args->arg.size(); i++) {
                            visit(args->arg[i]);
                            tryCastTo(managedPtr(moduleContext->getCompilerContext()->normalizeForeignBasicType(desiredArgTypes[i], false)));
                        }
                        moduleContext->getIRBuilder().invokeImportedOp(
                            irModule->externTable[importedFunctionIndex]->affiliateModule,
                            irModule->externTable[importedFunctionIndex]->itemIndex,
                            desiredArgTypes.size(),
                            importedFunc->returnType);
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::out_of_range &) {
                        moduleContext->getIRBuilder().restoreState();
                    }
                }
            }

            // Attempt 5: type alias
            if (auto it = irModule->typeAliases.find(baseName); it != irModule->typeAliases.end()) {
                if (it->second.type == IRValueType::valueType::structObject) {
                    auto targetModule = moduleContext->getCompilerContext()->getImportedModule(it->second.typeAffiliateModule);
                    auto structIndex = it->second.typeIndex;
                    auto structType = targetModule->structTable[structIndex];
                    moduleContext->getIRBuilder().newStructOp(structIndex, true, targetModule->identifier);
                    auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                    if (handleInvocationExtern(L"constructor", args, targetModule->identifier, rhs)) {
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } else {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not resolve constructor for struct '" + wstring2string(baseName) + "'.");
                    }
                } else if (it->second.type == IRValueType::valueType::interfaceObject) {
                    auto targetModuleForInterface = moduleContext->getCompilerContext()->getImportedModule(it->second.typeAffiliateModule);
                    moduleContext->getIRBuilder().saveState();
                    try {
                        // auto interfaceIndex = targetModuleForInterface->interfaceTable.getIndex(baseName);
                        auto interfaceIndex = it->second.typeIndex;
                        auto argTypes = evaluateArguments(args);
                        yoi_assert(argTypes.size() == 1, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Interface constructor expects exactly one argument.");
                        // moduleContext->getIRBuilder().newInterfaceOp(interfaceIndex, true, targetModuleForInterface->identifier);
                        auto interfaceImplName = getInterfaceImplName({currentModuleIndex, interfaceIndex}, argTypes[0]);
                        auto targetModule = moduleContext->getCompilerContext()->getImportedModule(argTypes[0]->typeAffiliateModule);
                        auto interfaceImplIndex = targetModule->interfaceImplementationTable.getIndex(interfaceImplName);
                        moduleContext->getIRBuilder().constructInterfaceImplOp({currentModuleIndex, interfaceIndex}, interfaceImplIndex, true, targetModule->identifier);
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::exception &) {
                        moduleContext->getIRBuilder().restoreState();
                    }
                } else {
                    panic(first_term->getLine(), first_term->getColumn(), "invalid type alias type");
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
                if (handleSubscript(it, end, isStoreOp, isLastTerm)) continue;
            } else if (currentTerm->isInvocation()) {
                if(objectOnStackType->type == IRValueType::valueType::structObject || objectOnStackType->type == IRValueType::valueType::interfaceObject) {
                    if(!handleInvocationExtern(L"operator()", currentTerm->args, currentModuleIndex, objectOnStackType))
                        panic(currentTerm->getLine(), currentTerm->getColumn(), "No matching method found for: " + wstring2string(subscriptExpr->id->getId().get().strVal));
                } else {
                    panic(currentTerm->getLine(), currentTerm->getColumn(), "Cannot call operator() on non-struct object or interface object.");
                }
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
        } catch (const std::out_of_range &) {
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

            generateNullInterfaceImplementation(managedPtr(baseType->getArrayType(dimensions)));
            moduleContext->getIRBuilder().newArrayOp(baseType, dimensions);
        } 
        // Case 2: Extern Invocation
        else if (firstTerm->isInvocation()) {
            firstTermHandled = true;
            auto baseName = subscriptExpr->id->getId().get().strVal;
            auto args = firstTerm->args;
            bool resolved = false;

            // --- Step 1: Handle explicit template specialization if present ---
            if (subscriptExpr->id->hasTemplateArg()) {
                auto concreteTemplateArgs = parseTemplateArgs(subscriptExpr->id->getArg());
                if (targetedModule->funcTemplateAsts.contains(baseName)) {
                    auto astNode = targetedModule->funcTemplateAsts.at(baseName);
                    specializeFunctionTemplate(astNode, concreteTemplateArgs, targetModule);
                } else {
                    try {
                        auto baseType = managedPtr(parseTypeSpecExtern(subscriptExpr->id, targetModule));
                        switch (baseType->type) {
                            case IRValueType::valueType::structObject: baseName = moduleContext->getCompilerContext()->getImportedModule(baseType->typeAffiliateModule)->structTable.getKey(baseType->typeIndex); break;
                            case IRValueType::valueType::interfaceObject: baseName = moduleContext->getCompilerContext()->getImportedModule(baseType->typeAffiliateModule)->interfaceTable.getKey(baseType->typeIndex); break;
                            default: panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Cannot specialize type: except structObject or interfaceObject but got: " + yoi::wstring2string(baseType->to_string())); break;
                        }
                    } catch (const std::runtime_error &e) {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not resolve template specialization for function '" + wstring2string(baseName) + "'" + ": \n" + e.what() + "\n");
                    }
                }
            }

            // Attempt 1: Extern Struct Constructor (regular or variadic)
            if (targetedModule->structTable.contains(baseName)) {
                auto externStructEntry = getExternEntry(targetModule, baseName);
                auto targetedStruct = targetedModule->structTable[baseName];
                
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
                resolved = handleInvocationExtern(baseName, args, targetModule);
            }

            // Attempt 3: Extern Interface Constructor
            if (!resolved) {
                if (targetedModule->interfaceTable.contains(baseName)) {
                    moduleContext->getIRBuilder().saveState();
                    try {
                        auto argTypes = evaluateArguments(args);
                        yoi_assert(argTypes.size() == 1, subscriptExpr->getLine(), subscriptExpr->getColumn(), "Interface constructor expects exactly one argument.");

                        auto concreteThis = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                        
                        auto externInterface = getExternEntry(targetModule, baseName);
                        // moduleContext->getIRBuilder().newInterfaceOp(externInterface.itemIndex, true, externInterface.affiliateModule);

                        auto interfaceImplName = getInterfaceImplName({externInterface.affiliateModule, externInterface.itemIndex}, argTypes[0]);
                        auto interfaceImplIndex = moduleContext->getCompilerContext()->getImportedModule(concreteThis->typeAffiliateModule)->interfaceImplementationTable.getIndex(interfaceImplName);
                        moduleContext->getIRBuilder().constructInterfaceImplOp({externInterface.affiliateModule, externInterface.itemIndex}, interfaceImplIndex, concreteThis->typeAffiliateModule != currentModuleIndex, concreteThis->typeAffiliateModule);
                        
                        resolved = true;
                        moduleContext->getIRBuilder().discardState();
                    } catch (const std::out_of_range &e) {
                        panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not find matched extern interface constructor for " + wstring2string(baseName));
                    }/* catch (const std::exception &e) {
                        // panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not find matched extern interface constructor for " + wstring2string(baseName));
                        throw e;
                    }*/
                }
            }

            // Attempt 4: FFI Imported Function (via an extern module)
            if (!resolved) {
                if (targetedModule->externTable.contains(baseName)) {
                    moduleContext->getIRBuilder().saveState();
                    try {
                        auto argTypes = evaluateArguments(args);
                        auto externEntry = targetedModule->externTable[baseName];
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
                panic(subscriptExpr->getLine(), subscriptExpr->getColumn(), "Could not find extern function, struct constructor, or interface in module: " + wstring2string(baseName));
            }
        } else { // First term is a variable access
            visitExtern(subscriptExpr->id, targetModule, false);
        }

        if (firstTermHandled) {
            it++;
        }

        // --- Loop for subsequent terms ---
        while (it != end) {
            auto currentTerm = *it;
            auto objectOnStackType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

            if (currentTerm->isSubscript()) {
                panic(currentTerm->getLine(),
                    currentTerm->getColumn(),
                    "TODO: Extern array access is not fully implemented yet.");
            } else if (currentTerm->isInvocation()) {
                if(objectOnStackType->type == IRValueType::valueType::structObject || objectOnStackType->type == IRValueType::valueType::interfaceObject) {
                    handleInvocationExtern(L"operator()", currentTerm->args, currentModuleIndex, objectOnStackType);
                } else {
                    panic(currentTerm->getLine(), currentTerm->getColumn(), "Cannot call operator() on non-struct object or interface object.");
                }
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
            return irModule->typeAliases.at(typeName);
        } catch (std::out_of_range &e) {
            // let it go
        }
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
        } else if (typeName == L"ptr") {
            return *moduleContext->getCompilerContext()->getPointerType();
        } else if (typeName == L"unsigned") {
            return *moduleContext->getCompilerContext()->getUnsignedObjectType();
        } else if (typeName == L"short") {
            return *moduleContext->getCompilerContext()->getShortObjectType();
        } else {
            panic(identifier->getLine(), identifier->getColumn(), "Unsupported type: " + wstring2string(typeName));
        }
    }

    yoi::indexT visitor::visit(yoi::funcDefStmt *funcDefStmt) {
        auto funcName = funcDefStmt->getId();

        if (funcName.hasDefTemplateArg()) {
            auto actualName = funcName.getId().node.strVal;
            irModule->funcTemplateAsts[actualName] = funcDefStmt;
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
                    auto argType = managedPtr(
                        i->spec->elipsis
                        ? parseTypeSpec(i->spec->elipsis).getDynamicArrayType()
                        : moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType()
                    );
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

            auto funcIndex = irModule->functionTable.put(builder.name, func);
            irModule->functionOverloadIndexies[funcName.getId().node.strVal].push_back(funcIndex);

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

            irModule->templateInterfaceAsts[interfaceName] = interfaceDefStmt;

            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

        auto &interfaceName = interfaceDefStmt->id->getId().get().strVal;
        auto interfaceIndex = irModule->interfaceTable.put(interfaceName, {});

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
                    auto argType = managedPtr(
                        arg->spec->elipsis
                        ? parseTypeSpec(arg->spec->elipsis).getDynamicArrayType()
                        : moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType()
                    );
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
        irModule->interfaceTable[interfaceIndex] = interfaceType;
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::structDefStmt *structDefStmt) {
        auto &structName = structDefStmt->id->getId().get().strVal;

        if (structDefStmt->getId().hasDefTemplateArg()) {
            irModule->structTemplateAsts[structName] = structDefStmt;
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
                        constructorBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Constructor);

                        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                        auto thisType = managedPtr(
                            IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                        constructorBuilder.setReturnType(thisType);
                        constructorBuilder.addArgument(L"this", thisType);
                        constructorBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Constructor);
                        for (auto &arg : i->getConstructor().getArgs().get()) {
                            if (&arg == &i->getConstructor().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                                isVaridic = true;
                                constructorBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                                auto argName = arg->getId().node.strVal;
                                auto argType = managedPtr(
                                    arg->spec->elipsis
                                    ? parseTypeSpec(arg->spec->elipsis).getDynamicArrayType()
                                    : moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType()
                                );
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
                        auto funcIndex = irModule->functionTable.put_create(func->name, func);
                        builder.addMethod(mangledName, funcIndex);
                        irModule->functionOverloadIndexies[structName + L"::constructor"].push_back(funcIndex);
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
                                auto argType = managedPtr(
                                    arg->spec->elipsis
                                    ? parseTypeSpec(arg->spec->elipsis).getDynamicArrayType()
                                    : moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType()
                                );
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
                        auto funcIndex = irModule->functionTable.put_create(func->name, func);
                        builder.addMethod(mangledName, funcIndex);
                        irModule->functionOverloadIndexies[structName + L"::" + methodName].push_back(funcIndex);
                        break;
                    }
                    case 3: {
                        // finalizer
                        IRFunctionDefinition::Builder finalizerBuilder;
                        finalizerBuilder.setName(structName + L"::finalizer");
                        finalizerBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
                        finalizerBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Finalizer);
                        finalizerBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Preserve);
                        auto thisType = managedPtr(
                            IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});
                        finalizerBuilder.setReturnType(moduleContext->getCompilerContext()->getNoneObjectType());
                        finalizerBuilder.addArgument(L"this", thisType);
                        auto func = finalizerBuilder.yield();
                        auto funcIndex = irModule->functionTable.put_create(structName + L"::finalizer", func);
                        builder.addMethod(L"finalizer", funcIndex);
                        // no need to prepare for manual calling, it is not legal.
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

        auto it = structIdNode.getTerms().begin();
        yoi::indexT targetModule = -1, lastModule = -1;
        while (it + 1 != structIdNode.getTerms().end() && (targetModule = isModuleName(*it, targetModule)) != lastModule) {
            it++;
            lastModule = targetModule;
        }
        if (targetModule == -1) {
            targetModule = currentModuleIndex;
        }
        yoi_assert(it + 1 == structIdNode.getTerms().end(),
                   structIdNode.getLine(),
                   structIdNode.getColumn(),
                   "Invalid interface name");

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);
        auto structBaseName = (*it)->getId().get().strVal;

        if ((*it)->hasTemplateArg()) {
            // Impl for a template struct
            yoi_assert(targetedModule->structTemplateAsts.contains(structBaseName),
                       implStmt->getLine(),
                       implStmt->getColumn(),
                       "Impl for undefined struct template: " + wstring2string(structBaseName));

            yoi::vec<std::shared_ptr<IRValueType>> concreteTemplateArgs = parseTemplateArgs((*it)->getArg());

            if (implStmt->isImplForStmt()) {
                // interface implementation for a template struct
                if (concreteTemplateArgs.empty()) {
                    targetedModule->templateInterfaceImplAsts[structBaseName].push_back(implStmt);
                } else {
                    // specialize template interface
                    auto interfaceTemplateAst = targetedModule->templateInterfaceAsts[structBaseName];
                    auto concreteStructName = getMangledTemplateName(structBaseName, concreteTemplateArgs);
                    auto concreteStructType = managedPtr(parseTypeSpec(implStmt->structName)); // quick specialization check
                    yoi_assert(concreteStructType->type == IRValueType::valueType::structObject, implStmt->getLine(), implStmt->getColumn(), "Invalid struct name for struct specialization: " + wstring2string(concreteStructName) + " (except structObject but got " + wstring2string(concreteStructType->to_string()) + ")");                    
                    // FIXED: module index
                    specializeInterfaceImplementation(implStmt, concreteStructType, concreteStructName, concreteTemplateArgs, targetModule);
                }
            } else {
                if (concreteTemplateArgs.empty()) {
                    // pure template struct, store them and specialize when used
                    targetedModule->templateImplAsts[structBaseName] = implStmt;
                } else {
                    // specialize template struct
                    auto structTemplateAst = targetedModule->structTemplateAsts[structBaseName];
                    specializeStructTemplate(structBaseName, concreteTemplateArgs, implStmt, targetModule);
                }
            }

            
            return moduleContext->getIRBuilder().getCurrentInsertionPoint();
        }

        if (implStmt->isImplForStmt()) {
            auto interfaceName = parseInterfaceName(implStmt->interfaceName);
            std::shared_ptr<IRValueType> srcType;
            try {
                srcType = managedPtr(parseTypeSpec(implStmt->structName));
                targetModule = srcType->typeAffiliateModule;
                targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);
            } catch (std::runtime_error &e) {
                panic(
                    implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(structBaseName));
                return {};
            }
            auto targetInterface = moduleContext->getCompilerContext()->getImportedModule(interfaceName.first.first)->interfaceTable[interfaceName.first.second];
            targetInterface->implementations.emplace_back(
                srcType->type, srcType->typeAffiliateModule, srcType->typeIndex
            );
            auto interfaceImplName =
                getInterfaceImplName(interfaceName.first, srcType);

            
            yoi::indexT interfaceImplIndex{};
            try {
                interfaceImplIndex = targetedModule->interfaceImplementationTable.getIndex(interfaceImplName);
                if (targetedModule->interfaceImplementationTable[interfaceImplIndex]) {
                    panic(
                        implStmt->getLine(),
                        implStmt->getColumn(),
                        "Redefinition of interface implementation: " + wstring2string(interfaceImplName));
                }
            } catch (std::out_of_range &e) {
                interfaceImplIndex = targetedModule->interfaceImplementationTable.put(interfaceImplName, {});
            }
            if (implStmt->inner) {
                IRInterfaceImplementationDefinition::Builder builder;
                builder.setName(interfaceImplName);
                builder.setImplStructIndex({srcType->type, srcType->typeAffiliateModule, srcType->typeIndex});
                builder.setImplInterfaceIndex(interfaceName.first);

                std::map<yoi::wstr, std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> virtualMethodMap;

                for (auto &i : implStmt->getInner().getInner()) {
                    yoi_assert(i->isMethod(),
                            i->getLine(),
                            i->getColumn(),
                            "impl-for statement only allows method definition, not constructor or finalizer");

                    bool isVaridic = false;

                    auto methodName = i->getMethod().getName().get().strVal;
                    IRFunctionDefinition::Builder methodBuilder;

                    methodBuilder.setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
                    methodBuilder.attrs = getFunctionAttributes(i->getMethod().attrs);
                    methodBuilder.attrs.emplace_back(IRFunctionDefinition::FunctionAttrs::Preserve);
                    methodBuilder.attrs.emplace_back(IRFunctionDefinition::FunctionAttrs::NoRawAndNullOptimization);

                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;

                    const auto& thisType = srcType;
                    if (std::find(methodBuilder.attrs.begin(), methodBuilder.attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) == methodBuilder.attrs.end())
                        methodBuilder.addArgument(L"this", thisType);

                    for (auto &arg : i->getMethod().getArgs().get()) {
                        if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                            isVaridic = true;
                            methodBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Variadic);
                            auto argName = arg->getId().node.strVal;
                            auto argType = managedPtr(
                                arg->spec->elipsis
                                    ? parseTypeSpec(arg->spec->elipsis).getDynamicArrayType()
                                    : moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType()
                            );
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
                    methodBuilder.setName(structBaseName + L"::" + methodName + uniq + L"interfaceImpl#" + interfaceImplName);

                    auto func = methodBuilder.yield();
                    auto funcIndex = targetedModule->functionTable.put_create(func->name, func);
                    targetedModule->functionOverloadIndexies[structBaseName + L"::" + methodName + L"interfaceImpl#" + interfaceImplName].push_back(funcIndex);
                    /*builder.addVirtualMethod(
                        methodName + uniq,
                        managedPtr(IRValueType{IRValueType::valueType::virtualMethod, targetModule, funcIndex}));*/
                    virtualMethodMap[methodName + getFuncUniqueNameStr(argTypes, true)] = {methodName + uniq, managedPtr(IRValueType{IRValueType::valueType::virtualMethod, targetModule, funcIndex})};

                    moduleContext->pushIRBuilder(IRBuilder{moduleContext->getCompilerContext(), irModule, func});
                    moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
                    moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                    visit(i->getMethod().block, true);
                    moduleContext->getIRBuilder().yield();
                    moduleContext->popIRBuilder();
                }

                for (auto &method: targetInterface->methodMap) {
                    yoi_assert(virtualMethodMap.contains(method.first), implStmt->getLine(), implStmt->getColumn(), "Method '" + wstring2string(method.first) + "' not implemented for interface '" + wstring2string(targetInterface->name) + "'");
                    builder.addVirtualMethod(virtualMethodMap[method.first].first, virtualMethodMap[method.first].second);
                }
                
                targetedModule->interfaceImplementationTable[interfaceImplIndex] = builder.yield();
            } else {
                // forward-declaration
            }
        } else {
            indexT structIndex;
            try {
                structIndex = targetedModule->structTable.getIndex(structBaseName);
            } catch (std::runtime_error &e) {
                panic(
                    implStmt->getLine(), implStmt->getColumn(), "Undefined struct: " + wstring2string(structBaseName));
            }

            for (auto &i : implStmt->getInner().getInner()) {
                yoi::wstr mangledName;
                yoi::codeBlock *block{};
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
                    block = i->getConstructor().block;
                } else if (i->isFinalizer()) {
                    mangledName = structBaseName + L"::finalizer";
                    block = i->getFinalizer().block;
                } else {
                    // whole bunch of shit doin' here is to get the mangled name of the method, no actual modification of original func def here.
                    bool isVaridic = false;
                    yoi::vec<std::shared_ptr<IRValueType>> argTypes;
                    for (auto &arg : i->getMethod().getArgs().get()) {
                        if (&arg == &i->getMethod().getArgs().get().back() && arg->spec->kind == 3 /* elipsis */) {
                            isVaridic = true;
                            auto type = arg->spec->elipsis ? parseTypeSpec(arg->spec->elipsis) : *moduleContext->getCompilerContext()->getNullInterfaceType();
                            auto argType = managedPtr(type.getDynamicArrayType());
                            argTypes.push_back(argType);
                            break;
                        }
                        argTypes.push_back(managedPtr(parseTypeSpec(arg->spec)));
                    }
                    mangledName =
                        structBaseName + L"::" + i->getMethod().getName().get().strVal + getFuncUniqueNameStr(argTypes);
                    block = i->getMethod().block;
                }

                try {
                    auto funcIndex = targetedModule->functionTable.getIndex(mangledName);
                    auto func = targetedModule->functionTable[funcIndex];
                    moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
                    moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, i->getLine(), i->getColumn()});
                    moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                    visit(block, true);
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
            auto type = i->type ? managedPtr(parseTypeSpec(i->type)) : moduleContext->getIRBuilder().getRhsFromTempVarStack();
            if (i->lhs->kind == letAssignmentPairLHS::vKind::identifier) {
                if (isVisitingGlobalScope()) {
                    // global variable
                    tryCastTo(type);
                    auto index = irModule->globalVariables.put(i->lhs->id->node.strVal, type);
                    moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global,
                                                          {IROperand::operandType::globalVar, index});

                } else {
                    tryCastTo(type);
                    auto index =
                        moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().put(i->lhs->id->node.strVal, type);
                    moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local,
                                                          {IROperand::operandType::localVar, index});
                }
            } else if (i->lhs->kind == letAssignmentPairLHS::vKind::list) {
                // structured binding
                if (type->isArrayType() || type->isDynamicArrayType()) {
                    // array binding
                    IRBuilder::ExtractType extractType{i->lhs->list.back().kind == lexer::token::tokenKind::kThreeDots ? IRBuilder::ExtractType::First : IRBuilder::ExtractType::Last};
                    bool isFull = i->lhs->list.back().kind != lexer::token::tokenKind::kThreeDots && i->lhs->list.front().kind != lexer::token::tokenKind::kThreeDots;
                    auto elementCount = i->lhs->list.size() - !isFull;
                    moduleContext->getIRBuilder().bindElementsOp(elementCount, extractType);
                    auto bindType = managedPtr(type->getElementType());
                    for (yoi::indexT curPos = extractType == IRBuilder::ExtractType::First && !isFull;curPos < elementCount; curPos++) {
                        if (isVisitingGlobalScope()) {
                            // global variable
                            auto index = irModule->globalVariables.put(i->lhs->list[i->lhs->list.size() - 1 - curPos].strVal, bindType);
                            moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global,
                                                                  {IROperand::operandType::globalVar, index});

                        } else {
                            auto index =
                                moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().put(i->lhs->list[i->lhs->list.size() - 1 - curPos].strVal, bindType);
                            moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local,
                                                                  {IROperand::operandType::localVar, index});
                        }
                    }
                } else if (type->type == IRValueType::valueType::structObject) {
                    // struct binding
                    IRBuilder::ExtractType extractType{i->lhs->list.back().kind == lexer::token::tokenKind::kThreeDots ? IRBuilder::ExtractType::First : IRBuilder::ExtractType::Last};
                    bool isFull = i->lhs->list.back().kind != lexer::token::tokenKind::kThreeDots && i->lhs->list.front().kind != lexer::token::tokenKind::kThreeDots;
                    yoi::indexT elementCount = i->lhs->list.size() - !isFull;
                    yoi::indexT startPos = extractType == IRBuilder::ExtractType::First ? elementCount - 1 : i->lhs->list.size() - 1;
                    yoi::indexT endPos = extractType == IRBuilder::ExtractType::First ? -1 : 0 - isFull;
                    moduleContext->getIRBuilder().bindFieldsOp(elementCount, extractType);
                    for (yoi::indexT curPos = startPos; curPos != endPos; curPos -= 1) {
                        auto fieldType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                        if (isVisitingGlobalScope()) {
                            // global variable
                            auto index = irModule->globalVariables.put(i->lhs->list[curPos].strVal, fieldType);
                            moduleContext->getIRBuilder().storeOp(IR::Opcode::store_global,
                                                                  {IROperand::operandType::globalVar, index});

                        } else {
                            auto index =
                                moduleContext->getIRBuilder().irFuncDefinition()->getVariableTable().put(i->lhs->list[curPos].strVal, fieldType);
                            moduleContext->getIRBuilder().storeOp(IR::Opcode::store_local,
                                                                  {IROperand::operandType::localVar, index});
                        }
                    }
                } else {
                    panic(i->getLine(), i->getColumn(), "Unsupported structured binding");
                }
            }
        }
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    void visitor::visit(yoi::globalStmt *globalStmt) {
        if (!checkMarcoSatisfaction(globalStmt->marco))
            return;
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
            case globalStmt::vKind::typeAliasStmt: {
                visit(globalStmt->value.typeAliasStmtVal);
                break;
            }
            case globalStmt::vKind::enumerationDef: {
                visit(globalStmt->value.enumerationDefVal);
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
        moduleContext->getIRBuilder().pushLoopContext(outBlock, condBlock);
        visit(whileStmt->block, true);
        moduleContext->getIRBuilder().popLoopContext();

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
        moduleContext->getIRBuilder().pushLoopContext(outBlock, condBlock);
        visit(forStmt->block, true);
        moduleContext->getIRBuilder().popLoopContext();

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
        moduleContext->getIRBuilder().continueOp();
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::visit(yoi::breakStmt *breakStmt) {
        moduleContext->getIRBuilder().breakOp();
        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    IRValueType visitor::parseTypeSpec(yoi::identifierWithTemplateArg *identifierWithTemplateArg) {
        if (identifierWithTemplateArg->hasTemplateArg()) {
            auto baseName = identifierWithTemplateArg->getId().node.strVal;
            auto concreteTypes = parseTemplateArgs(identifierWithTemplateArg->getArg());

            if (irModule->structTemplateAsts.contains(baseName)) {
                try {
                    auto pureTemplateAst = irModule->templateImplAsts.at(baseName);
                    auto specializedIndex = specializeStructTemplate(baseName, concreteTypes, pureTemplateAst, currentModuleIndex);
                    return {IRValueType::valueType::structObject, currentModuleIndex, specializedIndex};
                } catch (std::out_of_range &e) {
                    panic(identifierWithTemplateArg->getLine(),
                        identifierWithTemplateArg->getColumn(),
                        "No implementation block found for struct template: " + wstring2string(baseName));
                }
            } else if (irModule->templateInterfaceAsts.contains(baseName)) {
                auto specializedIndex = specializeInterfaceTemplate(baseName, concreteTypes, currentModuleIndex);
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
        if (auto it = mod->typeAliases.find(identifier->get().strVal); it != mod->typeAliases.end()) {
            return it->second;
        }
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
            auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);

            auto baseName = identifierWithTemplateArg->getId().node.strVal;
            auto concreteTypes = parseTemplateArgs(identifierWithTemplateArg->getArg());

            if (targetedModule->structTemplateAsts.contains(baseName)) {
                try {
                    auto pureTemplateAst = targetedModule->templateImplAsts.at(baseName);
                    auto specializedIndex = specializeStructTemplate(baseName, concreteTypes, pureTemplateAst, targetModule);
                    return {IRValueType::valueType::structObject, targetModule, specializedIndex};
                } catch (std::out_of_range &e) {
                    panic(identifierWithTemplateArg->getLine(),
                        identifierWithTemplateArg->getColumn(),
                        "No implementation block found for struct template: " + wstring2string(baseName));
                }
            } else if (targetedModule->templateInterfaceAsts.contains(baseName)) {
                auto specializedIndex = specializeInterfaceTemplate(baseName, concreteTypes, targetModule);
                return {IRValueType::valueType::interfaceObject, targetModule, specializedIndex};
            } else {
                panic(identifierWithTemplateArg->getLine(),
                    identifierWithTemplateArg->getColumn(),
                    "Unknown template: " + wstring2string(baseName));
            }
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
                return parseTypeSpec(typeSpec->func);
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
        auto &lhsType = moduleContext->getIRBuilder().getLhsFromTempVarStack();
        auto &rhsType = moduleContext->getIRBuilder().getRhsFromTempVarStack();

        if (lhsType->isForeignBasicType()) {
            lhsType = managedPtr(lhsType->getNormalizedForeignBasicType());
        }
        if (rhsType->isForeignBasicType()) {
            rhsType = managedPtr(rhsType->getNormalizedForeignBasicType());
        }

        // if bool and char with other, upcast to other
        if (lhsType->is1ByteType() && !rhsType->is1ByteType()) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        } else if (!lhsType->is1ByteType() && rhsType->is1ByteType()) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        }
        // if short with other, upcast to other
        if (lhsType->type == IRValueType::valueType::shortObject && rhsType->type != IRValueType::valueType::shortObject) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        } else if (lhsType->type != IRValueType::valueType::shortObject && rhsType->type == IRValueType::valueType::shortObject) {
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
        // if unsigned with int, upcast to int
        else if (lhsType->type == IRValueType::valueType::unsignedObject && rhsType->type == IRValueType::valueType::integerObject) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        } else if (lhsType->type == IRValueType::valueType::integerObject &&
                   rhsType->type == IRValueType::valueType::unsignedObject) {
            moduleContext->getIRBuilder().basicCast(lhsType, rhs);
        }
        // if unsigned with deci, upcast to deci
        else if (lhsType->type == IRValueType::valueType::unsignedObject &&
                 rhsType->type == IRValueType::valueType::decimalObject) {
            moduleContext->getIRBuilder().basicCast(rhsType, lhs, true);
        } else if (lhsType->type == IRValueType::valueType::decimalObject &&
                       rhsType->type == IRValueType::valueType::unsignedObject) {
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
            case IRValueType::valueType::characterObject:
                res = L"char";
                break;
            case IRValueType::valueType::shortObject:
                res = L"short";
                break;
            case IRValueType::valueType::unsignedObject:
                res = L"unsigned";
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
            case IRValueType::valueType::pointerObject: 
                res = L"pointerObject";
                break;
            case IRValueType::valueType::none:
                res = L"none";
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
    visitor::specializeFunctionTemplate(yoi::funcDefStmt *astNode,
                                        const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                        yoi::indexT moduleIndex) {
        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);

        yoi::wstr specializedName =
            getMangledTemplateName(astNode->id->id->get().strVal, concreteTemplateArgs);

        if (targetedModule->functionTable.contains(specializedName)) {
            return targetedModule->functionTable.getIndex(specializedName);
        }

        // Create a new function definition by specializing the template
        IRFunctionDefinition::Builder builder;

        builder.setDebugInfo({targetedModule->modulePath, astNode->getLine(), astNode->getColumn()});

        // Create specialization context
        IRTemplateBuilder specializationContext;
        for (yoi::indexT i = 0; i < astNode->id->arg->get().size(); ++i) {
            auto paramName = astNode->id->arg->get()[i]->id->get().strVal;
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }

        
        pushModuleContext(moduleIndex);
        moduleContext->pushTemplateBuilder(specializationContext);

        // Specialize arguments and return type
        yoi::vec<std::shared_ptr<IRValueType>> paramTypes;
        for (const auto &argPair : astNode->getArgs().get()) {
            auto argName = argPair->getId().get().strVal;
            auto argType = managedPtr(parseTypeSpec(&argPair->getSpec()));
            builder.addArgument(argName, argType);
            paramTypes.push_back(argType);
        }
        builder.setName(specializedName + getFuncUniqueNameStr(paramTypes));
        builder.setReturnType(managedPtr(parseTypeSpec(&astNode->getResultType())));

        auto specializedFunc = builder.yield();
        auto funcIndex = targetedModule->functionTable.put_create(specializedName + getFuncUniqueNameStr(paramTypes), specializedFunc);
        targetedModule->functionOverloadIndexies[specializedName].push_back(funcIndex);

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

        auto structAst = targetedModule->structTemplateAsts.at(templateName);

        IRTemplateBuilder specializationContext;
        yoi_assert(concreteTemplateArgs.size() == structAst->id->getArg().get().size(),
                0, 0, "Template argument count mismatch for struct " + wstring2string(templateName));
        for (yoi::indexT i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structAst->id->getArg().get()[i]->getId().get().strVal;
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }

        auto specializedStructIndex = targetedModule->structTable.put_create(specializedName, nullptr);

        generateNullInterfaceImplementation(managedPtr(IRValueType{IRValueType::valueType::structObject, moduleIndex, specializedStructIndex}));
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject, moduleIndex, specializedStructIndex});
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
            } else {
                auto [funcIndex, funcName] = specializeStructMethodDeclaration(specializationContext, field, specializedName, concreteTemplateArgs, moduleIndex);
                builder.addMethod(funcName, funcIndex);
            }
        }
        
        auto specializedStruct = builder.yield();
        targetedModule->structTable[specializedStructIndex] = specializedStruct;

        // Specialize methods defined in `impl MyStruct<T> { ... }`
        if (pureTemplateImplAst) {
            for (auto &methodAst : pureTemplateImplAst->getInner().getInner()) {
                specializeStructMethodDefinition(specializationContext, specializedStruct, methodAst, specializedName, concreteTemplateArgs, moduleIndex);
            }
        }

        if (targetedModule->templateInterfaceImplAsts.count(templateName)) {
            for (auto& implAst : targetedModule->templateInterfaceImplAsts.at(templateName)) {
                specializeInterfaceImplementation(implAst, selfType, specializedName, concreteTemplateArgs, currentModuleIndex); // now we are in the specialized context
            }
        }

        moduleContext->popTemplateBuilder();
        popModuleContext();

        return specializedStructIndex;
    }

    std::pair<yoi::indexT, yoi::wstr> visitor::specializeStructMethodDeclaration(IRTemplateBuilder &structTemplate,
                                         yoi::structDefInnerPair *methodAstNode,
                                         const yoi::wstr &specializedStructName,
                                         const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                         yoi::indexT moduleIndex) {

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);

        // Push the template's own context to resolve generic types like 'T' to their placeholder
        // 'incompleteTemplateType'.
        IRTemplateBuilder genericContext;
        genericContext.templateArguments = structTemplate.templateArguments;

        moduleContext->pushTemplateBuilder(genericContext);

        yoi::wstr baseMethodName;
        yoi::wstr genericMethodKey;
        yoi::vec<std::shared_ptr<IRValueType>> genericArgTypes;

        if (methodAstNode->kind == 1) {
            baseMethodName = L"constructor";
            for (auto &arg : methodAstNode->getConstructor().getArgs().get()) {
                genericArgTypes.push_back(managedPtr(parseTypeSpec(&arg->getSpec())));
            }
            genericMethodKey = baseMethodName + getFuncUniqueNameStr(genericArgTypes);
        } else if (methodAstNode->kind == 2) {
            baseMethodName = methodAstNode->getMethod().getName().get().strVal;
            for (auto &arg : methodAstNode->getMethod().getArgs().get()) {
                genericArgTypes.push_back(managedPtr(parseTypeSpec(&arg->getSpec())));
            }
            genericMethodKey = baseMethodName + getFuncUniqueNameStr(genericArgTypes);
        } else if (methodAstNode->kind == 3) {
            // finalizer
            baseMethodName = L"finalizer";
            genericMethodKey = baseMethodName;
        }

        moduleContext->popTemplateBuilder(); // Done with generic context

        IRTemplateBuilder specializationContext;
        for (size_t i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structTemplate.templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject,
                                               moduleIndex,
                                               targetedModule->structTable.getIndex(specializedStructName)});
        specializationContext.addTemplateArgument(L"STRUCT", selfType);
        moduleContext->pushTemplateBuilder(specializationContext);

        IRFunctionDefinition::Builder funcBuilder;

        funcBuilder.setDebugInfo({targetedModule->modulePath, methodAstNode->getLine(), methodAstNode->getColumn()});

        yoi::vec<std::shared_ptr<IRValueType>> specializedArgTypes;

        if (methodAstNode->kind == 1) {
            funcBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Constructor);
            funcBuilder.addArgument(L"this", selfType); // Specialized 'this'
            for (auto &arg : methodAstNode->getConstructor().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                funcBuilder.addArgument(arg->getId().get().strVal, specializedType);
                specializedArgTypes.push_back(specializedType);
            }
            funcBuilder.setReturnType(selfType);
        } else if (methodAstNode->kind == 2) {
            funcBuilder.attrs = getFunctionAttributes(methodAstNode->getMethod().attrs);
            if (std::find(funcBuilder.attrs.begin(), funcBuilder.attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) == funcBuilder.attrs.end()) {
                funcBuilder.addArgument(L"this", selfType); // Specialized 'this'
            }
            for (auto &arg : methodAstNode->getMethod().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                funcBuilder.addArgument(arg->getId().get().strVal, specializedType);
                specializedArgTypes.push_back(specializedType);
            }
            funcBuilder.setReturnType(managedPtr(parseTypeSpec(&methodAstNode->getMethod().getResultType())));
        } else if (methodAstNode->kind == 3) {
            funcBuilder.setName(specializedStructName + L"::finalizer");
            funcBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Finalizer);
            funcBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Preserve);
            funcBuilder.addArgument(L"this", selfType); // Specialized 'this'
            funcBuilder.setReturnType(moduleContext->getCompilerContext()->getNoneObjectType());
        }

        yoi::wstr specializedMethodName =
            specializedStructName + L"::" + baseMethodName;
        funcBuilder.setName(specializedMethodName + getFuncUniqueNameStr(specializedArgTypes));

        auto specializedFunc = funcBuilder.yield();
        auto funcIndex = targetedModule->functionTable.put_create(specializedMethodName + getFuncUniqueNameStr(specializedArgTypes), specializedFunc);
        targetedModule->functionOverloadIndexies[specializedMethodName].push_back(funcIndex);

        moduleContext->popTemplateBuilder();
        return {funcIndex, baseMethodName + getFuncUniqueNameStr(specializedArgTypes)};
    }

    void visitor::specializeStructMethodDefinition(IRTemplateBuilder &structTemplate,
                                         const std::shared_ptr<IRStructDefinition> &specializedStruct,
                                         yoi::implInnerPair *methodAstNode,
                                         const yoi::wstr &specializedStructName,
                                         const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs,
                                         yoi::indexT moduleIndex) {

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);

        // Push the template's own context to resolve generic types like 'T' to their placeholder
        // 'incompleteTemplateType'.
        IRTemplateBuilder genericContext;
        genericContext.templateArguments = structTemplate.templateArguments;

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
        } else if (methodAstNode->isFinalizer()) {
            baseMethodName = L"finalizer";
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

        IRTemplateBuilder specializationContext;
        for (size_t i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = structTemplate.templateArguments.getKey(i);
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
        auto selfType = managedPtr(IRValueType{IRValueType::valueType::structObject,
                                               moduleIndex,
                                               targetedModule->structTable.getIndex(specializedStructName)});
        specializationContext.addTemplateArgument(L"STRUCT", selfType);
        moduleContext->pushTemplateBuilder(specializationContext);

        yoi::vec<std::shared_ptr<IRValueType>> specializedArgTypes;

        if (methodAstNode->isConstructor()) {
            for (auto &arg : methodAstNode->getConstructor().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                specializedArgTypes.push_back(specializedType);
            }
        } else if (methodAstNode->isFinalizer()) {
            // no args
        } else {
            for (auto &arg : methodAstNode->getMethod().getArgs().get()) {
                auto specializedType = managedPtr(parseTypeSpec(&arg->getSpec()));
                specializedArgTypes.push_back(specializedType);
            }
        }

        yoi::wstr specializedMethodName =
            specializedStructName + L"::" + baseMethodName + getFuncUniqueNameStr(specializedArgTypes);

        auto funcIndex = targetedModule->functionTable.getIndex(specializedMethodName);

        moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), targetedModule, targetedModule->functionTable[funcIndex]});
        moduleContext->getIRBuilder().setDebugInfo({targetedModule->modulePath, methodAstNode->getLine(), methodAstNode->getColumn()});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        try {
            visit(methodAstNode->isConstructor() ? &methodAstNode->getConstructor().getBlock()
                                                 : &methodAstNode->getMethod().getBlock(),
                true);
        } catch (std::exception &e) {
            set_current_file_path(moduleContextStack.top().first->getIRBuilder().getCurrentDebugInfo().sourceFile);
            panic(moduleContextStack.top().first->getIRBuilder().getCurrentDebugInfo().line, moduleContextStack.top().first->getIRBuilder().getCurrentDebugInfo().column, std::string("Exception occurred while specializing method: ") + yoi::wstring2string(specializedMethodName) + ": " + e.what() + "\n");
        }

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
                if (!moduleContext->getCompilerContext()->getImportedModule(targetModule)->funcTemplateAsts.contains(templateName)) {
                    throw std::out_of_range("Cannot find the template: " + wstring2string(templateName));
                }

                yoi_assert((*it)->hasTemplateArg(),
                           exportDecl->getLine(),
                           exportDecl->getColumn(),
                           "Expected template arguments for template: " + wstring2string(templateName));

                auto templateArgs = parseTemplateArgs((*it)->getArg());

                auto funcIndex = specializeFunctionTemplate(moduleContext->getCompilerContext()->getImportedModule(targetModule)->funcTemplateAsts[templateName],templateArgs, targetModule);

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
        if (rhs->isForeignBasicType()) {
            rhs = managedPtr(rhs->getNormalizedForeignBasicType());
        }

        if (*rhs == *toType) {
            return;
        } else if (rhs->type == IRValueType::valueType::pointerObject || rhs->type == IRValueType::valueType::pointer || toType->type == IRValueType::valueType::pointerObject || toType->type == IRValueType::valueType::pointer) {
            // no cast needed for pointer type
            return;
        } else if (rhs->isBasicType() && toType->isBasicType() && !rhs->isArrayType() && !toType->isArrayType() && (rhs->type != IRValueType::valueType::stringObject || toType->type == IRValueType::valueType::pointerObject)) {
            emitBasicCastTo(toType);
        } else if (toType->type == IRValueType::valueType::interfaceObject) {
            // check implemented interfaces
            try {
                auto implName = getInterfaceImplName({toType->typeAffiliateModule, toType->typeIndex}, rhs);
                auto implIndex = moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->interfaceImplementationTable.getIndex(implName);
                // construct interface object
                // moduleContext->getIRBuilder().newInterfaceOp(toType->typeIndex, toType->typeAffiliateModule != currentModuleIndex, toType->typeAffiliateModule);
                moduleContext->getIRBuilder().constructInterfaceImplOp({toType->typeAffiliateModule, toType->typeIndex}, implIndex, rhs->typeAffiliateModule != currentModuleIndex, rhs->typeAffiliateModule);
            } catch (std::out_of_range &e) {
                panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to interface " + yoi::wstring2string((toType->to_string())) + ": no implementation found.");
            }
        } else if (toType->type == IRValueType::valueType::structObject) {
            // check whether owns the constructor
            auto structType = moduleContext->getCompilerContext()->getImportedModule(toType->typeAffiliateModule)->structTable[toType->typeIndex];
            auto result = resolveOverloadExtern(L"constructor", {rhs}, toType->typeAffiliateModule, structType);
            if (result.found()) {
                if (result.isCastRequired) {
                    tryCastTo(result.function->argumentTypes.back());
                }
                moduleContext->getIRBuilder().newStructOp(toType->typeIndex, true, toType->typeAffiliateModule);
                moduleContext->getIRBuilder().invokeDanglingOp(result.functionIndex, 2, result.function->returnType, true, toType->typeAffiliateModule);
            } else {
                panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to " + yoi::wstring2string((toType->to_string())) + ": no viable conversion found.");
            }
        } else {
            panic(moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Cannot cast type " + yoi::wstring2string((rhs->to_string())) + " to " + yoi::wstring2string((toType->to_string())) + ": no viable conversion found.");
        }
    }

    bool visitor::canCastTo(const std::shared_ptr<IRValueType> &fromType, const std::shared_ptr<IRValueType> &toType) {
        auto rhs = fromType;
        if (fromType->isForeignBasicType()) {
            rhs = managedPtr(rhs->getNormalizedForeignBasicType());
        }
        if (*rhs == *toType) {
            return true;
        } else if (rhs->isBasicType() && toType->isBasicType() && !rhs->isArrayType() && !toType->isArrayType() && (rhs->type != IRValueType::valueType::stringObject || toType->type == IRValueType::valueType::pointerObject)) {
            return true;
        } else if (rhs->type == IRValueType::valueType::pointerObject) {
            // no cast needed for pointer type
            return true;
        } else if (toType->type == IRValueType::valueType::interfaceObject) {
            // check implemented interfaces
            try {
                auto implName = getInterfaceImplName({toType->typeAffiliateModule, toType->typeIndex}, rhs);
                auto implIndex = moduleContext->getCompilerContext()->getImportedModule(rhs->typeAffiliateModule)->interfaceImplementationTable.getIndex(implName);
                return true;
            } catch (std::out_of_range &e) {
                return false;
            }
        } else if (toType->type == IRValueType::valueType::structObject) {
            // check whether owns the constructor
            auto structType = moduleContext->getCompilerContext()->getImportedModule(toType->typeAffiliateModule)->structTable[toType->typeIndex];
            auto result = resolveOverloadExtern(L"constructor", {rhs}, toType->typeAffiliateModule, structType);
            return result.found();
        } else {
            return false;
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
        try {
            return moduleContext->getCompilerContext()->getImportedModule(structType->typeAffiliateModule)->interfaceImplementationTable.getIndex(nullImplName);
        } catch (std::out_of_range &e) {
            moduleContext->getCompilerContext()->getImportedModule(HOSHI_COMPILER_CTX_GLOB_ID_CONST)->interfaceTable[0]->implementations.emplace_back(
                structType->type, structType->typeAffiliateModule, structType->typeIndex);
            auto nullImpl = managedPtr(IRInterfaceImplementationDefinition{nullImplName, {structType->type, structType->typeAffiliateModule, structType->typeIndex}, {HOSHI_COMPILER_CTX_GLOB_ID_CONST, 0}, {}, {}});
            return moduleContext->getCompilerContext()->getImportedModule(structType->typeAffiliateModule)->interfaceImplementationTable.put_create(nullImplName, nullImpl);
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
                case lexer::token::tokenKind::kIntrinsic:
                    res.push_back(IRFunctionDefinition::FunctionAttrs::Intrinsic);
                default:
                    break;
            }
        }
        return std::move(res);
    }

    yoi::indexT visitor::visit(yoi::newExpression *newExpression) {
        auto baseType = parseTypeSpec(newExpression->type);
        generateNullInterfaceImplementation(managedPtr(baseType.getDynamicArrayType()));
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
            const auto &paramTypes = func->argumentTypes;
            size_t fixedParamCount = paramTypes.size() - 1 - (skipFirstParam && paramTypes.size() > 1 ? 1 : 0);
            if (std::find(func->attrs.begin(), func->attrs.end(), IRFunctionDefinition::FunctionAttrs::Variadic) !=
                func->attrs.end()) {
                if (argTypes.size() >= fixedParamCount) {
                    bool fixedMatch = true;
                    for (size_t i = 0; i < fixedParamCount; ++i) {
                        if (!canCastTo(argTypes[i], paramTypes[i + skipFirstParam])) {
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
            } else {
                if (argTypes.size() != fixedParamCount + 1) // balance the variadic argument
                    return false;

                for (size_t i = 0; i < fixedParamCount + 1; ++i) {
                    if (!canCastTo(argTypes[i], paramTypes[i + skipFirstParam])) {
                        return false;
                    }
                }

                result.functionIndex = targetedModule->functionTable.getIndex(funcKey);
                result.isVariadic = false;
                result.fixedArgCount = fixedParamCount;
                result.function = func;
                result.isCastRequired = true;

                return true;
            }
            return false;
        };

        yoi::wstr prefix = structContext ? structContext->name + L"::" + baseName : baseName;
        for (const auto it : targetedModule->functionOverloadIndexies[prefix]) {
            const auto &key = targetedModule->functionTable.getKey(it);
            if (key.starts_with(prefix)) {
                if (findVariadicMatch(key, structContext != nullptr && !targetedModule->functionTable[it]->hasAttribute(IRFunctionDefinition::FunctionAttrs::Static)))
                    return result;
            }
        }

        if (!structContext && targetedModule->funcTemplateAsts.contains(baseName)) {
            try {
                auto astNode = targetedModule->funcTemplateAsts.at(baseName);
                auto templateArgs = getTemplateArgs(astNode->id->getArg());

                yoi::vec<std::shared_ptr<IRValueType>> deducedArgs(templateArgs.size());
                
                for (yoi::indexT i = 0; i < argTypes.size(); i++) {
                    if (i < templateArgs.size() &&
                        templateArgs[i].templateType->type == IRValueType::valueType::incompleteTemplateType) {
                        auto &srcTypeToPlace = argTypes[i];
                        auto incompleteTypeIndex = templateArgs[i].templateType->typeIndex;
                        if (deducedArgs[incompleteTypeIndex] && *deducedArgs[incompleteTypeIndex] != *srcTypeToPlace) {
                             throw std::runtime_error("Template argument type mismatch during deduction.");
                        }
                        deducedArgs[incompleteTypeIndex] = srcTypeToPlace;
                    }
                }
                for (yoi::indexT i = 0; i < deducedArgs.size(); i++) {
                    if (deducedArgs[i] == nullptr) {
                        throw std::runtime_error("Cannot deduce all template arguments for: " + yoi::wstring2string(baseName));
                    }
                }
                
                auto specializedFuncIndex = specializeFunctionTemplate(astNode, deducedArgs, targetModule);
                result.functionIndex = specializedFuncIndex;
                result.function = targetedModule->functionTable[specializedFuncIndex];

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
                                         const std::shared_ptr<IRValueType> &structContext, bool noThisCall) {
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
        bool skipFirstParam = structContext != nullptr && !noThisCall;

        if (overload.isVariadic) {
            moduleContext->getIRBuilder().restoreState();
            for (size_t i = 0; i < overload.fixedArgCount; ++i) {
                visit(args->get()[i]);
                tryCastTo(overload.function->argumentTypes[i + skipFirstParam]);
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
        } else if (overload.isCastRequired) {
            moduleContext->getIRBuilder().restoreState();
            for (size_t i = 0; i < overload.fixedArgCount + 1; ++i) { // balanced for interface
                visit(args->get()[i]);
                tryCastTo(overload.function->argumentTypes[i + skipFirstParam]);
            }
        } else {
            moduleContext->getIRBuilder().discardState();
        }

        size_t finalParamCount = overload.function->argumentTypes.size();
        if (structContext && structContext->type == IRValueType::valueType::structObject) {
            auto externEntry = getExternEntry(targetModule, fullMangledName);
            auto isStaticMethod = std::find(overload.function->attrs.begin(), overload.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) != overload.function->attrs.end();
            bool usePureStaticLogic = noThisCall && isStaticMethod;

            if (usePureStaticLogic) {
                moduleContext->getIRBuilder().invokeOp(externEntry.itemIndex,
                                                            finalParamCount,
                                                            overload.function->returnType,
                                                            true,
                                                            externEntry.affiliateModule);
            } else {
                moduleContext->getIRBuilder().invokeMethodOp(externEntry.itemIndex,
                                                            finalParamCount - !isStaticMethod,
                                                            overload.function->returnType,
                                                            isStaticMethod,
                                                            true,
                                                            externEntry.affiliateModule);
            }
        } else if (structContext && structContext->type == IRValueType::valueType::interfaceObject) {
            moduleContext->getIRBuilder().invokeVirtualOp(overload.functionIndex,
                                                          structContext->typeIndex,
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

    template <typename T>
    yoi::indexT visitor::handleBinaryOperatorOverload(const yoi::wstr &overloadName, T *rhsAST) {
        auto lhs = moduleContext->getIRBuilder().getLhsFromTempVarStack();
        auto rhs = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        bool isResolved = false;

        if (lhs->type == IRValueType::valueType::structObject)  {
            auto resolved = resolveOverloadExtern(overloadName, {lhs, rhs}, lhs->typeAffiliateModule, moduleContext->getCompilerContext()->getImportedModule(lhs->typeAffiliateModule)->structTable[lhs->typeIndex]);
            if (resolved.found()) {
                yoi_assert(resolved.isVariadic == false, 0, 0, "Binary operator overloading with variadic functions is not supported.");
                yoi_assert(std::find(resolved.function->attrs.begin(), resolved.function->attrs.end(), IRFunctionDefinition::FunctionAttrs::Static) != resolved.function->attrs.end(), moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Binary operator overloading with non-static functions is not supported.");

                if (resolved.isCastRequired) {
                    moduleContext->getIRBuilder().restoreState();
                    tryCastTo(resolved.function->argumentTypes.front());
                    visit(rhsAST);
                    tryCastTo(resolved.function->argumentTypes.back());
                } else {
                    moduleContext->getIRBuilder().discardState();
                }

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

                if (resolved.isCastRequired) {
                    moduleContext->getIRBuilder().restoreState();
                    tryCastTo(resolved.function->argumentTypes.front());
                    visit(rhsAST);
                    tryCastTo(resolved.function->argumentTypes.back());
                } else {
                    moduleContext->getIRBuilder().discardState();
                }

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

            auto resolved = resolveOverloadInterface(baseName, {lhs, rhs}, lhs->typeAffiliateModule, moduleContext->getCompilerContext()->getImportedModule(lhs->typeAffiliateModule)->interfaceTable[lhs->typeIndex]);

            if (resolved.found()) {
                if (resolved.isCastRequired) {
                    moduleContext->getIRBuilder().restoreState();
                    tryCastTo(resolved.function->argumentTypes.front());
                    visit(rhsAST);
                    tryCastTo(resolved.function->argumentTypes.back());
                } else {
                    moduleContext->getIRBuilder().discardState();
                }

                moduleContext->getIRBuilder().invokeVirtualOp(resolved.functionIndex, lhs->typeIndex, 1, resolved.function->returnType, true, lhs->typeAffiliateModule);
                isResolved = true;
            }
        }

        if (!isResolved) {
            moduleContext->getIRBuilder().restoreState();
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
            moduleContext->getIRBuilder().invokeVirtualOp(methodIdx, rhs->typeIndex, 0, method->returnType, true, rhs->typeAffiliateModule);
            isResolved = true;
        }
        yoi_assert(isResolved, moduleContext->getIRBuilder().getCurrentDebugInfo().line, moduleContext->getIRBuilder().getCurrentDebugInfo().column, "Unary operator overloading not found for " + yoi::wstring2string(overloadName));

        return moduleContext->getIRBuilder().getCurrentInsertionPoint();
    }

    yoi::indexT visitor::specializeInterfaceTemplate(const yoi::wstr &templateName,
                                                     const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs, yoi::indexT moduleIndex) {
        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);
        yoi::wstr specializedName = getMangledTemplateName(templateName, concreteTemplateArgs);
        if (targetedModule->interfaceTable.contains(specializedName)) {
            return targetedModule->interfaceTable.getIndex(specializedName);
        }
    
        yoi_assert(targetedModule->templateInterfaceAsts.contains(templateName), 0, 0, "Unknown interface template: " + wstring2string(templateName));
    
        auto interfaceAst = targetedModule->templateInterfaceAsts.at(templateName);
    
        IRTemplateBuilder specializationContext;
        yoi_assert(concreteTemplateArgs.size() == interfaceAst->id->arg->get().size(), 0, 0, "Template argument count mismatch for interface " + wstring2string(templateName));
        for (yoi::indexT i = 0; i < concreteTemplateArgs.size(); ++i) {
            auto paramName = interfaceAst->id->arg->get()[i]->getId().get().strVal;
            specializationContext.addTemplateArgument(paramName, concreteTemplateArgs[i]);
        }
    
        pushModuleContext(moduleIndex);
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
                    auto argType = managedPtr(
                        arg->spec->elipsis
                        ? parseTypeSpec(arg->spec->elipsis).getDynamicArrayType()
                        : moduleContext->getCompilerContext()->getNullInterfaceType()->getDynamicArrayType()
                    );
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
        auto interfaceIndex = irModule->interfaceTable.put_create(specializedName, specializedInterface); // since module context is still in foreign module, no need to change to targetedModule
    
        moduleContext->popTemplateBuilder();
        popModuleContext();
        return interfaceIndex;
    }

    void visitor::specializeInterfaceImplementation(yoi::implStmt *implAst,
                                                const std::shared_ptr<IRValueType> &concreteStructType,
                                                const yoi::wstr& specializedStructName,
                                                const yoi::vec<std::shared_ptr<IRValueType>> &concreteTemplateArgs, yoi::indexT targetModule) {
        
        yoi_assert(implAst->isImplForStmt(), implAst->getLine(), implAst->getColumn(), "Expected 'impl for' AST node for interface implementation specialization.");

        auto targetedModule = moduleContext->getCompilerContext()->getImportedModule(targetModule);

        // The active specialization context (from specializeStructTemplate) resolves types like `T` to concrete types.
        auto concreteInterfaceType = managedPtr(parseTypeSpec(implAst->interfaceName));
        yoi_assert(concreteInterfaceType->type == IRValueType::valueType::interfaceObject, implAst->getLine(), implAst->getColumn(), "Expected an interface type.");

        // pushModuleContext(targetModule);

        auto interfaceSrcPair = std::make_pair(concreteInterfaceType->typeAffiliateModule, concreteInterfaceType->typeIndex);

        auto targetInterface = moduleContext->getCompilerContext()->getImportedModule(interfaceSrcPair.first)
            ->interfaceTable[interfaceSrcPair.second];

        targetInterface->implementations.emplace_back(
            concreteStructType->type, concreteStructType->typeAffiliateModule, concreteStructType->typeIndex
        );
        
        auto implName = getInterfaceImplName(interfaceSrcPair, concreteStructType);
        if (irModule->interfaceImplementationTable.contains(implName)) {
            return; // Already specialized and created.
        }

        yoi::indexT implIndex{};
        try {
            implIndex = targetedModule->interfaceImplementationTable.getIndex(implName);
            if (targetedModule->interfaceImplementationTable[implIndex]) {
                panic(implAst->getLine(), implAst->getColumn(), "Redefinition of interface implementation: " + yoi::wstring2string(implName));
            }
        } catch (std::out_of_range &e) {
            implIndex = targetedModule->interfaceImplementationTable.put_create(implName, nullptr);
        }

        if (implAst->inner) {
            IRInterfaceImplementationDefinition::Builder builder;
            builder.setName(implName);
            builder.setImplStructIndex({concreteStructType->type, concreteStructType->typeAffiliateModule, concreteStructType->typeIndex});
            builder.setImplInterfaceIndex(interfaceSrcPair);

            std::map<yoi::wstr, std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> virtualMethodMap;

            for (auto &methodNode : implAst->getInner().getInner()) {
                yoi_assert(!methodNode->isConstructor(), methodNode->getLine(), methodNode->getColumn(), "Only methods are allowed in interface implementations.");
                auto &methodAst = methodNode->getMethod();

                IRFunctionDefinition::Builder methodBuilder;
                methodBuilder.setDebugInfo({irModule->modulePath, methodAst.getLine(), methodAst.getColumn()});
                methodBuilder.attrs = getFunctionAttributes(methodAst.attrs);
                methodBuilder.attrs.emplace_back(IRFunctionDefinition::FunctionAttrs::Preserve);
                methodBuilder.attrs.emplace_back(IRFunctionDefinition::FunctionAttrs::NoRawAndNullOptimization);
                
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
                irModule->functionOverloadIndexies[specializedStructName + L"::" + baseMethodName].emplace_back(funcIndex);

                moduleContext->pushIRBuilder({moduleContext->getCompilerContext(), irModule, func});
                moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, methodAst.getLine(), methodAst.getColumn()});
                moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
                visit(methodAst.block, true);
                moduleContext->getIRBuilder().yield();
                moduleContext->popIRBuilder();
                virtualMethodMap[baseMethodName + getFuncUniqueNameStr(specializedArgTypes, true)] = {baseMethodName + uniq, managedPtr(IRValueType{IRValueType::valueType::virtualMethod, currentModuleIndex, funcIndex})};
            }
            for (auto &method : targetInterface->methodMap) {
                yoi_assert(virtualMethodMap.contains(method.first), implAst->getLine(), implAst->getColumn(), "Interface method not found in implementation: " + wstring2string(method.first));
                builder.addVirtualMethod(virtualMethodMap[method.first].first, virtualMethodMap[method.first].second);
            }
            
            // popModuleContext();

            targetedModule->interfaceImplementationTable[implIndex] = builder.yield();
        } else {
            // forward-declaration
        }
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
            const auto &paramTypes = func->argumentTypes;
            size_t fixedParamCount = paramTypes.size() - 1;
            if (std::find(func->attrs.begin(), func->attrs.end(), IRFunctionDefinition::FunctionAttrs::Variadic) !=
                func->attrs.end()) {
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
            } else {
                if (argTypes.size() != fixedParamCount + 1) // balance the variadic argument
                    return false;

                for (size_t i = 0; i < fixedParamCount; ++i) {
                    if (!canCastTo(argTypes[i], paramTypes[i])) {
                        return false;
                    }
                }

                result.functionIndex = interfaceContext->methodMap.getIndex(funcKey);
                result.isVariadic = false;
                result.fixedArgCount = fixedParamCount;
                result.function = func;
                result.isCastRequired = true;

                return true;
            }
            return false;
        };

        for (const auto &it : interfaceContext->methodMap) {
            if (it.second->name.starts_with(baseName)) {
                if (findVariadicMatch(it.first))
                    return result;
            }
        }

        return result; // Not found
    }

    void visitor::pushModuleContext(yoi::indexT moduleIndex) {
        moduleContextStack.emplace(moduleContext, currentModuleIndex);
        this->moduleContext = moduleContext->getCompilerContext()->getModuleContext(moduleIndex);
        this->irModule = moduleContext->getCompilerContext()->getImportedModule(moduleIndex);
        currentModuleIndex = moduleIndex;
    }

    void visitor::popModuleContext() {
        this->moduleContext = moduleContextStack.top().first;
        currentModuleIndex = moduleContextStack.top().second;
        this->irModule = moduleContext->getCompilerContext()->getImportedModule(currentModuleIndex);
        moduleContextStack.pop();
    }

    bool visitor::handleSubscript(yoi::vec<yoi::subscript *>::iterator &it,
                                  yoi::vec<yoi::subscript *>::iterator end,
                                  bool isStoreOp,
                                  bool isLastTerm) {
        auto objectOnStackType = moduleContext->getIRBuilder().getRhsFromTempVarStack();
        auto currentTerm = *it;

        if (objectOnStackType->isArrayType() || objectOnStackType->isDynamicArrayType()) {
            const auto &dimensions = objectOnStackType->dimensions;
            yoi::vec<yoi::indexT> strides(dimensions.size());
            strides.back() = 1;
            for (long long i = static_cast<long long>(dimensions.size()) - 2; i >= 0; --i) {
                strides[i] = strides[i + 1] * dimensions[i + 1];
            }

            moduleContext->getIRBuilder().pushOp(
                IR::Opcode::push_unsigned,
                {IROperand::operandType::unsignedInt, IROperand::operandValue{static_cast<int64_t>(0)}});

            yoi::indexT currentDim = 0;
            while (it != end && (*it)->isSubscript()) {
                yoi_assert(currentDim < dimensions.size(),
                           (*it)->getLine(),
                           (*it)->getColumn(),
                           "Too many indices for array dimension.");
                visit((*it)->expr);
                tryCastTo(moduleContext->getCompilerContext()->getUnsignedObjectType());
                yoi_assert(moduleContext->getIRBuilder().getRhsFromTempVarStack()->type ==
                               IRValueType::valueType::unsignedObject,
                           (*it)->getLine(),
                           (*it)->getColumn(),
                           "Array subscript index must be an integer or unsigned integer.");
                moduleContext->getIRBuilder().pushOp(IR::Opcode::push_unsigned,
                                                     {IROperand::operandType::unsignedInt, strides[currentDim]});
                moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::mul);
                moduleContext->getIRBuilder().arithmeticOp(IR::Opcode::add);
                it++;
                currentDim++;
            }

            yoi_assert(currentDim == dimensions.size() || (isStoreOp && isLastTerm),
                       currentTerm->getLine(),
                       currentTerm->getColumn(),
                       "Partial array access is not a loadable value. Not enough indices provided.");

            if (isStoreOp && isLastTerm) {
                moduleContext->getIRBuilder().storeOp(IR::Opcode::store_element, {});
            } else {
                auto elementType = managedPtr(objectOnStackType->getElementType());
                moduleContext->getIRBuilder().loadOp(IR::Opcode::load_element, {}, elementType);
            }
            return true;
        } else {
            if (isLastTerm && isStoreOp) {
                // current stack: [..., value, array, index]
                auto value = moduleContext->getIRBuilder().getLhsFromTempVarStack();
                auto array = moduleContext->getIRBuilder().getRhsFromTempVarStack();
                moduleContext->getIRBuilder().saveState();
                visit(currentTerm->expr);
                auto index = moduleContext->getIRBuilder().getRhsFromTempVarStack();

                OverloadResult overload;
                if (array->type == IRValueType::valueType::structObject)
                    overload = resolveOverloadExtern(L"operator[]",
                                                     {value, array, index},
                                                     array->typeAffiliateModule,
                                                     moduleContext->getCompilerContext()
                                                         ->getImportedModule(array->typeAffiliateModule)
                                                         ->structTable[array->typeIndex]);
                /*else if(array->type == IRValueType::valueType::interfaceObject)
                    overload = resolveOverloadInterface(L"operator[]", {value, array, index},
                   array->typeAffiliateModule,
                        moduleContext->getCompilerContext()->getImportedModule(array->typeAffiliateModule)->interfaceTable[array->typeIndex]);*/

                yoi_assert(overload.found(),
                           currentTerm->getLine(),
                           currentTerm->getColumn(),
                           "No matching overload found for operator[].");
                yoi_assert(!overload.isVariadic,
                           currentTerm->getLine(),
                           currentTerm->getColumn(),
                           "Variadic operator[] overloading is not supported.");

                // FIXME: 前面value已经被运算了而且没有保存状态，不知道要怎么搞了，除非每次入栈的时候顺便记录一下当前insertion point
                if (overload.isCastRequired) {
                    moduleContext->getIRBuilder().restoreState();
                    visit(currentTerm->expr);
                    tryCastTo(overload.function->argumentTypes.back());
                } else {
                    moduleContext->getIRBuilder().discardState();
                }

                moduleContext->getIRBuilder().invokeMethodOp(
                    overload.functionIndex, 2, overload.function->returnType, false, true, array->typeAffiliateModule);

                moduleContext->getIRBuilder().popOp();
            } else {
                moduleContext->getIRBuilder().saveState();
                visit(currentTerm->expr);
                handleBinaryOperatorOverload(L"operator[]", currentTerm->expr);
            }
        }
        return false;
    }

    IRValueType visitor::parseTypeSpec(yoi::funcTypeSpec *typeSpec) {
        vec<std::shared_ptr<IRValueType>> argTypes;
        for (auto &arg : typeSpec->args->types) {
            argTypes.push_back(managedPtr(parseTypeSpec(arg)));
        }
        auto returnType = managedPtr(parseTypeSpec(typeSpec->resultType));

        return IRValueType{IRValueType::valueType::interfaceObject,
                            HOSHI_COMPILER_CTX_GLOB_ID_CONST,
                            createCallableInterface(argTypes, returnType)};
    }

    yoi::indexT visitor::createCallableInterface(const yoi::vec<std::shared_ptr<IRValueType>> &parameterTypes,
                                                 const std::shared_ptr<IRValueType> &returnType) {
        auto callableInterfaceName = L"callable" + getFuncUniqueNameStr(parameterTypes);
        callableInterfaceName += getTypeSpecUniqueNameStr(returnType);
        if (moduleContext->getCompilerContext()->getImportedModule(HOSHI_COMPILER_CTX_GLOB_ID_CONST)->interfaceTable.contains(callableInterfaceName)) {
            return moduleContext->getCompilerContext()->getImportedModule(HOSHI_COMPILER_CTX_GLOB_ID_CONST)
                ->interfaceTable.getIndex(callableInterfaceName);
        }

        auto interfaceIndex = moduleContext->getCompilerContext()
            ->getImportedModule(HOSHI_COMPILER_CTX_GLOB_ID_CONST)
            ->interfaceTable.put_create(callableInterfaceName, {});

        yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> argTypes;
        for (auto &arg : parameterTypes) {
            argTypes.emplace_back(L"arg", arg);
        }
        IRInterfaceInstanceDefinition::Builder builder;
        builder.setName(callableInterfaceName);
        builder.addMethod(L"operator()" + getFuncUniqueNameStr(parameterTypes), managedPtr(IRFunctionDefinition{
            L"operator()" + getFuncUniqueNameStr(parameterTypes),
            argTypes,
            returnType,
            {},
            {},
            {}
        }));
        
        moduleContext->getCompilerContext()->getImportedModule(HOSHI_COMPILER_CTX_GLOB_ID_CONST)->interfaceTable[interfaceIndex] = builder.yield();

        return interfaceIndex;
    }

    yoi::indexT visitor::createLambdaUnnamedStruct(yoi::lambdaExpr *lambdaExpr) {
        auto structName = L"lambda" + std::to_wstring(lambdaExpr->getLine()) + L"_" + std::to_wstring(lambdaExpr->getColumn());
        auto structIndex = irModule->structTable.put_create(structName, nullptr);

        auto structType = managedPtr(IRValueType{IRValueType::valueType::structObject, currentModuleIndex, structIndex});

        IRStructDefinition::Builder builder;
        builder.setName(structName);
        // add captured variables as fields
        IRFunctionDefinition::Builder callableBuilder;
        yoi::vec<std::shared_ptr<IRValueType>> argTypes;

        callableBuilder.setDebugInfo({irModule->modulePath, lambdaExpr->getLine(), lambdaExpr->getColumn()});
        callableBuilder.attrs.push_back(IRFunctionDefinition::FunctionAttrs::Preserve);
        callableBuilder.attrs.emplace_back(IRFunctionDefinition::FunctionAttrs::NoRawAndNullOptimization);
        callableBuilder.addArgument(L"this", structType);
        for (auto &i : lambdaExpr->args->spec) {
            auto argType = managedPtr(parseTypeSpec(i->spec));
            callableBuilder.addArgument(i->id->node.strVal, argType);
            argTypes.push_back(argType);
        }
        auto returnType = managedPtr(parseTypeSpec(lambdaExpr->resultType));
        callableBuilder.setReturnType(returnType);
        callableBuilder.setName(structName + L"::operator()" + getFuncUniqueNameStr(argTypes));
        auto callableFunc = callableBuilder.yield();
        auto callableFuncIndex = irModule->functionTable.put_create(callableFunc->name, callableFunc);
        irModule->functionOverloadIndexies[structName + L"::operator()"].push_back(callableFuncIndex);
        builder.addMethod(L"operator()" + getFuncUniqueNameStr(argTypes), callableFuncIndex);
        
        // add captured variables as fields
        // now a trick here, we put new_struct op first, so that we can build the IR without rolling back the builder state
        // when we add captured variables as fields
        argTypes.clear();
        moduleContext->getIRBuilder().newStructOp(structIndex);
        for (auto &i : lambdaExpr->captures) {
            visit(i);
            auto capturedVar = moduleContext->getIRBuilder().getRhsFromTempVarStack();
            // i guess the IRValueType here is referenceable.
            capturedVar->addAttribute(IRValueType::ValueAttr::Nullable);
            argTypes.push_back(managedPtr(*capturedVar));
            builder.addField(i->node.strVal, managedPtr(*capturedVar));
        }
        // add the constructor method
        IRFunctionDefinition::Builder constructorBuilder;
        constructorBuilder.setDebugInfo({irModule->modulePath, lambdaExpr->getLine(), lambdaExpr->getColumn()});
        constructorBuilder.addArgument(L"this", structType);
        constructorBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::Constructor);
        constructorBuilder.addAttr(IRFunctionDefinition::FunctionAttrs::NoRawAndNullOptimization);
        for (yoi::indexT i = 0;i < argTypes.size(); ++i) {
            constructorBuilder.addArgument(lambdaExpr->captures[i]->node.strVal, argTypes[i]);
        }
        constructorBuilder.setReturnType(structType);
        constructorBuilder.setName(structName + L"::constructor" + getFuncUniqueNameStr(argTypes));
        auto constructorFunc = constructorBuilder.yield();
        auto constructorFuncIndex = irModule->functionTable.put_create(constructorFunc->name, constructorFunc);
        irModule->functionOverloadIndexies[structName + L"::constructor"].push_back(constructorFuncIndex);
        builder.addMethod(L"constructor" + getFuncUniqueNameStr(argTypes), constructorFuncIndex);

        // now we add the struct to the module
        irModule->structTable[structIndex] = builder.yield();

        // now we invokes the constructor method to initialize the struct
        moduleContext->getIRBuilder().invokeMethodOp(constructorFuncIndex, argTypes.size(), structType, false, true, currentModuleIndex);

        // finally, we generate the implementations for both operator() and constructor
        moduleContext->pushIRBuilder(IRBuilder{
            moduleContext->getCompilerContext(),
            irModule,
            irModule->functionTable[callableFuncIndex]
        });
        moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, lambdaExpr->getLine(), lambdaExpr->getColumn()});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        visit(lambdaExpr->block, true);
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();

        moduleContext->pushIRBuilder(IRBuilder{
            moduleContext->getCompilerContext(),
            irModule,
            irModule->functionTable[constructorFuncIndex]
        });
        moduleContext->getIRBuilder().setDebugInfo({irModule->modulePath, lambdaExpr->getLine(), lambdaExpr->getColumn()});
        moduleContext->getIRBuilder().switchCodeBlock(moduleContext->getIRBuilder().createCodeBlock());
        for (yoi::indexT i = 0; i < lambdaExpr->captures.size(); ++i) {
            // we store our parameters into the fields of the struct
            // load local in accordance with the order of arguments
            moduleContext->getIRBuilder().loadOp(IR::Opcode::load_local, {IROperand::operandType::localVar, i + 1}, argTypes[i]); // take advantages of the argTypes which we haven't clear it yet.
            // store the parameter into the field
            // now we load the this pointer, and store the parameter into the field
            moduleContext->getIRBuilder().loadOp(IR::Opcode::load_local, {IROperand::operandType::localVar, IROperand::operandValue{static_cast<yoi::indexT>(0)}}, structType);
            moduleContext->getIRBuilder().storeMemberOp({IROperand::operandType::index, i});
        }
        moduleContext->getIRBuilder().loadOp(IR::Opcode::load_local, {IROperand::operandType::localVar, IROperand::operandValue{static_cast<yoi::indexT>(0)}}, structType);
        moduleContext->getIRBuilder().retOp();
        moduleContext->getIRBuilder().yield();
        moduleContext->popIRBuilder();

        return structIndex;
    }

    std::pair<yoi::indexT, std::pair<yoi::indexT, yoi::indexT>> visitor::createCallableImplementationForLambda(const std::shared_ptr<IRStructDefinition> &lambda,
                                                               yoi::indexT lambdaStructIndex,
                                                               yoi::indexT moduleIndex) {
        yoi::wstr callableName;
        yoi::indexT callableIndex;
        yoi::vec<std::shared_ptr<IRValueType>> argTypes;
        for (auto &[key, value] : lambda->nameIndexMap) {
            if (key.starts_with(L"operator()")) {
                callableName = key;
                callableIndex = value.index;
                break;
            }
        }
        auto callableFunc = moduleContext->getCompilerContext()->getImportedModule(moduleIndex)->functionTable[callableIndex];
        // ignore the first argument, which is the this pointer
        for (yoi::indexT i = 1; i < callableFunc->argumentTypes.size(); ++i) {
            argTypes.push_back(callableFunc->argumentTypes[i]);
        }
        auto returnType = callableFunc->returnType;

        auto interfaceSrc = std::pair{HOSHI_COMPILER_CTX_GLOB_ID_CONST, createCallableInterface(argTypes, returnType)};
        auto interfaceImpl = getInterfaceImplName(interfaceSrc, moduleContext->getIRBuilder().getRhsFromTempVarStack());


        IRInterfaceImplementationDefinition::Builder builder;
        builder.setImplInterfaceIndex(interfaceSrc)
               .setImplStructIndex({IRValueType::valueType::structObject, moduleIndex, lambdaStructIndex})
               .setName(interfaceImpl)
               .addVirtualMethod(callableName, managedPtr(IRValueType{
                IRValueType::valueType::virtualMethod,
                moduleIndex,
                callableIndex
               }));
        auto implIndex = irModule->interfaceImplementationTable.put_create(interfaceImpl, builder.yield());

        return {implIndex, interfaceSrc};
    }

    bool visitor::checkMarcoSatisfaction(yoi::marcoDescriptor *desc) {
        if (!desc)
            return true;
        bool satisfied = true;

        auto convertToSameType = [](lexer::token &lhs, lexer::token &rhs) -> std::pair<lexer::token, lexer::token> {
            switch (lhs.kind) {
                case lexer::token::tokenKind::integer:
                    switch (rhs.kind) {
                        case lexer::token::tokenKind::integer: return {lhs, rhs};
                        case lexer::token::tokenKind::decimal: return {lexer::token{0, 0, lexer::token::tokenKind::decimal, static_cast<double>(lhs.basicVal.vDeci)}, rhs};
                        default: panic(lhs.line, lhs.col, "Cannot convert marco value to comparable type.");
                    }
                case lexer::token::tokenKind::decimal:
                    switch (rhs.kind) {
                        case lexer::token::tokenKind::integer: return {lhs, lexer::token{0, 0, lexer::token::tokenKind::decimal, static_cast<double>(rhs.basicVal.vUint)}};
                        case lexer::token::tokenKind::decimal: return {lhs, rhs};
                        default: panic(lhs.line, lhs.col, "Cannot convert marco value to comparable type.");
                    }
                case lexer::token::tokenKind::string:
                    switch (rhs.kind) {
                        case lexer::token::tokenKind::string: return {lhs, rhs};
                        case lexer::token::tokenKind::boolean: return {lexer::token{0, 0, lexer::token::tokenKind::integer, static_cast<uint64_t>(lhs.strVal == rhs.strVal)}, rhs};
                        default: panic(lhs.line, lhs.col, "Cannot convert marco value to comparable type.");
                    }
                case lexer::token::tokenKind::boolean:
                    switch (rhs.kind) {
                        case lexer::token::tokenKind::string: return {lexer::token{0, 0, lexer::token::tokenKind::integer, static_cast<uint64_t>(lhs.strVal == rhs.strVal)}, rhs};
                        case lexer::token::tokenKind::boolean: return {lhs, rhs};
                        default: panic(lhs.line, lhs.col, "Cannot convert marco value to comparable type.");
                    }
                default:;
            }
            return {lhs, rhs};
        };
        auto compare = [&](lexer::token &lhs, lexer::token &rhs, lexer::token::tokenKind op) {
            auto [lhsTok, rhsTok] = convertToSameType(lhs, rhs);
            switch (op) {
                case lexer::token::tokenKind::equal: return lhsTok.basicVal.vUint == rhsTok.basicVal.vUint && lhsTok.strVal == rhsTok.strVal;
                case lexer::token::tokenKind::notEqual: return lhsTok.basicVal.vUint != rhsTok.basicVal.vUint || lhsTok.strVal != rhsTok.strVal;
                case lexer::token::tokenKind::greaterThan:
                    switch (lhsTok.kind) {
                        case lexer::token::tokenKind::integer: return lhsTok.basicVal.vUint > rhsTok.basicVal.vUint;
                        case lexer::token::tokenKind::decimal: return lhsTok.basicVal.vDeci > rhsTok.basicVal.vDeci;
                        default: panic(lhs.line, lhs.col, "Cannot compare marco value.");
                    }
                case lexer::token::tokenKind::greaterEqual:
                    switch (lhsTok.kind) {
                        case lexer::token::tokenKind::integer: return lhsTok.basicVal.vUint >= rhsTok.basicVal.vUint;
                        case lexer::token::tokenKind::decimal: return lhsTok.basicVal.vDeci >= rhsTok.basicVal.vDeci;
                        default: panic(lhs.line, lhs.col, "Cannot compare marco value.");
                    }
                case lexer::token::tokenKind::lessThan:
                    switch (lhsTok.kind) {
                        case lexer::token::tokenKind::integer: return lhsTok.basicVal.vUint < rhsTok.basicVal.vUint;
                        case lexer::token::tokenKind::decimal: return lhsTok.basicVal.vDeci < rhsTok.basicVal.vDeci;
                        default: panic(lhs.line, lhs.col, "Cannot compare marco value.");
                    }
                case lexer::token::tokenKind::lessEqual:
                    switch (lhsTok.kind) {
                        case lexer::token::tokenKind::integer: return lhsTok.basicVal.vUint <= rhsTok.basicVal.vUint;
                        case lexer::token::tokenKind::decimal: return lhsTok.basicVal.vDeci <= rhsTok.basicVal.vDeci;
                        default: panic(lhs.line, lhs.col, "Cannot compare marco value.");
                    }
                default:;
            }
            return false;
        };

        auto &marcos = moduleContext->getCompilerContext()->getBuildConfig()->marcos;
        for (auto &i : desc->pairs) {
            bool currentSatisfied = false;
            auto &marco = i->identifier.strVal;
            yoi_assert(marcos.contains(marco), desc->getLine(), desc->getColumn(), "Undefined marco: " + yoi::wstring2string(marco));
            auto &value = marcos[marco];
            auto tok = lexer(std::wstringstream(value)).scan();
            tok.kind = tok.kind == lexer::token::tokenKind::identifier ? lexer::token::tokenKind::string : tok.kind; 

            auto &targetValue = i->rhs;
            switch (i->constraint.kind) {
                case lexer::token::tokenKind::equal: {
                    currentSatisfied = tok.basicVal.vUint == targetValue.basicVal.vUint && tok.strVal == targetValue.strVal;
                    break;
                }
                case lexer::token::tokenKind::notEqual: {
                    currentSatisfied = tok.basicVal.vUint != targetValue.basicVal.vUint || tok.strVal != targetValue.strVal;
                    break;
                }
                case lexer::token::tokenKind::greaterThan:
                case lexer::token::tokenKind::greaterEqual:
                case lexer::token::tokenKind::lessThan:
                case lexer::token::tokenKind::lessEqual: {
                    currentSatisfied = compare(tok, targetValue, i->constraint.kind);
                    break;
                }
                default: {
                    panic(desc->getLine(), desc->getColumn(), "Unsupported marco constraint kind: " + std::string{magic_enum::enum_name(i->constraint.kind)});
                    break;
                }
            }
            if (!currentSatisfied) {
                satisfied = false;
                break;
            }
        }
        return satisfied; 
    }

    void visitor::visit(yoi::typeAliasStmt *typeAlias) {
        if (typeAlias->lhs->hasDefTemplateArg()) {
            panic(typeAlias->getLine(), typeAlias->getColumn(), "type alias template not implemented yet");
        } else {
            auto aliasName = typeAlias->lhs->getId().node.strVal;
            auto rhs = parseTypeSpec(typeAlias->rhs);
            if (irModule->typeAliases.contains(aliasName)) {
                panic(typeAlias->getLine(), typeAlias->getColumn(), "Redefinition of type alias: " + yoi::wstring2string(aliasName));
            } else {
                irModule->typeAliases[aliasName] = rhs;
            }
        }
    }

    std::shared_ptr<IRValueType> visitor::mapEnumTypeToBasicType(yoi::indexT targetModule, yoi::indexT targetEnumType) {
        auto enumDef = moduleContext->getCompilerContext()->getImportedModule(targetModule)->enumerationTable[targetEnumType];
        switch (enumDef->getUnderlyingType()) {
            case IREnumerationType::UnderlyingType::I8:
                return moduleContext->getCompilerContext()->getCharObjectType();
            case IREnumerationType::UnderlyingType::I16:
                return moduleContext->getCompilerContext()->getShortObjectType();
            case IREnumerationType::UnderlyingType::I64:
                return moduleContext->getCompilerContext()->getUnsignedObjectType();
            default:
                return nullptr;
        }
    }

    void visitor::visit(yoi::enumerationDefinition *enumerationDefinition) {
        IREnumerationType::Builder builder;
        builder.setName(enumerationDefinition->name->get().strVal);
        yoi::indexT idx = 0;
        for (auto &node : enumerationDefinition->values) {
            idx = node->value.kind != lexer::token::tokenKind::unknown ? node->value.basicVal.vInt : idx;
            builder.addValue(node->name->get().strVal, idx++);
        }
        auto enumType = builder.yield();
        auto enumIndex = irModule->enumerationTable.put_create(enumType->name, enumType);
        auto underlyingEnumType = mapEnumTypeToBasicType(currentModuleIndex, enumIndex);
        irModule->typeAliases[enumerationDefinition->name->get().strVal] = *underlyingEnumType;
    }
} // namespace yoi