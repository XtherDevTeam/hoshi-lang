//
// Created by XIaokang00010 on 2024/9/6.
//

#include "IR.h"

#include "compiler/builtinModule.hpp"
#include "compiler/moduleContext.h"
#include "share/def.hpp"
#include "share/magic_enum.h"
#include <memory>
#include <ranges>

#include <compiler/frontend/ast.hpp>
#include <stdexcept>
#include <string>

namespace yoi {
    IROperand::operandValue::operandValue() : stringLiteralIndex(0) {}

    IROperand::operandValue::operandValue(int64_t integer) : integer(integer) {}

    IROperand::operandValue::operandValue(yoi::indexT indexV) : stringLiteralIndex(indexV) {}

    IROperand::operandValue::operandValue(bool boolean) : boolean(boolean) {}

    IROperand::operandValue::operandValue(double decimal) : decimal(decimal) {}

    IROperand::operandValue::operandValue(yoi::wchar character) : character(character) {}

    IROperand::IROperand() : type(operandType::unknown), value((yoi::indexT)0) {}

    IROperand::IROperand(IROperand::operandType type, IROperand::operandValue value) : type(type), value(value) {}

    IROperand::IROperand(IROperand::operandType type, std::shared_ptr<IRValueType> lvalueType) : type(type), lvalueType(lvalueType) {}

    std::shared_ptr<IRValueType> IROperand::getLvalueType() {
        return lvalueType;
    }

    yoi::wstr IROperand::to_string() const {
        switch (type) {
            case operandType::integer:
                return L"int(" + std::to_wstring(value.integer) + L")";
            case operandType::decimal:
                return L"double(" + std::to_wstring(value.decimal) + L")";
            case operandType::boolean:
                return L"bool(" + yoi::wstr(value.boolean ? L"true" : L"false") + L")";
            case operandType::character:
                return L"char(" + yoi::wstr(1, value.character) + L")";
            case operandType::stringLiteral:
                return L"string_const#" + std::to_wstring(value.stringLiteralIndex);
            case operandType::codeBlock:
                return L"codeBlock#" + std::to_wstring(value.codeBlockIndex);
            case operandType::index:
                return L"index(" + std::to_wstring(value.symbolIndex) + L")";
            case operandType::localVar:
                return L"localVar#" + std::to_wstring(value.symbolIndex);
            case operandType::globalVar:
                return L"globalVar#" + std::to_wstring(value.symbolIndex);
            case operandType::externVar:
                return L"externVar#" + std::to_wstring(value.symbolIndex);
            case operandType::shortInt:
                return L"short(" + std::to_wstring(value.shortV) + L")";
            case operandType::unsignedInt:
                return L"unsigned(" + std::to_wstring(value.unsignedV) + L")";
            default:
                return L"unknown";
        }
    }

    IR::IR(IR::Opcode opcode, const vec<IROperand> &operands, IRDebugInfo debugInfo) : opcode(opcode), operands(operands), debugInfo(debugInfo) {}

    yoi::wstr IR::to_string() const {
        yoi::wstr r;
        r += string2wstring(magic_enum::enum_name<>(opcode).data());
        r += L" ";
        for (auto &operand : operands) {
            r += operand.to_string();
            r += L" ";
        }
        return r;
    }

    IRInterfaceImplementationDefinition::IRInterfaceImplementationDefinition(
        const yoi::wstr &name,
        std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex,
        const std::pair<yoi::indexT, yoi::indexT> &implInterfaceIndex,
        const yoi::vec<std::shared_ptr<IRValueType>> &virtualMethods,
        const std::map<yoi::wstr, yoi::indexT> &virtualMethodIndexMap)
        : name(name), implStructIndex(implStructIndex), virtualMethods(virtualMethods), virtualMethodIndexMap(virtualMethodIndexMap),
          implInterfaceIndex(implInterfaceIndex) {}

    IRInterfaceInstanceDefinition::IRInterfaceInstanceDefinition(const yoi::wstr &name,
                                                                 const std::map<yoi::wstr, yoi::vec<yoi::indexT>> &functionOverloadIndexies,
                                                                 const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> &methodMap)
        : name(name), functionOverloadIndexies(functionOverloadIndexies), methodMap(methodMap) {}

