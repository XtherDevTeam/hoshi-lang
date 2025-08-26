//
// Created by XIaokang00010 on 2024/9/6.
//

#include "IR.h"
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

    IROperand::IROperand(IROperand::operandType type, std::shared_ptr<IRValueType> lvalueType)
        : type(type), lvalueType(lvalueType) {}

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
        yoi::indexT implInterfaceIndex,
        const yoi::vec<std::shared_ptr<IRValueType>> &virtualMethods,
        const std::map<yoi::wstr, yoi::indexT> &virtualMethodIndexMap)
        : name(name), implStructIndex(implStructIndex), virtualMethods(virtualMethods),
          virtualMethodIndexMap(virtualMethodIndexMap), implInterfaceIndex(implInterfaceIndex) {}

    IRInterfaceInstanceDefinition::IRInterfaceInstanceDefinition(
        const yoi::wstr &name, const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> &methodMap)
        : name(name), methodMap(methodMap) {}

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
        : compilerCtx(compilerCtx), currentModule(currentModule), currentFunction(currentFunction),
          currentCodeBlockIndex(0), currentDebugInfo() {}

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
        switch (valType->type) {
            case IRValueType::valueType::integerObject:
                insert({IR::Opcode::basic_cast_int, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    compilerCtx->getIntObjectType();
                break;
            case IRValueType::valueType::decimalObject:
                insert({IR::Opcode::basic_cast_deci, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    compilerCtx->getDeciObjectType();
                break;
            case IRValueType::valueType::characterObject:
                insert({IR::Opcode::basic_cast_char, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    compilerCtx->getCharObjectType();
                break;
            case IRValueType::valueType::booleanObject:
                insert({IR::Opcode::basic_cast_bool, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    compilerCtx->getBoolObjectType();
                break;
            case IRValueType::valueType::pointerObject:
                insert({IR::Opcode::pointer_cast, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    managedPtr(IRValueType{IRValueType::valueType::pointerObject});
                break;
            case IRValueType::valueType::shortObject:
                insert({IR::Opcode::basic_cast_short, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    compilerCtx->getShortObjectType();
                break;
            case IRValueType::valueType::unsignedObject:
                insert({IR::Opcode::basic_cast_unsigned, {}, currentDebugInfo}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) =
                    compilerCtx->getUnsignedObjectType();
                break;
            default: {
                panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported type for basicCast");
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
        yoi_assert(left->type == right->type, 0, 0, "Type mismatch in arithmetic operation: " + wstring2string(left->to_string()) + " " + std::string{magic_enum::enum_name<>(op)} + " " + wstring2string(right->to_string()));
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
        insert(IR(IR::Opcode::jump, {IROperand(IROperand::operandType::codeBlock, target)}, currentDebugInfo));
    }

    void IRBuilder::jumpIfOp(IR::Opcode op, yoi::indexT target) {
        // fetch condition from tempVarStack
        auto condition = tempVarStack.back();
        tempVarStack.pop_back();
        yoi_assert(
            condition->type == IRValueType::valueType::booleanObject, 0, 0, "Type mismatch in jumpIf operation.");
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
        } else if (constV.type == IROperand::operandType::shortInt) {
            tempVarStack.emplace_back(compilerCtx->getShortObjectType());
        } else if (constV.type == IROperand::operandType::unsignedInt) {
            tempVarStack.emplace_back(compilerCtx->getUnsignedObjectType());
        } else {
            panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported constant type for pushOp");
        }
        insert({op, {constV}, currentDebugInfo});
    }

    void IRBuilder::loadOp(IR::Opcode op,
                           const yoi::IROperand &source,
                           const std::shared_ptr<IRValueType> &expectedType,
                           yoi::indexT moduleIndex) {
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
            case IR::Opcode::store_member: {
                // fetch rhs from tempVarStack
                tempVarStack.pop_back();
                insert({op, {operand}, currentDebugInfo});
                break;
            }
            case IR::Opcode::store_global: {
                // fetch rhs from tempVarStack
                tempVarStack.pop_back();
                insert({op, {{IROperand::operandType::index, moduleIndex}, operand}, currentDebugInfo});
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
                   {IROperand::operandType::index, funcArgsCount}}, currentDebugInfo));
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
                   {IROperand::operandType::index, methodArgsCount + 1 - isStatic}}, currentDebugInfo));
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
                   {IROperand::operandType::index, methodArgsCount + 1}}, currentDebugInfo));
    }

    void IRBuilder::retOp(bool returnWithNone) {
        // fetch return value from tempVarStack
        if (returnWithNone) {
            insert(IR(IR::Opcode::ret_none, {}, currentDebugInfo));
            return;
        }
        auto retValue = tempVarStack.back();
        tempVarStack.pop_back();
        insert(IR(IR::Opcode::ret, {}, currentDebugInfo));
    }

    void IRBuilder::newStructOp(yoi::indexT structIndex, bool isExternal, yoi::indexT moduleIndex) {
        insert(IR{IR::Opcode::new_struct,
                  {IROperand(IROperand::operandType::index, isExternal ? moduleIndex : currentModule->identifier),
                   IROperand(IROperand::operandType::index, structIndex)}, currentDebugInfo});
        tempVarStack.emplace_back(managedPtr(IRValueType{
            IRValueType::valueType::structObject, isExternal ? moduleIndex : currentModule->identifier, structIndex}));
    }

    void IRBuilder::newInterfaceOp(yoi::indexT interfaceIndex, bool isExternal, yoi::indexT moduleIndex) {
        insert(IR{IR::Opcode::new_interface,
                  {IROperand(IROperand::operandType::index, isExternal ? moduleIndex : currentModule->identifier),
                   IROperand(IROperand::operandType::index, interfaceIndex)}, currentDebugInfo});
        tempVarStack.emplace_back(managedPtr(IRValueType{IRValueType::valueType::interfaceObject,
                                                         isExternal ? moduleIndex : this->currentModule->identifier,
                                                         interfaceIndex}));
    }

    void IRBuilder::constructInterfaceImplOp(yoi::indexT interfaceImplIndex, bool isExternal, yoi::indexT moduleIndex) {
        auto rhs = tempVarStack.back();
        this->tempVarStack.pop_back(); // remove interfaceObject from tempVarStack
        this->tempVarStack.pop_back(); // remove structObject from tempVarStack
        tempVarStack.push_back(rhs);
        insert(IR{IR::Opcode::construct_interface_impl,
                  {IROperand(IROperand::operandType::index, isExternal ? moduleIndex : currentModule->identifier),
                   IROperand(IROperand::operandType::index, interfaceImplIndex)}, currentDebugInfo});
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
                r += (*it)->to_string() + L", ";
            }
            r += argumentTypes.back()->to_string();
        }
        r += L") : " + returnType->to_string() + L" {\n";
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

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::setName(const yoi::wstr &interfaceName) {
        this->name = interfaceName;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::setImplStructIndex(std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex) {
        this->implStructIndex = implStructIndex;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::setImplInterfaceIndex(yoi::indexT implInterfaceIndex) {
        this->implInterfaceIndex = implInterfaceIndex;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &
    IRInterfaceImplementationDefinition::Builder::addVirtualMethod(const yoi::wstr &methodName,
                                                                   const std::shared_ptr<IRValueType> &methodType) {
        this->virtualMethods.emplace_back(methodType);
        this->virtualMethodIndexMap[methodName] = this->virtualMethods.size() - 1;
        return *this;
    }

    std::shared_ptr<IRInterfaceImplementationDefinition> IRInterfaceImplementationDefinition::Builder::yield() {
        return std::make_shared<IRInterfaceImplementationDefinition>(
            IRInterfaceImplementationDefinition{std::move(name),
                                                implStructIndex,
                                                implInterfaceIndex,
                                                std::move(virtualMethods),
                                                std::move(virtualMethodIndexMap)});
    }

    IRInterfaceInstanceDefinition::Builder &
    IRInterfaceInstanceDefinition::Builder::setName(const yoi::wstr &interfaceName) {
        this->name = interfaceName;
        return *this;
    }

    IRInterfaceInstanceDefinition::Builder &
    IRInterfaceInstanceDefinition::Builder::addMethod(const yoi::wstr &methodName,
                                                      const std::shared_ptr<IRFunctionDefinition> &methodSignature) {
        this->methodMap.put_create(methodName, methodSignature);
        return *this;
    }

    std::shared_ptr<IRInterfaceInstanceDefinition> IRInterfaceInstanceDefinition::Builder::yield() {
        return std::make_shared<IRInterfaceInstanceDefinition>(std::move(name), std::move(methodMap));
    }

    IRFunctionDefinition::Builder &
    IRFunctionDefinition::Builder::addArgument(const yoi::wstr &argumentName,
                                               const std::shared_ptr<IRValueType> &argumentType) {
        this->argumentTypes.emplace_back(argumentName, argumentType);
        return *this;
    }

    IRFunctionDefinition::Builder &
    IRFunctionDefinition::Builder::setReturnType(const std::shared_ptr<IRValueType> &returnType) {
        this->returnType = returnType;
        return *this;
    }

    std::shared_ptr<IRFunctionDefinition> IRFunctionDefinition::Builder::yield() {
        return managedPtr(IRFunctionDefinition{name, argumentTypes, returnType, {}, attrs, debugInfo});
    }

    IRFunctionDefinition::IRFunctionDefinition(
        const yoi::wstr &name,
        const yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> &argumentTypes,
        const std::shared_ptr<IRValueType> &returnType, const yoi::vec<std::shared_ptr<IRCodeBlock>> &codeBlock, const yoi::vec<FunctionAttrs> &attrs, const IRDebugInfo &debugInfo) 
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

    IRValueType::IRValueType(IRValueType::valueType type,
                             yoi::indexT typeAffiliateModule,
                             yoi::indexT objectPrototypeIndex)
        : type(type), typeAffiliateModule(typeAffiliateModule), typeIndex(objectPrototypeIndex), dimensions() {}

    IRValueType::IRValueType(IRValueType::valueType type)
        : type(type), typeIndex(0), typeAffiliateModule(0), dimensions() {}

    bool IRValueType::isBasicType() const {
        return type == valueType::integerObject || type == valueType::decimalObject ||
               type == valueType::booleanObject || type == valueType::stringObject || 
               type == valueType::characterObject || type == valueType::foreignFloatType || type == valueType::foreignInt32Type ||
               type == valueType::shortObject || type == valueType::unsignedObject;
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
            default:
                res = L"unknown";
                break;
        }

        if (!dimensions.empty()) {
            if (dimensions.back() == -1) res += L"[]";
            else for (auto d : dimensions) res += L"[" + std::to_wstring(d) + L"]";
        }

        if (showAttributes)
            for (auto &i : attributes) res += L" @" + string2wstring(std::string{magic_enum::enum_name(i)});

        return res;
    }

    bool IRValueType::operator==(const yoi::IRValueType &rhs) const {
        return type == rhs.type && typeIndex == rhs.typeIndex && typeAffiliateModule == rhs.typeAffiliateModule && dimensions == rhs.dimensions;
    }

    IRStructDefinition::IRStructDefinition(const yoi::wstr &name,
                                           const std::map<yoi::wstr, nameInfo> &nameInfoMap,
                                           const vec<std::shared_ptr<IRValueType>> &fieldTypes)
        : name(name), nameIndexMap(nameInfoMap), fieldTypes(fieldTypes) {}

    yoi::wstr IRStructDefinition::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"struct " + name + L" {\n";
        for (auto &i : nameIndexMap) {
            if (i.second.type == nameInfo::nameType::field) {
                r += yoi::wstr(indent + 4, L' ') + i.first + L" " + fieldTypes[i.second.index]->to_string() + L"\n";
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

    IRStructDefinition::Builder &IRStructDefinition::Builder::addField(const yoi::wstr &fieldName,
                                                                       const std::shared_ptr<IRValueType> &fieldType) {
        this->fieldTypes.push_back(fieldType);
        nameIndexMap[fieldName] = nameInfo{nameInfo::nameType::field, this->fieldTypes.size() - 1};
        return *this;
    }

    IRStructDefinition::Builder &IRStructDefinition::Builder::addMethod(const yoi::wstr &methodName,
                                                                        yoi::indexT index) {
        nameIndexMap[methodName] = nameInfo{nameInfo::nameType::method, index};
        return *this;
    }

    std::shared_ptr<IRStructDefinition> IRStructDefinition::Builder::yield() {
        return std::make_shared<IRStructDefinition>(std::move(name), std::move(nameIndexMap), std::move(fieldTypes));
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
        panic(0, 0, wstring2string(L"Undefined variable: " + name));
        return 0; // make compiler happy
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
            r += yoi::wstr(indent + 4, L' ') + L"#" + std::to_wstring(i) + L" " + reversedVariableNameMap[i] +
                 L"(scope#" + std::to_wstring(variableScopeMap[i]) + L") : " + variables[i]->to_string(true) + L"\n";
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

    IRExternEntry::IRExternEntry(externType type,
                                 const yoi::wstr &name,
                                 yoi::indexT affiliateModule,
                                 yoi::indexT itemIndex)
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
                                        preserveIntermediateFiles,
                                        searchPaths,
                                        additionalLinkingFiles});
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
        r += yoi::wstr(indent, L' ')
            + L"impl "
            + name
            + L" for "
            + yoi::string2wstring(std::string{magic_enum::enum_name(std::get<0>(implStructIndex))})
            + L"#"
            + std::to_wstring(std::get<1>(implStructIndex))
            + L"#"
            + std::to_wstring(std::get<2>(implStructIndex)) + L" {\n";

        for (auto &i : virtualMethods) {
            r += yoi::wstr(indent + 4, L' ') + L"virtual " + i->to_string() + L"\n";
        }
        r += yoi::wstr(indent, L' ') + L"}\n";
        return r;
    }

    IRTemplateBuilder::Argument::Argument(const std::shared_ptr<IRValueType> &templateType,
                                          const std::pair<yoi::indexT, yoi::indexT> &interfaceType)
        : templateType(templateType), interfaceType(interfaceType) {}

    IRTemplateBuilder::Argument::Argument(const std::shared_ptr<IRValueType> &templateType)
        : templateType(templateType), interfaceType({0, 0}) {}

    IRFunctionTemplate::IRFunctionTemplate(
        const std::shared_ptr<IRFunctionDefinition> &templateDefinition,
        const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
        : templateDefinition(templateDefinition), templateArguments(templateArguments) {}

    IRFunctionTemplate::Builder &IRFunctionTemplate::Builder::setTemplateDefinition(
        const std::shared_ptr<IRFunctionDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }

    std::shared_ptr<IRFunctionTemplate> IRFunctionTemplate::Builder::yield() {
        return std::make_shared<IRFunctionTemplate>(templateDefinition, templateArguments);
    }

    IRStructTemplate::Builder &
    IRStructTemplate::Builder::setTemplateDefinition(const std::shared_ptr<IRStructDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }

    std::shared_ptr<IRStructTemplate> IRStructTemplate::Builder::yield() {
        return std::make_shared<IRStructTemplate>(
            IRStructTemplate{templateDefinition, templateMethods, templateArguments});
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

    IRTemplateBuilder & 
    IRTemplateBuilder::addTemplateArgument(const yoi::wstr &templateName,
                                           const std::shared_ptr<IRValueType> &templateType,
                                           const std::pair<yoi::indexT, yoi::indexT> &interfaceType) {
        templateArguments.put_create(templateName, {templateType, interfaceType});
        return *this;
    }

    IRStructTemplate::Builder &
    IRStructTemplate::Builder::setTemplateMethod(const yoi::wstr &methodName,
                                                 const std::shared_ptr<IRFunctionTemplate> &methodTemplate) {
        // templateMethods[methodName] = methodTemplate;
        templateMethods.put_create(methodName, methodTemplate);
        return *this;
    }

    yoi::vec<std::shared_ptr<IRValueType>> &IRVariableTable::getVariables() {
        return variables;
    }

    const std::map<yoi::indexT, yoi::wstr> &IRVariableTable::getReversedVariableNameMap() const {
        return reversedVariableNameMap;
    }

    IRStructTemplate::IRStructTemplate(
        const std::shared_ptr<IRStructDefinition> &templateDefinition,
        const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionTemplate>> &templateMethods,
        const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
        : templateDefinition(templateDefinition), templateMethods(templateMethods),
          templateArguments(templateArguments) {}

    IRFFITable::ImportLibrary::ImportLibrary(const yoi::wstr &libraryPath) : libraryPath(libraryPath) {}

    yoi::indexT IRFFITable::addImportedFunction(const yoi::wstr &libraryName,
                                                const yoi::wstr &functionName,
                                                const std::shared_ptr<IRFunctionDefinition> &functionDefinition) {
        if (!importedLibraries.contains(libraryName)) {
            importedLibraries.put_create(libraryName, {libraryName});
        }

        return importedLibraries[libraryName].importedFunctionTable.put(functionName, functionDefinition);
    }

    void
    IRFFITable::addExportedFunction(const yoi::wstr &exportName, yoi::indexT moduleIndex, yoi::indexT functionIndex, const yoi::vec<IRFunctionDefinition::FunctionAttrs> &attrs) {
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
        tempVarStack.push_back(returnType);

        // insert(IR{IR::Opcode::invoke_imported, {IROperand(IROperand::operandType::index, externIndex),
        // IROperand(IROperand::operandType::index, funcArgsCount)}});
        insert(IR{IR::Opcode::invoke_imported,
                  {IROperand(IROperand::operandType::index, libIndex),
                   IROperand(IROperand::operandType::index, funcIndex),
                   IROperand(IROperand::operandType::index, funcArgsCount)}, currentDebugInfo});
    }

    bool IRValueType::isArrayType() const {
        return !dimensions.empty() && dimensions.back() != static_cast<yoi::indexT>(-1);
    }

    IRValueType::IRValueType(valueType type,
                             yoi::indexT typeAffiliateModule,
                             yoi::indexT objectPrototypeIndex,
                             const yoi::vec<yoi::indexT> &dimensions)
        : type(type), dimensions(dimensions), typeAffiliateModule(typeAffiliateModule),
          typeIndex(objectPrototypeIndex) {}

    IRValueType::IRValueType(valueType type, const yoi::vec<yoi::indexT> &dimensions)
        : type(type), dimensions(dimensions), typeAffiliateModule(0), typeIndex(0) {}

    IRValueType IRValueType::getElementType() {
        return {this->type, this->typeAffiliateModule, this->typeIndex, yoi::vec<yoi::indexT>{}};
    }

    void IRBuilder::newArrayOp(const std::shared_ptr<IRValueType> &elementType,
                               const yoi::vec<yoi::indexT> &dimensions) {
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
                panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported array element type: " + yoi::wstring2string(elementType->to_string()));
                break;
        }
        auto size = 1;
        for (auto &dim : dimensions) {
            operands.emplace_back(IROperand::operandType::index, dim);
            size *= dim;
        }
        for (yoi::indexT i = 0; i < size; ++i) {
            tempVarStack.pop_back();
        }
        insert(IR{op, operands, currentDebugInfo});
        tempVarStack.push_back(managedPtr(elementType->getArrayType(dimensions)));
    }

    IRValueType IRValueType::getArrayType(const yoi::vec<yoi::indexT> &dimensions) {
        return {type, typeAffiliateModule, typeIndex, dimensions};
    }

    yoi::indexT IRBuilder::saveState() {
        codeBlockInsertionStates.push_back({codeBlocks[currentCodeBlockIndex]->getIRArray().size(), tempVarStack.size()});
        return codeBlockInsertionStates.size() - 1;
    }

    void IRBuilder::discardState() {
        codeBlockInsertionStates.pop_back();
    }

    void IRBuilder::restoreState() {
        codeBlocks[currentCodeBlockIndex]->getIRArray().resize(codeBlockInsertionStates.back().first);
        tempVarStack.resize(codeBlockInsertionStates.back().second);
        codeBlockInsertionStates.pop_back();
    }

    void IRBuilder::pushTempVar(const std::shared_ptr<IRValueType> &type) {
        tempVarStack.push_back(type);
    }

    void IRBuilder::popOp() {
        tempVarStack.pop_back();
        insert(IR{IR::Opcode::pop, {}, currentDebugInfo});
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
            switch (type->type) {
                case IRValueType::valueType::integerObject:
                    op = IR::Opcode::push_integer;
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(0));
                    break;
                case IRValueType::valueType::booleanObject:
                    op = IR::Opcode::push_integer;
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(2));
                    break;
                case IRValueType::valueType::decimalObject:
                    op = IR::Opcode::push_integer;
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(1));
                    break;
                case IRValueType::valueType::characterObject:
                    op = IR::Opcode::push_integer;
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(3));
                    break;
                case IRValueType::valueType::stringObject:
                    op = IR::Opcode::push_integer;
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(4));
                    break;
                case IRValueType::valueType::structObject:
                case IRValueType::valueType::interfaceObject:
                    op = IR::Opcode::typeid_object_non_stack;
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->type));
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->typeAffiliateModule));
                    operand.emplace_back(IROperand::operandType::index, type->typeIndex);
                    operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(0));
                    break;
                default:
                    /* TODO: add more typeid opcodes */
                    break;
            }
        }
        insert(IR(op, operand, currentDebugInfo));
        tempVarStack.push_back(managedPtr(IRValueType(IRValueType::valueType::integerObject)));
    }

    void IRBuilder::dynCastOp(const std::shared_ptr<IRValueType> &type) {
        IR::Opcode op;
        vec<IROperand> operand;
        if (type->isDynamicArrayType() || type->isArrayType()) {
            yoi::indexT size = 1;
            for (auto &dim : type->dimensions) {
                size *= dim;
            }

            op = IR::Opcode::dyn_cast_any;
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->type));
            operand.emplace_back(IROperand::operandType::index, static_cast<yoi::indexT>(type->typeAffiliateModule));
            operand.emplace_back(IROperand::operandType::index, type->typeIndex);
            operand.emplace_back(IROperand::operandType::index, size);
        } else {
            switch (type->type) {
                case IRValueType::valueType::integerObject:
                    op = IR::Opcode::dyn_cast_int;
                    break;
                case IRValueType::valueType::booleanObject:
                    op = IR::Opcode::dyn_cast_bool;
                    break;
                case IRValueType::valueType::decimalObject:
                    op = IR::Opcode::dyn_cast_deci;
                    break;
                case IRValueType::valueType::stringObject:
                    op = IR::Opcode::dyn_cast_str;
                    break;
                case IRValueType::valueType::characterObject:
                    op = IR::Opcode::dyn_cast_char;
                    break;
                case IRValueType::valueType::structObject:
                    op = IR::Opcode::dyn_cast_struct;
                    break;
                default:
                    /* TODO: add more dynamic cast opcodes */
                    break;
            }
            operand.emplace_back(IROperand::operandType::index, type->typeAffiliateModule);
            operand.emplace_back(IROperand::operandType::index, type->typeIndex);
        }
        insert(IR(op, operand, currentDebugInfo));
        tempVarStack.pop_back();
        tempVarStack.push_back(type);
    }

    IRFunctionDefinition::Builder &IRFunctionDefinition::Builder::addAttr(FunctionAttrs attr) {
        attrs.push_back(attr);
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
                panic(currentDebugInfo.line, currentDebugInfo.column, "Unsupported array element type: " + yoi::wstring2string(elementType->to_string()));
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

    void IRValueType::removeAttribute(ValueAttr attr) {
        attributes.erase(attr);
    }

    void IRValueType::addAttribute(ValueAttr attr) {
        attributes.insert(attr);
    }
    
    IRValueType::IRValueType(valueType type,
                             yoi::indexT typeAffiliateModule,
                             yoi::indexT objectPrototypeIndex,
                             const std::set<ValueAttr> &attributes)
        : type(type), typeAffiliateModule(typeAffiliateModule), typeIndex(objectPrototypeIndex), dimensions(),
          attributes(attributes) {}

    bool IRValueType::isBasicRawType() const {
        return type == valueType::integerRaw || type == valueType::decimalRaw || type == valueType::booleanRaw ||
               type == valueType::charRaw || type == IRValueType::valueType::shortRaw || type == IRValueType::valueType::unsignedRaw;
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
        tempStateCodeBlock = std::vector<IR>(codeBlocks[currentCodeBlockIndex]->getIRArray().begin() + current.first, codeBlocks[currentCodeBlockIndex]->getIRArray().end());
        tempStateTempVarStack = std::vector<std::shared_ptr<IRValueType>>(tempVarStack.begin() + current.second, tempVarStack.end());

        codeBlocks[currentCodeBlockIndex]->getIRArray().resize(current.first);
        tempVarStack.resize(current.second);
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
} // namespace yoi