    yoi::wstr IRModule::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"Module#" + std::to_wstring(identifier) + L" {\n";
        for (auto &function : functionTable) {
            r += function.second->to_string(indent + 4);
        }
        for (auto &structType : structTable) {
            r += structType.second->to_string(indent + 4);
        }
        for (auto &interfaceType : interfaceTable) {
            r += interfaceType.second->to_string(indent + 4);
        }
        for (auto &interfaceImpl : interfaceImplementationTable) {
            r += interfaceImpl.second->to_string(indent + 4);
        }
        r += yoi::wstr(indent, L' ') + L"}\n";
        return r;
    }

    IRBuilder::IRBuilder(std::shared_ptr<compilerContext> compilerCtx,
                         std::shared_ptr<IRModule> currentModule,
                         std::shared_ptr<IRFunctionDefinition> currentFunction)
        : compilerCtx(compilerCtx), currentModule(currentModule), currentFunction(currentFunction), currentCodeBlockIndex(0), currentDebugInfo() {}

    yoi::indexT IRBuilder::createCodeBlock() {
        codeBlocks.emplace_back(std::make_shared<IRCodeBlock>(IRCodeBlock{}));
        return (yoi::indexT)codeBlocks.size() - 1;
    }

    IRCodeBlock &IRBuilder::getCodeBlock(yoi::indexT index) {
        return *codeBlocks[index];
    }

    void IRBuilder::yield() {
        currentFunction->codeBlock = std::move(codeBlocks);
        currentFunction = nullptr;
        codeBlocks.clear();
    }

    IRCodeBlock &IRBuilder::getCurrentCodeBlock() {
        return *codeBlocks[currentCodeBlockIndex];
    }

    yoi::IROperand IRBuilder::createLocalVar(const yoi::wstr &varName, const std::shared_ptr<IRValueType> &type) {
        auto idx = currentFunction->getVariableTable().put(varName, type);
        return {IROperand::operandType::localVar, {idx}};
    }

    void IRBuilder::insert(const IR &ir, yoi::indexT insertionPoint) {
        if (insertionPoint == 0xffffffff) {
            getCurrentCodeBlock().getIRArray().push_back(ir);
        } else {
            auto insPoint = getCurrentCodeBlock().getIRArray().begin() + insertionPoint;
            // if (insPoint != getCurrentCodeBlock().getIRArray().end()) {
            //     insPoint++;
            // }
            getCurrentCodeBlock().getIRArray().insert(insPoint, ir);
        }
    }

    std::shared_ptr<IRValueType> &IRBuilder::getLhsFromTempVarStack() {
        yoi_assert(tempVarStack.size() > 1, currentDebugInfo.line, currentDebugInfo.column, "tempVarStack is empty.");
        return tempVarStack[tempVarStack.size() - 2];
    }

    std::shared_ptr<IRValueType> &IRBuilder::getRhsFromTempVarStack() {
        yoi_assert(tempVarStack.size() > 0, currentDebugInfo.line, currentDebugInfo.column, "tempVarStack is empty.");
        return tempVarStack[tempVarStack.size() - 1];
    }

    void IRBuilder::basicCast(const std::shared_ptr<IRValueType> &valType, yoi::indexT insertionPoint, bool lhs) {
        auto &target = tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1);
        switch (valType->type) {
            case IRValueType::valueType::integerObject:
                yoi_assert(target->type != IRValueType::valueType::stringObject,
                           currentDebugInfo.line,
                           currentDebugInfo.column,
                           "Type mismatch in basicCast.");
                insert({IR::Opcode::basic_cast_int, {}, currentDebugInfo}, insertionPoint);
                target = compilerCtx->getIntObjectType();
                break;
            case IRValueType::valueType::decimalObject:
                yoi_assert(target->type != IRValueType::valueType::stringObject,
                           currentDebugInfo.line,
                           currentDebugInfo.column,
                           "Type mismatch in basicCast.");
                insert({IR::Opcode::basic_cast_deci, {}, currentDebugInfo}, insertionPoint);
                target = compilerCtx->getDeciObjectType();
                break;
            case IRValueType::valueType::characterObject:
                yoi_assert(target->type != IRValueType::valueType::stringObject,
                           currentDebugInfo.line,
                           currentDebugInfo.column,
                           "Type mismatch in basicCast.");
                insert({IR::Opcode::basic_cast_char, {}, currentDebugInfo}, insertionPoint);
                target = compilerCtx->getCharObjectType();
                break;
            case IRValueType::valueType::booleanObject:
                yoi_assert(target->type != IRValueType::valueType::stringObject,
                           currentDebugInfo.line,
                           currentDebugInfo.column,
                           "Type mismatch in basicCast.");
                insert({IR::Opcode::basic_cast_bool, {}, currentDebugInfo}, insertionPoint);
                target = compilerCtx->getBoolObjectType();
                break;
            case IRValueType::valueType::pointerObject:
                insert({IR::Opcode::pointer_cast, {}, currentDebugInfo}, insertionPoint);
                target = managedPtr(IRValueType{IRValueType::valueType::pointerObject});
                break;
            case IRValueType::valueType::shortObject:
                yoi_assert(target->type != IRValueType::valueType::stringObject,
                           currentDebugInfo.line,
                           currentDebugInfo.column,
                           "Type mismatch in basicCast.");
                insert({IR::Opcode::basic_cast_short, {}, currentDebugInfo}, insertionPoint);
                target = compilerCtx->getShortObjectType();
                break;
            case IRValueType::valueType::unsignedObject:
                yoi_assert(target->type != IRValueType::valueType::stringObject,
                           currentDebugInfo.line,
                           currentDebugInfo.column,
                           "Type mismatch in basicCast.");
                insert({IR::Opcode::basic_cast_unsigned, {}, currentDebugInfo}, insertionPoint);
                target = compilerCtx->getUnsignedObjectType();
                break;
            default: {
                panic(currentDebugInfo.line,
                      currentDebugInfo.column,
                      "Unsupported type for basicCast: " + std::string{magic_enum::enum_name<>(valType->type)});
                break;
            }
        }
    }

    void IRBuilder::uniqueArithmeticOp(IR::Opcode op) {
        // fetch lhs from tempVarStack
        auto left = tempVarStack.back();
        tempVarStack.pop_back();
        // push result to tempVarStack
        tempVarStack.emplace_back(left);
        insert({op, {}, currentDebugInfo});
    }

    void IRBuilder::arithmeticOp(IR::Opcode op) {
        // fetch lhs and rhs from tempVarStack
        auto right = tempVarStack.back();
        tempVarStack.pop_back();
        auto left = tempVarStack.back();
        tempVarStack.pop_back();
        yoi_assert(left->type == right->type,
                   currentDebugInfo.line,
                   currentDebugInfo.column,
                   "Type mismatch in arithmetic operation: " + wstring2string(left->to_string()) + " " + std::string{magic_enum::enum_name<>(op)} +
                       " " + wstring2string(right->to_string()));
        // push result to tempVarStack
        switch (op) {
            case IR::Opcode::add:
            case IR::Opcode::sub:
            case IR::Opcode::mul:
            case IR::Opcode::div:
            case IR::Opcode::mod:
            case IR::Opcode::left_shift:
            case IR::Opcode::right_shift:
            case IR::Opcode::bitwise_not:
            case IR::Opcode::bitwise_and:
            case IR::Opcode::bitwise_or:
            case IR::Opcode::bitwise_xor: {
                tempVarStack.emplace_back(left);
                break;
            }
            case IR::Opcode::less_than:
            case IR::Opcode::greater_than:
            case IR::Opcode::less_equal:
            case IR::Opcode::greater_equal:
            case IR::Opcode::equal:
            case IR::Opcode::not_equal: {
                tempVarStack.emplace_back(compilerCtx->getBoolObjectType());
                break;
            }
            default: {
                panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported type for arithmetic operation.");
                break;
            }
        }
        insert({op, {}, currentDebugInfo});
    }

    void IRBuilder::jumpOp(yoi::indexT target) {
        if (hasTerminated())
            return;
        insert(IR(IR::Opcode::jump, {IROperand(IROperand::operandType::codeBlock, target)}, currentDebugInfo));
    }

    void IRBuilder::jumpIfOp(IR::Opcode op, yoi::indexT target) {
        // fetch condition from tempVarStack
        auto condition = tempVarStack.back();
        tempVarStack.pop_back();
        if (hasTerminated())
            return;
        yoi_assert(condition->type == IRValueType::valueType::booleanObject, 0, 0, "Type mismatch in jumpIf operation.");
        // insert jumpIf operation
        insert(IR(op, {IROperand(IROperand::operandType::codeBlock, target)}, currentDebugInfo));
    }

    void IRBuilder::pushOp(IR::Opcode op, const yoi::IROperand &constV) {
        if (op == IR::Opcode::push_null) {
            tempVarStack.emplace_back(managedPtr(IRValueType(IRValueType::valueType::pointerObject)));
        } else if (constV.type == IROperand::operandType::integer) {
            // tempVarStack.push_back()
            tempVarStack.emplace_back(compilerCtx->getIntObjectType());
        } else if (constV.type == IROperand::operandType::boolean) {
            tempVarStack.emplace_back(compilerCtx->getBoolObjectType());
        } else if (constV.type == IROperand::operandType::character) {
            tempVarStack.emplace_back(compilerCtx->getCharObjectType());
        } else if (constV.type == IROperand::operandType::decimal) {
            tempVarStack.emplace_back(compilerCtx->getDeciObjectType());
        } else if (constV.type == IROperand::operandType::stringLiteral) {
            tempVarStack.emplace_back(compilerCtx->getStrObjectType());
            insert({op, {{IROperand::operandType::index, currentModule->identifier}, constV}, currentDebugInfo});
            return;
        } else if (constV.type == IROperand::operandType::shortInt) {
            tempVarStack.emplace_back(compilerCtx->getShortObjectType());
        } else if (constV.type == IROperand::operandType::unsignedInt) {
            tempVarStack.emplace_back(compilerCtx->getUnsignedObjectType());
        } else {
            panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported constant type for pushOp");
        }
        insert({op, {constV}, currentDebugInfo});
    }

    void IRBuilder::loadOp(IR::Opcode op, const yoi::IROperand &source, const std::shared_ptr<IRValueType> &expectedType, yoi::indexT moduleIndex) {
        if (op == IR::Opcode::load_member || op == IR::Opcode::load_local) {
            insert({op, {source}, currentDebugInfo});
        } else if (op == IR::Opcode::load_element) {
            tempVarStack.pop_back();
            tempVarStack.pop_back();
            insert({op, {}, currentDebugInfo});
        } else if (moduleIndex == -1) {
            insert({op, {{IROperand::operandType::index, currentModule->identifier}, source}, currentDebugInfo});
        } else {
            insert({op, {{IROperand::operandType::index, moduleIndex}, source}, currentDebugInfo});
        }
        tempVarStack.emplace_back(expectedType);
    }

    void IRBuilder::loadMemberOp(const yoi::IROperand &memberIndex, const std::shared_ptr<IRValueType> &memberType) {
        tempVarStack.pop_back();
        tempVarStack.emplace_back(memberType);
        insert({IR::Opcode::load_member, {memberIndex}, currentDebugInfo});
    }

    void IRBuilder::storeOp(IR::Opcode op, const yoi::IROperand &operand, yoi::indexT moduleIndex) {
        switch (op) {
            case IR::Opcode::store_local:
            case IR::Opcode::store_element:
            case IR::Opcode::store_member: 
            case IR::Opcode::store_field: {
                // fetch rhs from tempVarStack
                tempVarStack.pop_back();
                insert({op, {operand}, currentDebugInfo});
                break;
            }
            case IR::Opcode::store_global: {
                // fetch rhs from tempVarStack
                tempVarStack.pop_back();
                insert(
                    {op, {{IROperand::operandType::index, moduleIndex == -1 ? currentModule->identifier : moduleIndex}, operand}, currentDebugInfo});
                break;
            }
            default: {
                panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported operand type for storeOp");
                break;
            }
        }
    }

    void IRBuilder::storeMemberOp(const yoi::IROperand &memberIndex) {
        // pop rhs and lhs from tempVarStack
        tempVarStack.pop_back();
        tempVarStack.pop_back();
        insert({IR::Opcode::store_member, {memberIndex}, currentDebugInfo});
    }

    void IRBuilder::invokeOp(yoi::indexT funcIndex,
                             yoi::indexT funcArgsCount,
                             const std::shared_ptr<IRValueType> &returnType,
                             bool externalInvocation,
                             yoi::indexT moduleIndex) {
        for (yoi::indexT i = 0; i < funcArgsCount; i++) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(returnType);
        insert(IR(IR::Opcode::invoke,
                  {{IROperand::operandType::index, externalInvocation ? moduleIndex : currentModule->identifier},
                   {IROperand::operandType::index, funcIndex},
                   {IROperand::operandType::index, funcArgsCount}},
                  currentDebugInfo));
    }

    void IRBuilder::invokeMethodOp(yoi::indexT funcIndex,
                                   yoi::indexT methodArgsCount,
                                   const std::shared_ptr<IRValueType> &returnType,
                                   bool isStatic,
                                   bool externalInvocation,
                                   yoi::indexT moduleIndex) {
        // this pointer is popped from tempVarStack
        for (yoi::indexT i = 0; i < methodArgsCount + 1; i++) {
            tempVarStack.pop_back();
        }
        // pop this pointer from tempVarStack, the instructions related to `this` pointer should be eradicated during IROptimizer.
        if (isStatic)
            insert(IR{IR::Opcode::pop, {}, currentDebugInfo});

        tempVarStack.push_back(returnType);
        insert(IR(IR::Opcode::invoke,
                  {{IROperand::operandType::index, externalInvocation ? moduleIndex : currentModule->identifier},
                   {IROperand::operandType::index, funcIndex},
                   {IROperand::operandType::index, methodArgsCount + 1 - isStatic}},
                  currentDebugInfo));
    }

    void IRBuilder::invokeVirtualOp(yoi::indexT funcIndex,
                                    yoi::indexT interfaceIndex,
                                    yoi::indexT methodArgsCount,
                                    const std::shared_ptr<IRValueType> &returnType,
                                    bool externalInvocation,
                                    yoi::indexT moduleIndex) {
        for (yoi::indexT i = 0; i < methodArgsCount + 1; i++) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(returnType);
        insert(IR(IR::Opcode::invoke_virtual,
                  {{IROperand::operandType::index, externalInvocation ? moduleIndex : currentModule->identifier},
                   {IROperand::operandType::index, interfaceIndex},
                   {IROperand::operandType::index, funcIndex},
                   {IROperand::operandType::index, methodArgsCount + 1}},
                  currentDebugInfo));
    }

    void IRBuilder::retOp(bool returnWithNone) {
        // fetch return value from tempVarStack
        if (returnWithNone) {
            if (hasTerminated())
                return;
            insert(IR(IR::Opcode::ret_none, {}, currentDebugInfo));
            return;
        }
        auto retValue = tempVarStack.back();
        tempVarStack.pop_back();
        if (hasTerminated())
            return;
        insert(IR(IR::Opcode::ret, {}, currentDebugInfo));
    }

    void IRBuilder::newStructOp(yoi::indexT structIndex, bool isExternal, yoi::indexT moduleIndex) {
        insert(IR{IR::Opcode::new_struct,
                  {IROperand(IROperand::operandType::index, isExternal ? moduleIndex : currentModule->identifier),
                   IROperand(IROperand::operandType::index, structIndex)},
                  currentDebugInfo});
        tempVarStack.emplace_back(
            managedPtr(IRValueType{IRValueType::valueType::structObject, isExternal ? moduleIndex : currentModule->identifier, structIndex}));
    }

    void IRBuilder::newDataStructOp(yoi::indexT structIndex, bool isExternal, yoi::indexT moduleIndex) {
        insert(IR{IR::Opcode::new_datastruct,
                  {IROperand(IROperand::operandType::index, isExternal ? moduleIndex : currentModule->identifier),
                   IROperand(IROperand::operandType::index, structIndex)},
                  currentDebugInfo});
        tempVarStack.emplace_back(
            managedPtr(IRValueType{IRValueType::valueType::datastructObject, isExternal ? moduleIndex : currentModule->identifier, structIndex}));
    }

    void IRBuilder::constructInterfaceImplOp(const std::pair<yoi::indexT, yoi::indexT> &interfaceId,
                                             yoi::indexT interfaceImplIndex,
                                             bool isExternal,
                                             yoi::indexT moduleIndex) {
        auto rhs = managedPtr(IRValueType{IRValueType::valueType::interfaceObject, interfaceId.first, interfaceId.second});
        this->tempVarStack.pop_back(); // remove structObject from tempVarStack
        tempVarStack.push_back(rhs);
        insert(IR{IR::Opcode::construct_interface_impl,
                  {IROperand(IROperand::operandType::index, moduleIndex == -1 ? currentModule->identifier : moduleIndex),
                   IROperand(IROperand::operandType::index, interfaceImplIndex)},
                  currentDebugInfo});
    }

    yoi::indexT IRBuilder::getCurrentInsertionPoint() {
        return (yoi::indexT)getCurrentCodeBlock().getIRArray().size();
    }

    void IRBuilder::popFromTempVarStack() {
        tempVarStack.pop_back();
    }

    yoi::indexT IRBuilder::switchCodeBlock(yoi::indexT index) {
        auto res = currentCodeBlockIndex;
        currentCodeBlockIndex = index;
        return res;
    }

    yoi::indexT IRBuilder::getCurrentCodeBlockIndex() {
        return currentCodeBlockIndex;
    }

    std::shared_ptr<IRFunctionDefinition> IRBuilder::irFuncDefinition() {
        return currentFunction;
    }

    void IRCodeBlock::insert(const IR &ir) {
        codeBlock.emplace_back(ir);
    }

    yoi::wstr IRCodeBlock::to_string(yoi::indexT indent) {
        yoi::wstr r;
        for (auto &ir : codeBlock) {
            r += yoi::wstr(indent, L' ') + ir.to_string() + L"\n";
        }
        return r;
    }

    yoi::vec<IR> &IRCodeBlock::getIRArray() {
        return codeBlock;
    }

    yoi::wstr IRFunctionDefinition::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"func " + name + L"(";
        if (!argumentTypes.empty()) {
            for (auto it = argumentTypes.begin(); it != argumentTypes.end() - 1; ++it) {
                r += (*it)->to_string(true) + L", ";
            }
            r += argumentTypes.back()->to_string(true);
        }
        r += L") : " + returnType->to_string(true) + L" {\n";
        r += variableTable.to_string(indent + 4);
        for (auto idx = 0; idx < codeBlock.size(); ++idx) {
            r += yoi::wstr(indent + 4, L' ') + L"block#" + std::to_wstring(idx) + L":\n";
            r += codeBlock[idx]->to_string(indent + 8);
        }
        r += yoi::wstr(indent, L' ') + L"}\n";
        return r;
    }

    IRFunctionDefinition::Builder &IRFunctionDefinition::Builder::setName(const yoi::wstr &name) {
        this->name = name;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &IRInterfaceImplementationDefinition::Builder::setName(const yoi::wstr &interfaceName) {
        this->name = interfaceName;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::setImplStructIndex(std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex) {
        this->implStructIndex = implStructIndex;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::setImplInterfaceIndex(const std::pair<yoi::indexT, yoi::indexT> &implInterfaceIndex) {
        this->implInterfaceIndex = implInterfaceIndex;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::addVirtualMethod(const yoi::wstr &methodName, const std::shared_ptr<IRValueType> &methodType) {
        this->virtualMethods.emplace_back(methodType);
        this->virtualMethodIndexMap[methodName] = this->virtualMethods.size() - 1;
        return *this;
    }

    std::shared_ptr<IRInterfaceImplementationDefinition> IRInterfaceImplementationDefinition::Builder::yield() {
        return std::make_shared<IRInterfaceImplementationDefinition>(IRInterfaceImplementationDefinition{
            std::move(name), implStructIndex, implInterfaceIndex, std::move(virtualMethods), std::move(virtualMethodIndexMap)});
    }

    IRInterfaceInstanceDefinition::Builder &IRInterfaceInstanceDefinition::Builder::setName(const yoi::wstr &interfaceName) {
        this->name = interfaceName;
        return *this;
    }

    IRInterfaceInstanceDefinition::Builder &IRInterfaceInstanceDefinition::Builder::addMethod(
        const yoi::wstr &methodNameOri, const yoi::wstr &methodName, const std::shared_ptr<IRFunctionDefinition> &methodSignature) {
        this->functionOverloadIndexies[methodNameOri].push_back(this->methodMap.put_create(methodName, methodSignature));
        return *this;
    }

    std::shared_ptr<IRInterfaceInstanceDefinition> IRInterfaceInstanceDefinition::Builder::yield() {
        return std::make_shared<IRInterfaceInstanceDefinition>(std::move(name), std::move(functionOverloadIndexies), std::move(methodMap));
    }

    IRFunctionDefinition::Builder &IRFunctionDefinition::Builder::addArgument(const yoi::wstr &argumentName,
                                                                              const std::shared_ptr<IRValueType> &argumentType) {
        this->argumentTypes.emplace_back(argumentName, argumentType);
        return *this;
    }

    IRFunctionDefinition::Builder &IRFunctionDefinition::Builder::setReturnType(const std::shared_ptr<IRValueType> &returnType) {
        this->returnType = returnType;
        return *this;
    }

    std::shared_ptr<IRFunctionDefinition> IRFunctionDefinition::Builder::yield() {
        return managedPtr(IRFunctionDefinition{name, argumentTypes, returnType, {}, attrs, debugInfo});
    }

    IRFunctionDefinition::IRFunctionDefinition(const yoi::wstr &name,
                                               const yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> &argumentTypes,
                                               const std::shared_ptr<IRValueType> &returnType,
                                               const yoi::vec<std::shared_ptr<IRCodeBlock>> &codeBlock,
                                               const std::set<FunctionAttrs> &attrs,
                                               const IRDebugInfo &debugInfo)
        : name(name), returnType(returnType), variableTable(), codeBlock(), debugInfo(debugInfo), attrs(attrs) {
        variableTable.createScope();
        for (auto &i : argumentTypes) {
            variableTable.put(i.first, i.second);
            this->argumentTypes.push_back(i.second);
        }
    }

    IRVariableTable &IRFunctionDefinition::getVariableTable() {
        return variableTable;
    }

    IRValueType::IRValueType(IRValueType::valueType type, yoi::indexT typeAffiliateModule, yoi::indexT objectPrototypeIndex)
        : type(type), typeAffiliateModule(typeAffiliateModule), typeIndex(objectPrototypeIndex), dimensions() {}

    IRValueType::IRValueType(IRValueType::valueType type) : type(type), typeIndex(0), typeAffiliateModule(0), dimensions() {}

    bool IRValueType::isBasicType() const {
        return type == valueType::integerObject || type == valueType::decimalObject || type == valueType::booleanObject ||
               type == valueType::stringObject || type == valueType::characterObject || type == valueType::foreignFloatType ||
               type == valueType::foreignInt32Type || type == valueType::shortObject || type == valueType::unsignedObject ||
               type == valueType::pointer || type == valueType::datastructObject;
    }

    bool IRValueType::isForeignBasicType() const {
        return type == valueType::foreignFloatType || type == valueType::foreignInt32Type || type == valueType::pointer;
    }

    bool IRValueType::is1ByteType() const {
        return type == valueType::booleanObject || type == valueType::characterObject;
    }

    IRValueType IRValueType::getNormalizedForeignBasicType() {
        auto res = *this;
        switch (type) {
            case IRValueType::valueType::foreignFloatType:
                res.type = valueType::decimalObject;
                break;
            case IRValueType::valueType::foreignInt32Type:
                res.type = valueType::integerObject;
                break;
            case IRValueType::valueType::pointer:
                res.type = valueType::unsignedObject;
                break;
            default:
                break;
        }
        return res;
    }

    yoi::wstr IRValueType::to_string(bool showAttributes) const {
        yoi::wstr res;
        switch (type) {
            case valueType::integerRaw:
                res = L"int_literal";
                break;
            case valueType::decimalRaw:
                res = L"deci_literal";
                break;
            case valueType::booleanRaw:
                res = L"bool_literal";
                break;
            case valueType::shortRaw:
                res = L"short_literal";
                break;
            case valueType::unsignedRaw:
                res = L"unsigned_literal";
                break;
            case valueType::characterObject:
                res = L"char";
                break;
            case valueType::stringLiteral:
                res = L"string";
                break;
            case valueType::structObject:
                res = L"struct#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
                break;
            case valueType::null:
                res = L"null";
                break;
            case valueType::integerObject:
                res = L"int";
                break;
            case valueType::booleanObject:
                res = L"bool";
                break;
            case valueType::decimalObject:
                res = L"decimal";
                break;
            case valueType::shortObject:
                res = L"short";
                break;
            case valueType::unsignedObject:
                res = L"unsigned";
                break;
            case valueType::stringObject:
                res = L"string";
                break;
            case valueType::none:
                res = L"none";
                break;
            case valueType::interfaceObject:
                res = L"interface#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
                break;
            case valueType::pointerObject:
                res = L"pointerObject";
                break;
            case valueType::pointer:
                res = L"pointer";
                break;
            case valueType::virtualMethod:
                res = L"virtual_method#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
                break;
            case valueType::incompleteTemplateType:
                res = L"incomplete_template_type#" + std::to_wstring(typeIndex);
                break;
            case valueType::datastructObject:
                res = L"datastruct#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
                break;
            default:
                res = L"unknown";
                break;
        }

        if (!dimensions.empty()) {
            if (dimensions.back() == -1)
                res += L"[]";
            else
                for (auto d : dimensions)
                    res += L"[" + std::to_wstring(d) + L"]";
        }

        if (showAttributes)
            for (auto &i : attributes)
                res += L" @" + string2wstring(std::string{magic_enum::enum_name(i)});

        if (showAttributes)
            res += L" " + metadata.to_string();

        return res;
    }

    bool IRValueType::operator==(const yoi::IRValueType &rhs) const {
        return type == rhs.type && typeIndex == rhs.typeIndex && typeAffiliateModule == rhs.typeAffiliateModule &&
               dimensions.size() == rhs.dimensions.size();
    }

    IRStructDefinition::IRStructDefinition(const yoi::wstr &name,
                                           const std::map<yoi::wstr, nameInfo> &nameInfoMap,
                                           const vec<std::shared_ptr<IRValueType>> &fieldTypes,
                                           const yoi::vec<yoi::wstr> &templateParamNames,
                                           const yoi::vec<std::shared_ptr<IRValueType>> &storedTemplateArgs,
                                           const std::map<yoi::wstr, yoi::structDefInnerPair *> &templateMethodDecls,
                                           const std::map<yoi::wstr, yoi::implInnerPair *> &templateMethodDefs)
        : name(name), nameIndexMap(nameInfoMap), fieldTypes(fieldTypes), templateParamNames(templateParamNames),
          storedTemplateArgs(storedTemplateArgs), templateMethodDecls(templateMethodDecls), templateMethodDefs(templateMethodDefs) {}

    yoi::wstr IRStructDefinition::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"struct " + name + L" {\n";
        for (auto &i : nameIndexMap) {
            if (i.second.type == nameInfo::nameType::field) {
                r += yoi::wstr(indent + 4, L' ') + i.first + L" " + fieldTypes[i.second.index]->to_string(true) + L"\n";
            }
        }
        r += yoi::wstr(indent, L' ') + L"}\n";
        return r;
    }

    const IRStructDefinition::nameInfo &IRStructDefinition::lookupName(const wstr &name) {
        try {
            return nameIndexMap.at(name);
        } catch (std::out_of_range &e) {
            throw std::out_of_range("Undefined field: " + yoi::wstring2string(name));
        }
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::setName(const yoi::wstr &name) {
        this->name = name;
        return *this;
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::addField(const yoi::wstr &fieldName, const std::shared_ptr<IRValueType> &fieldType) {
        this->fieldTypes.push_back(fieldType);
        nameIndexMap[fieldName] = nameInfo{nameInfo::nameType::field, this->fieldTypes.size() - 1};
        return *this;
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::addMethod(const yoi::wstr &methodName, yoi::indexT index) {
        nameIndexMap[methodName] = nameInfo{nameInfo::nameType::method, index};
        return *this;
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::setStoredTemplateArgs(const yoi::vec<yoi::wstr> &paramNames,
                                                                                    const yoi::vec<std::shared_ptr<IRValueType>> &args) {
        this->templateParamNames = paramNames;
        this->storedTemplateArgs = args;
        return *this;
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::addTemplateMethodDecl(const yoi::wstr &name, yoi::structDefInnerPair *decl) {
        this->templateMethodDecls[name] = decl;
        return *this;
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::addTemplateMethodDef(const yoi::wstr &name, yoi::implInnerPair *def) {
        this->templateMethodDefs[name] = def;
        return *this;
    }

    std::shared_ptr<IRStructDefinition> IRStructDefinition::Builder::yield() {
        return std::make_shared<IRStructDefinition>(std::move(name),
                                                    std::move(nameIndexMap),
                                                    std::move(fieldTypes),
                                                    std::move(templateParamNames),
                                                    std::move(storedTemplateArgs),
                                                    std::move(templateMethodDecls),
                                                    std::move(templateMethodDefs));
    }

    yoi::indexT IRStringLiteralPool::addStringLiteral(const wstr &str) {
        return pool.put(str);
    }

    yoi::wstr &IRStringLiteralPool::getStringLiteral(yoi::indexT index) {
        return pool[index];
    }

    yoi::indexT IRVariableTable::lookup(const wstr &name) {
        for (auto &it : std::ranges::reverse_view(variableNameIndexMap)) {
            if (auto item = it.find(name); item != it.end()) {
                return item->second;
            }
        }
        throw std::out_of_range("Undefined variable: " + yoi::wstring2string(name));
    }

    yoi::indexT IRVariableTable::createScope() {
        variableNameIndexMap.emplace_back();
        return (yoi::indexT)variableNameIndexMap.size() - 1;
    }

    void IRVariableTable::popScope() {
        variableNameIndexMap.pop_back();
    }

    yoi::wstr IRVariableTable::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"Variables {\n";
        for (int64_t i = 0; i < variables.size(); ++i) {
            r += yoi::wstr(indent + 4, L' ') + L"#" + std::to_wstring(i) + L" " + reversedVariableNameMap[i] + L"(scope#" +
                 std::to_wstring(variableScopeMap[i]) + L") : " + variables[i]->to_string(true) + L"\n";
        }
        return r + yoi::wstr(indent, L' ') + L"}\n";
    }

    std::shared_ptr<IRValueType> IRVariableTable::get(yoi::indexT index) {
        return variables[index];
    }

    std::shared_ptr<IRValueType> IRVariableTable::operator[](const wstr &name) {
        return variables[lookup(name)];
    }

    yoi::indexT IRVariableTable::put(const wstr &name, const std::shared_ptr<IRValueType> &type) {
        variables.emplace_back(type);
        variableNameIndexMap.back()[name] = variables.size() - 1;
        variableScopeMap[variables.size() - 1] = variableNameIndexMap.size() - 1;
        reversedVariableNameMap[variables.size() - 1] = name;
        return variables.size() - 1;
    }

    IRExternEntry::IRExternEntry(externType type, const yoi::wstr &name, yoi::indexT affiliateModule, yoi::indexT itemIndex)
        : type(type), name(name), affiliateModule(affiliateModule), itemIndex(itemIndex) {}

    IRExternEntry::externType IRExternEntry::getExternType() const {
        return type;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setBuildType(BuildType buildType) {
        this->buildType = buildType;
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setBuildPlatform(const yoi::wstr &buildPlatform) {
        this->buildPlatform = buildPlatform;
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setBuildArch(const yoi::wstr &buildArch) {
        this->buildArch = buildArch;
        return *this;
    }

    std::shared_ptr<IRBuildConfig> IRBuildConfig::Builder::yield() {
        return managedPtr(IRBuildConfig{buildType,
                                        buildMode,
                                        useObjectLinker,
                                        buildPlatform,
                                        buildArch,
                                        targetTriple,
                                        preserveIntermediateFiles,
                                        searchPaths,
                                        additionalLinkingFiles,
                                        additionalLinkerOptions,
                                        marcos,
                                        buildCachePath,
                                        immediatelyClearupCache});
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setBuildMode(BuildMode buildMode) {
        this->buildMode = buildMode;
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setUseObjectLinker(UseObjectLinker useObjectLinker) {
        this->useObjectLinker = useObjectLinker;
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setPreserveIntermediateFiles(bool preserveIntermediateFiles) {
        this->preserveIntermediateFiles = preserveIntermediateFiles;
        return *this;
    }

    yoi::wstr IRInterfaceInstanceDefinition::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"interface " + name + L" {\n";
        for (auto &i : methodMap) {
            r += i.second->to_string(indent + 4) + L"\n";
        }
        r += yoi::wstr(indent, L' ') + L"}\n";
        return r;
    }

    yoi::wstr IRInterfaceImplementationDefinition::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"impl " + name + L" for " +
             yoi::string2wstring(std::string{magic_enum::enum_name(std::get<0>(implStructIndex))}) + L"#" +
             std::to_wstring(std::get<1>(implStructIndex)) + L"#" + std::to_wstring(std::get<2>(implStructIndex)) + L" {\n";

        for (auto &i : virtualMethods) {
            r += yoi::wstr(indent + 4, L' ') + L"virtual " + i->to_string() + L"\n";
        }
        r += yoi::wstr(indent, L' ') + L"}\n";
        return r;
    }

    IRTemplateBuilder::Argument::Argument(const std::shared_ptr<IRValueType> &templateType) : templateType(templateType) {}

    IRFunctionTemplate::IRFunctionTemplate(const std::shared_ptr<IRFunctionDefinition> &templateDefinition,
                                           const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
        : templateDefinition(templateDefinition), templateArguments(templateArguments) {}

    IRFunctionTemplate::Builder &IRFunctionTemplate::Builder::setTemplateDefinition(const std::shared_ptr<IRFunctionDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }

    std::shared_ptr<IRFunctionTemplate> IRFunctionTemplate::Builder::yield() {
        return std::make_shared<IRFunctionTemplate>(templateDefinition, templateArguments);
    }

    IRStructTemplate::Builder &IRStructTemplate::Builder::setTemplateDefinition(const std::shared_ptr<IRStructDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }

    std::shared_ptr<IRStructTemplate> IRStructTemplate::Builder::yield() {
        return std::make_shared<IRStructTemplate>(IRStructTemplate{templateDefinition, templateMethods, templateArguments});
    }

    IRInterfaceInstanceTemplate::IRInterfaceInstanceTemplate(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition,
                                                             const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
        : templateDefinition(templateDefinition), templateArguments(templateArguments) {}

    IRInterfaceInstanceTemplate::Builder &
    IRInterfaceInstanceTemplate::Builder::setTemplateDefinition(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }

    std::shared_ptr<IRInterfaceInstanceTemplate> IRInterfaceInstanceTemplate::Builder::yield() {
        return std::make_shared<IRInterfaceInstanceTemplate>(templateDefinition, templateArguments);
    }

    IRInterfaceImplementationTemplate::IRInterfaceImplementationTemplate(
        const std::shared_ptr<IRInterfaceImplementationDefinition> &templateDefinition,
        const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
        : templateDefinition(templateDefinition), templateArguments(templateArguments) {}

    IRInterfaceImplementationTemplate::Builder &IRInterfaceImplementationTemplate::Builder::setTemplateDefinition(
        const std::shared_ptr<IRInterfaceImplementationDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }

    std::shared_ptr<IRInterfaceImplementationTemplate> IRInterfaceImplementationTemplate::Builder::yield() {
        return std::make_shared<IRInterfaceImplementationTemplate>(templateDefinition, templateArguments);
    }

    IRTemplateBuilder &IRTemplateBuilder::addTemplateArgument(const yoi::wstr &templateName,
                                                              const std::shared_ptr<IRValueType> &templateType,
                                                              const yoi::vec<externModuleAccessExpression *> &satisfyConditions) {
        templateArguments.put_create(templateName, {templateType, satisfyConditions});
        return *this;
    }

    IRStructTemplate::Builder &IRStructTemplate::Builder::setTemplateMethod(const yoi::wstr &methodName,
                                                                            const std::shared_ptr<IRFunctionTemplate> &methodTemplate) {
        // templateMethods[methodName] = methodTemplate;
        templateMethods.put_create(methodName, methodTemplate);
        return *this;
    }

    yoi::vec<std::shared_ptr<IRValueType>> &IRVariableTable::getVariables() {
        return variables;
    }

    std::map<yoi::indexT, yoi::wstr> &IRVariableTable::getReversedVariableNameMap() {
        return reversedVariableNameMap;
    }

    IRStructTemplate::IRStructTemplate(const std::shared_ptr<IRStructDefinition> &templateDefinition,
                                       const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionTemplate>> &templateMethods,
                                       const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
        : templateDefinition(templateDefinition), templateMethods(templateMethods), templateArguments(templateArguments) {}

    IRFFITable::ImportLibrary::ImportLibrary(const yoi::wstr &libraryPath) : libraryPath(libraryPath) {}

    yoi::indexT IRFFITable::addImportedFunction(const yoi::wstr &libraryName,
                                                const yoi::wstr &functionName,
                                                const std::shared_ptr<IRFunctionDefinition> &functionDefinition) {
        if (!importedLibraries.contains(libraryName)) {
            importedLibraries.put_create(libraryName, {libraryName});
        }

        return importedLibraries[libraryName].importedFunctionTable.put(functionName, functionDefinition);
    }

    void IRFFITable::addExportedFunction(const yoi::wstr &exportName,
                                         yoi::indexT moduleIndex,
                                         yoi::indexT functionIndex,
                                         const std::set<IRFunctionDefinition::FunctionAttrs> &attrs) {
        exportedFunctionTable.put_create(exportName, std::make_tuple(moduleIndex, functionIndex, attrs));
    }

    void IRFFITable::addForeignType(const yoi::wstr &foreignTypeName, const std::shared_ptr<IRValueType> &structType) {
        foreignTypeTable.put_create(foreignTypeName, structType);
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setSearchPaths(const yoi::vec<yoi::wstr> &searchPaths) {
        this->searchPaths = searchPaths;
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::addSearchPath(const yoi::wstr &searchPath) {
        searchPaths.push_back(searchPath);
        return *this;
    }

    void IRBuilder::invokeImportedOp(yoi::indexT libIndex,
                                     yoi::indexT funcIndex,
                                     yoi::indexT funcArgsCount,
                                     const std::shared_ptr<IRValueType> &returnType) {
        for (yoi::indexT i = 0; i < funcArgsCount; ++i) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(managedPtr(compilerCtx->normalizeForeignBasicType(returnType)));

        // insert(IR{IR::Opcode::invoke_imported, {IROperand(IROperand::operandType::index, externIndex),
        // IROperand(IROperand::operandType::index, funcArgsCount)}});
        insert(IR{IR::Opcode::invoke_imported,
                  {IROperand(IROperand::operandType::index, libIndex),
                   IROperand(IROperand::operandType::index, funcIndex),
                   IROperand(IROperand::operandType::index, funcArgsCount)},
                  currentDebugInfo});
    }

    bool IRValueType::isArrayType() const {
        return !dimensions.empty() && dimensions.back() != static_cast<yoi::indexT>(-1);
    }

    IRValueType::IRValueType(valueType type,
                             yoi::indexT typeAffiliateModule,
                             yoi::indexT objectPrototypeIndex,
                             const yoi::vec<yoi::indexT> &dimensions)
        : type(type), dimensions(dimensions), typeAffiliateModule(typeAffiliateModule), typeIndex(objectPrototypeIndex) {}

    IRValueType::IRValueType(valueType type, const yoi::vec<yoi::indexT> &dimensions)
        : type(type), dimensions(dimensions), typeAffiliateModule(0), typeIndex(0) {}

    IRValueType IRValueType::getElementType() {
        return {this->type, this->typeAffiliateModule, this->typeIndex, yoi::vec<yoi::indexT>{}};
    }

    void
    IRBuilder::newArrayOp(const std::shared_ptr<IRValueType> &elementType, const yoi::vec<yoi::indexT> &dimensions, yoi::indexT onstackElementCount) {
        IR::Opcode op = IR::Opcode::nop;
        yoi::vec<IROperand> operands;
        switch (elementType->type) {
            case IRValueType::valueType::integerObject:
                op = IR::Opcode::new_array_int;
                break;
            case IRValueType::valueType::booleanObject:
                op = IR::Opcode::new_array_bool;
                break;
            case IRValueType::valueType::decimalObject:
                op = IR::Opcode::new_array_deci;
                break;
            case IRValueType::valueType::stringObject:
                op = IR::Opcode::new_array_str;
                break;
            case IRValueType::valueType::characterObject:
                op = IR::Opcode::new_array_char;
                break;
            case IRValueType::valueType::shortObject:
                op = IR::Opcode::new_array_short;
                break;
            case IRValueType::valueType::unsignedObject:
                op = IR::Opcode::new_array_unsigned;
                break;
            case IRValueType::valueType::structObject:
                op = IR::Opcode::new_array_struct;
                operands.emplace_back(IROperand::operandType::index, elementType->typeAffiliateModule);
                operands.emplace_back(IROperand::operandType::index, elementType->typeIndex);
                break;
            case IRValueType::valueType::interfaceObject:
                op = IR::Opcode::new_array_interface;
                operands.emplace_back(IROperand::operandType::index, elementType->typeAffiliateModule);
                operands.emplace_back(IROperand::operandType::index, elementType->typeIndex);
                break;
            default:
                panic(currentDebugInfo.line,
                      currentDebugInfo.column,
                      "Unsupported array element type: " + yoi::wstring2string(elementType->to_string()));
                break;
        }
        auto size = 1;
        operands.emplace_back(IROperand::operandType::index, onstackElementCount);
        for (auto &dim : dimensions) {
            operands.emplace_back(IROperand::operandType::index, dim);
            size *= dim;
        }
        for (yoi::indexT i = 0; i < onstackElementCount; ++i) {
            tempVarStack.pop_back();
        }
        insert(IR{op, operands, currentDebugInfo});
        tempVarStack.push_back(managedPtr(elementType->getArrayType(dimensions)));
    }

    IRValueType IRValueType::getArrayType(const yoi::vec<yoi::indexT> &dimensions) {
        return {type, typeAffiliateModule, typeIndex, dimensions};
    }

    yoi::indexT IRBuilder::saveState() {
        codeBlockInsertionStates.emplace_back(currentCodeBlockIndex, codeBlocks[currentCodeBlockIndex]->getIRArray().size(), tempVarStack.size());
        return codeBlockInsertionStates.size() - 1;
    }

    void IRBuilder::discardState() {
        yoi_assert(!codeBlockInsertionStates.empty(), currentDebugInfo.line, currentDebugInfo.column, "Empty code states when poping back");
        codeBlockInsertionStates.pop_back();
    }

    void IRBuilder::restoreState() {
        yoi_assert(currentCodeBlockIndex == std::get<0>(codeBlockInsertionStates.back()),
                   currentDebugInfo.line,
                   currentDebugInfo.column,
                   "Invalid code block index");
        codeBlocks[currentCodeBlockIndex]->getIRArray().resize(std::get<1>(codeBlockInsertionStates.back()));
        tempVarStack.resize(std::get<2>(codeBlockInsertionStates.back()));
        yoi_assert(!codeBlockInsertionStates.empty(), currentDebugInfo.line, currentDebugInfo.column, "Empty code states when poping back");
        codeBlockInsertionStates.pop_back();
    }

    void IRBuilder::pushTempVar(const std::shared_ptr<IRValueType> &type) {
        tempVarStack.push_back(type);
    }

    void IRBuilder::popOp() {
        if (tempVarStack.back()->type == IRValueType::valueType::bracedInitalizerList) {
            auto elementCount = tempVarStack.back()->bracedTypes.size();
            for (yoi::indexT i = 0; i < elementCount; ++i) {
                tempVarStack.pop_back();
                insert(IR{IR::Opcode::pop, {}, currentDebugInfo});
            }
        } else {
            tempVarStack.pop_back();
            insert(IR{IR::Opcode::pop, {}, currentDebugInfo});
        }
    }

    void IRBuilder::setDebugInfo(const IRDebugInfo &debugInfo) {
        set_current_file_path(debugInfo.sourceFile);
        this->currentDebugInfo = debugInfo;
    }

    const IRDebugInfo &IRBuilder::getCurrentDebugInfo() {
        return currentDebugInfo;
    }

    IRFunctionDefinition::Builder &IRFunctionDefinition::Builder::setDebugInfo(const IRDebugInfo &debugInfo) {
        this->debugInfo = debugInfo;
        return *this;
    }

    void IRBuilder::typeIdOp() {
        auto rhs = tempVarStack.back();
        tempVarStack.pop_back();
        typeIdOp(rhs);
    }

    void IRBuilder::typeIdOp(const std::shared_ptr<IRValueType> &type) {
        IR::Opcode op;
        vec<IROperand> operand;
        if (type->isArrayType() || type->isDynamicArrayType()) {
            yoi::indexT size = 1;
            for (auto &dim : type->dimensions) {
                size *= dim;
            }

            op = IR::Opcode::typeid_object_non_stack;
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->type));
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->typeAffiliateModule));
            operand.emplace_back(IROperand::operandType::index, type->typeIndex);
            operand.emplace_back(IROperand::operandType::index, size);
        } else {
            op = IR::Opcode::typeid_object_non_stack;
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->type));
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->typeAffiliateModule));
            operand.emplace_back(IROperand::operandType::index, type->typeIndex);
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(0));
        }
        insert(IR(op, operand, currentDebugInfo));
        tempVarStack.push_back(managedPtr(IRValueType(IRValueType::valueType::integerObject)));
    }

    void IRBuilder::dynCastOp(const std::shared_ptr<IRValueType> &type) {
        IR::Opcode op;
        vec<IROperand> operand;

        op = IR::Opcode::dyn_cast_any;
        operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->type));
        operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->typeAffiliateModule));
        operand.emplace_back(IROperand::operandType::index, type->typeIndex);

        if (type->isDynamicArrayType() || type->isArrayType()) {
            yoi::indexT size = 1;
            for (auto &dim : type->dimensions) {
                size *= dim;
            }

            operand.emplace_back(IROperand::operandType::index, size);
        } else {
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(0));
        }
        insert(IR(op, operand, currentDebugInfo));
        tempVarStack.pop_back();
        tempVarStack.push_back(type);
    }

    IRFunctionDefinition::Builder &IRFunctionDefinition::Builder::addAttr(FunctionAttrs attr) {
        attrs.insert(attr);
        return *this;
    }

    void IRBuilder::pointerCastOp() {
        insert(IR{IR::Opcode::pointer_cast, {}, currentDebugInfo});
        tempVarStack.pop_back();
        tempVarStack.emplace_back(managedPtr(IRValueType{IRValueType::valueType::pointerObject}));
    }

    void IRBuilder::newDynamicArrayOp(const std::shared_ptr<IRValueType> &elementType, yoi::indexT initializerSize) {
        IR::Opcode op = IR::Opcode::nop;
        yoi::vec<IROperand> operands;
        switch (elementType->type) {
            case IRValueType::valueType::integerObject:
                op = IR::Opcode::new_dynamic_array_int;
                break;
            case IRValueType::valueType::booleanObject:
                op = IR::Opcode::new_dynamic_array_bool;
                break;
            case IRValueType::valueType::decimalObject:
                op = IR::Opcode::new_dynamic_array_deci;
                break;
            case IRValueType::valueType::shortObject:
                op = IR::Opcode::new_dynamic_array_short;
                break;
            case IRValueType::valueType::unsignedObject:
                op = IR::Opcode::new_dynamic_array_unsigned;
                break;
            case IRValueType::valueType::stringObject:
                op = IR::Opcode::new_dynamic_array_str;
                break;
            case IRValueType::valueType::characterObject:
                op = IR::Opcode::new_dynamic_array_char;
                break;
            case IRValueType::valueType::structObject:
                op = IR::Opcode::new_dynamic_array_struct;
                operands.emplace_back(IROperand::operandType::index, elementType->typeAffiliateModule);
                operands.emplace_back(IROperand::operandType::index, elementType->typeIndex);
                break;
            case IRValueType::valueType::interfaceObject:
                op = IR::Opcode::new_dynamic_array_interface;
                operands.emplace_back(IROperand::operandType::index, elementType->typeAffiliateModule);
                operands.emplace_back(IROperand::operandType::index, elementType->typeIndex);
                break;
            default:
                panic(currentDebugInfo.line,
                      currentDebugInfo.column,
                      "Unsupported array element type: " + yoi::wstring2string(elementType->to_string()));
                break;
        }
        operands.emplace_back(IROperand::operandType::index, initializerSize);
        insert(IR{op, operands, currentDebugInfo});
        for (yoi::indexT i = 0; i <= initializerSize; i++) {
            tempVarStack.pop_back(); // size and initializer
        }
        tempVarStack.push_back(managedPtr(elementType->getDynamicArrayType()));
    }

    IRValueType IRValueType::getDynamicArrayType() {
        return {type, typeAffiliateModule, typeIndex, {static_cast<yoi::indexT>(-1)}};
    }

    bool IRValueType::isDynamicArrayType() const {
        return dimensions.size() == 1 && dimensions.back() == static_cast<yoi::indexT>(-1);
    }

    void IRBuilder::arrayLengthOp() {
        tempVarStack.pop_back();
        tempVarStack.push_back(compilerCtx->getIntObjectType());
        insert(IR{IR::Opcode::array_length, {}, currentDebugInfo});
    }

    void IRBuilder::interfaceOfOp() {
        tempVarStack.pop_back();
        tempVarStack.pop_back();
        tempVarStack.push_back(compilerCtx->getBoolObjectType());
        insert(IR{IR::Opcode::interfaceof, {}, currentDebugInfo});
    }

    bool IRValueType::hasAttribute(ValueAttr attr) const {
        return attributes.count(attr) > 0;
    }

    IRValueType &IRValueType::removeAttribute(ValueAttr attr) {
        attributes.erase(attr);
        return *this;
    }

    IRValueType &IRValueType::addAttribute(ValueAttr attr) {
        attributes.insert(attr);
        return *this;
    }

    IRValueType::IRValueType(valueType type, yoi::indexT typeAffiliateModule, yoi::indexT objectPrototypeIndex, const std::set<ValueAttr> &attributes)
        : type(type), typeAffiliateModule(typeAffiliateModule), typeIndex(objectPrototypeIndex), dimensions(), attributes(attributes) {}

    bool IRValueType::isBasicRawType() const {
        return type == valueType::integerRaw || type == valueType::decimalRaw || type == valueType::booleanRaw || type == valueType::charRaw ||
               type == IRValueType::valueType::shortRaw || type == IRValueType::valueType::unsignedRaw || type == valueType::stringLiteral;
    }

    IRValueType IRValueType::getBasicRawType() const {
        IRValueType result = *this;
        switch (type) {
            case valueType::integerObject:
                result.type = valueType::integerRaw;
                break;
            case valueType::decimalObject:
                result.type = valueType::decimalRaw;
                break;
            case valueType::booleanObject:
                result.type = valueType::booleanRaw;
                break;
            case valueType::characterObject:
                result.type = valueType::charRaw;
                break;
            case IRValueType::valueType::shortObject:
                result.type = IRValueType::valueType::shortRaw;
                break;
            case IRValueType::valueType::unsignedObject:
                result.type = IRValueType::valueType::unsignedRaw;
                break;
            case valueType::stringObject:
                result.type = IRValueType::valueType::stringLiteral;
                break;
            default:
                break;
        }
        return result;
    }

    IRValueType IRValueType::getBasicObjectType() const {
        switch (type) {
            case valueType::integerRaw:
                return {valueType::integerObject, typeAffiliateModule, typeIndex, dimensions};
            case valueType::decimalRaw:
                return {valueType::decimalObject, typeAffiliateModule, typeIndex, dimensions};
            case valueType::booleanRaw:
                return {valueType::booleanObject, typeAffiliateModule, typeIndex, dimensions};
            case valueType::charRaw:
                return {valueType::characterObject, typeAffiliateModule, typeIndex, dimensions};
            case IRValueType::valueType::shortRaw:
                return {IRValueType::valueType::shortObject, typeAffiliateModule, typeIndex, dimensions};
            case IRValueType::valueType::unsignedRaw:
                return {IRValueType::valueType::unsignedObject, typeAffiliateModule, typeIndex, dimensions};
            case valueType::stringLiteral:
                return {valueType::stringObject, typeAffiliateModule, typeIndex, dimensions};
            default:
                return {type, typeAffiliateModule, typeIndex, dimensions};
        }
    }

    bool IRFunctionDefinition::hasAttribute(const FunctionAttrs &attr) {
        return std::find(attrs.begin(), attrs.end(), attr) != attrs.end();
    }

    IROperand::operandValue::operandValue(short shortV) {
        this->shortV = shortV;
    }

    void IRBuilder::restoreStateTemporarily() {
        auto current = codeBlockInsertionStates.back();
        yoi_assert(currentCodeBlockIndex == std::get<0>(current), currentDebugInfo.line, currentDebugInfo.column, "Invalid code block index");
        tempStateCodeBlock = std::vector<IR>(codeBlocks[currentCodeBlockIndex]->getIRArray().begin() + std::get<1>(current),
                                             codeBlocks[currentCodeBlockIndex]->getIRArray().end());
        tempStateTempVarStack = std::vector<std::shared_ptr<IRValueType>>(tempVarStack.begin() + std::get<2>(current), tempVarStack.end());

        codeBlocks[currentCodeBlockIndex]->getIRArray().resize(std::get<1>(current));
        tempVarStack.resize(std::get<2>(current));
    }

    void IRBuilder::commitState() {
        for (auto &block : tempStateCodeBlock) {
            codeBlocks[currentCodeBlockIndex]->getIRArray().push_back(block);
        }
        for (auto &type : tempStateTempVarStack) {
            tempVarStack.push_back(type);
        }
        tempStateCodeBlock.clear();
        tempStateTempVarStack.clear();
        discardState();
    }

    void IRBuilder::discardStateUntil(yoi::indexT stateIndex) {
        while (codeBlockInsertionStates.size() > stateIndex) {
            discardState();
        }
    }

    bool IRBuilder::hasTerminated() {
        if (!codeBlocks[currentCodeBlockIndex]->getIRArray().empty()) {
            switch (codeBlocks[currentCodeBlockIndex]->getIRArray().back().opcode) {
                case IR::Opcode::ret:
                case IR::Opcode::ret_none:
                case IR::Opcode::jump:
                    return true;
                default:
                    break;
            }
        }
        return false;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setMarco(const yoi::wstr &name, const yoi::wstr &value) {
        marcos[name] = value;
        return *this;
    }

    yoi::indexT IRVariableTable::scopeIndex(yoi::indexT varIndex) {
        return variableScopeMap[varIndex];
    }

    void IRBuilder::popLoopContext() {
        loopContext.pop_back();
    }

    void IRBuilder::breakOp() {
        yoi_assert(!loopContext.empty(), currentDebugInfo.line, currentDebugInfo.column, "break statement outside a `while` or `for` loop");
        jumpOp(loopContext.back().breakTarget);
    }

    void IRBuilder::continueOp() {
        yoi_assert(!loopContext.empty(), currentDebugInfo.line, currentDebugInfo.column, "continue statement outside a `while` or `for` loop");
        jumpOp(loopContext.back().continueTarget);
    }

    void IRBuilder::bindElementsOp(yoi::indexT extractElementCount, ExtractType extractType) {
        auto rhs = tempVarStack.back();
        tempVarStack.pop_back();
        for (yoi::indexT i = 0; i < extractElementCount; i++) {
            tempVarStack.push_back(managedPtr(rhs->getElementType()));
        }

        insert(IR{extractType == ExtractType::First ? IR::Opcode::bind_elements_post : IR::Opcode::bind_elements_pred,
                  {{IROperand::operandType::index, extractElementCount}},
                  currentDebugInfo});
    }

    void IRBuilder::bindFieldsOp(yoi::indexT extractFieldCount, ExtractType extractType) {
        auto rhs = tempVarStack.back();
        auto structDef = compilerCtx->getImportedModule(rhs->typeAffiliateModule)->structTable[rhs->typeIndex];
        yoi::indexT startPos = (extractType == ExtractType::First ? 0 : structDef->fieldTypes.size() - extractFieldCount);

        for (auto curPos = startPos; curPos < startPos + extractFieldCount; curPos++) {
            tempVarStack.push_back(structDef->fieldTypes[curPos]);
        }
        insert(IR{extractType == ExtractType::First ? IR::Opcode::bind_fields_post : IR::Opcode::bind_fields_pred,
                  {{IROperand::operandType::index, extractFieldCount}},
                  currentDebugInfo});
    }

    void IRBuilder::pushLoopContext(yoi::indexT breakTarget, yoi::indexT continueTarget) {
        loopContext.push_back({breakTarget, continueTarget});
    }

    IRValueType::IRValueType() : type(valueType::none), typeAffiliateModule(0), typeIndex(0), dimensions() {}

    void IRBuilder::invokeDanglingOp(yoi::indexT funcIndex,
                                     yoi::indexT funcArgsCount,
                                     const std::shared_ptr<IRValueType> &returnType,
                                     bool externalInvocation,
                                     yoi::indexT moduleIndex) {
        for (yoi::indexT i = 0; i < funcArgsCount; i++) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(returnType);
        insert(IR{IR::Opcode::invoke_dangling,
                  {{IROperand::operandType::index, moduleIndex == -1 ? currentModule->identifier : moduleIndex},
                   {IROperand::operandType::index, funcIndex},
                   {IROperand::operandType::index, funcArgsCount}},
                  currentDebugInfo});
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setAdditionalLinkingFiles(const yoi::vec<yoi::wstr> &additionalLinkingFiles) {
        this->additionalLinkingFiles = additionalLinkingFiles;
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setAdditionalLinkerOptions(const yoi::vec<yoi::wstr> &additionalLinkerOptions) {
        this->additionalLinkerOptions = additionalLinkerOptions;
        return *this;
    }

    IREnumerationType::IREnumerationType(const yoi::wstr &name, const yoi::indexTable<yoi::wstr, yoi::indexT> &valueToIndexMap)
        : name(name), valueToIndexMap(valueToIndexMap) {}

    IREnumerationType::UnderlyingType IREnumerationType::getUnderlyingType() {
        yoi::indexT maxIndex = 0;

        for (auto &entry : valueToIndexMap)
            maxIndex = std::max(maxIndex, entry.second);

        if (maxIndex <= 255)
            return UnderlyingType::I8;
        else if (maxIndex <= 65535)
            return UnderlyingType::I16;
        else
            return UnderlyingType::I64;
    }

    IREnumerationType::Builder &IREnumerationType::Builder::setName(const yoi::wstr &name) {
        this->name = name;
        return *this;
    }

    IREnumerationType::Builder &IREnumerationType::Builder::addValue(const yoi::wstr &valueName, yoi::indexT valueIndex) {
        this->valueToIndexMap.put_create(valueName, valueIndex);
        return *this;
    }

    void IRMetadata::eraseMetadata(const yoi::wstr &key) {
        if (metadata.count(key)) {
            metadata.erase(key);
        }
    }

    bool IRMetadata::hasMetadata(const yoi::wstr &key) const {
        return metadata.find(key) != metadata.end();
    }

    yoi::wstr IRMetadata::to_string() const {
        std::wstringstream ss;
        for (auto &entry : metadata) {
            if (entry.first == L"regressed_interface_impl") {
                auto &implIndex = getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                ss << L"!" << entry.first << "{" << implIndex.first << "," << implIndex.second << "}" << ' ';
            } else if (entry.first == L"delayed_interface_impl") {
                auto &implIndex = getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"delayed_interface_impl");
                ss << L"!" << entry.first << "{" << implIndex.first << "," << implIndex.second << "}" << ' ';
            } else if (entry.first == L"from_local") {
                auto localIndex = getMetadata<yoi::indexT>(L"from_local");
                ss << L"!" << entry.first << "{" << localIndex << "}" << ' ';
            }
        }
        if (!ss.str().empty())
            ss.unget();
        return ss.str();
    }

    std::shared_ptr<IREnumerationType> IREnumerationType::Builder::yield() {
        return std::make_shared<IREnumerationType>(name, valueToIndexMap);
    }

    IRValueType::IRValueType(valueType type, const yoi::vec<yoi::IRValueType> &bracedTypes) : type(type), bracedTypes(bracedTypes) {}

    IRBuildConfig::Builder &IRBuildConfig::Builder::setBuildCachePath(const yoi::wstr &buildCachePath) {
        if (!buildCachePath.empty()) {
            this->buildCachePath = buildCachePath;
        }
        return *this;
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setImmediatelyClearupCache(bool immediatelyClearupCache) {
        this->immediatelyClearupCache = immediatelyClearupCache;
        return *this;
    }

    yoi::indexT IRValueType::calculateDimensionSize() const {
        if (dimensions.empty()) {
            return 1;
        }
        yoi::indexT size = 1;
        for (auto &dimension : dimensions) {
            size *= dimension;
        }
        return size;
    }

    IRDataStructDefinition::IRDataStructDefinition(const yoi::wstr &name,
                                                   const yoi::vec<std::shared_ptr<IRValueType>> &fieldTypes,
                                                   const std::map<yoi::wstr, yoi::indexT> &fields,
                                                   yoi::indexT linkedModuleId)
        : name(name), fieldTypes(fieldTypes), fields(fields), linkedModuleId(linkedModuleId) {}

    IRDataStructDefinition::Builder &IRDataStructDefinition::Builder::setName(const yoi::wstr &name) {
        this->name = name;
        return *this;
    }

    IRDataStructDefinition::Builder &IRDataStructDefinition::Builder::addField(const yoi::wstr &fieldName,
                                                                               const std::shared_ptr<IRValueType> &fieldType) {
        this->fieldTypes.push_back(fieldType);
        this->fields[fieldName] = this->fieldTypes.size() - 1;
        return *this;
    }

    IRDataStructDefinition::Builder &IRDataStructDefinition::Builder::setLinkedModuleId(yoi::indexT linkedModuleId) {
        this->linkedModuleId = linkedModuleId;
        return *this;
    }

    std::shared_ptr<IRDataStructDefinition> IRDataStructDefinition::Builder::yield() {
        return std::make_shared<IRDataStructDefinition>(name, fieldTypes, fields, linkedModuleId);
    }

    yoi::wstr IRDataStructDefinition::to_string(yoi::indexT indent) {
        std::wstringstream ss;
        ss << yoi::wstr(indent, L' ') << L"datastruct " << name << L" {\n";
        for (auto &field : fieldTypes) {
            ss << yoi::wstr(indent + 4, L' ') << field->to_string() << L'\n';
        }
        ss << yoi::wstr(indent, L' ') << L'}' << L'\n';
        return ss.str();
    }

    void IRBuilder::initializeFieldsOp(yoi::indexT parameterCount) {
        for (yoi::indexT i = 0; i < parameterCount; i++) {
            popFromTempVarStack();
        }
        insert(IR{IR::Opcode::initialize_field, {{IROperand::operandType::index, parameterCount}}, currentDebugInfo});
    }

    void IRBuilder::loadFieldOp(yoi::vec<yoi::IROperand> &accessors,
                                const std::shared_ptr<IRValueType> &expectedType) {
        popFromTempVarStack();
        insert(IR{IR::Opcode::load_field, accessors, currentDebugInfo});
        pushTempVar(expectedType);
    }

    void IRBuilder::storeFieldOp(yoi::vec<yoi::IROperand> &accessors) {
        popFromTempVarStack();
        insert(IR{IR::Opcode::store_field, accessors, currentDebugInfo});
        popFromTempVarStack();
    }

    void IRBuilder::resumeOp() {
        popFromTempVarStack();
        insert(IR{IR::Opcode::resume, {}, currentDebugInfo});
    }

    void IRBuilder::yieldOp(bool yieldNone) {
        if (!yieldNone)
            popFromTempVarStack();
        popFromTempVarStack();
        insert(IR{IR::Opcode::yield, {}, currentDebugInfo});
    }
    
    void IRVariableTable::set(yoi::indexT index, const std::shared_ptr<IRValueType> &type) {
        variables[index] = type;
    }

    IRTemplateBuilder::Argument::Argument(const std::shared_ptr<IRValueType> &templateType,
                                          const yoi::vec<externModuleAccessExpression *> &satisfyConditions) : templateType(templateType), satisfyCondition(satisfyConditions) {
            
    }

    IRBuildConfig::Builder &IRBuildConfig::Builder::setTargetTriple(const yoi::wstr &targetTriple) {
        this->targetTriple = targetTriple;
        return *this;
    }
} // namespace yoi
