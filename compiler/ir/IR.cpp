//
// Created by XIaokang00010 on 2024/9/6.
//

#include <memory>
#include <ranges>
#include "IR.h"
#include "share/def.hpp"

#include <compiler/frontend/ast.hpp>
#include <stdexcept>
#include <string>

namespace yoi {
    IROperand::operandValue::operandValue() : stringLiteralIndex(0) {}

    IROperand::operandValue::operandValue(int64_t integer) : integer(integer){

    }

    IROperand::operandValue::operandValue(yoi::indexT indexT) : stringLiteralIndex(indexT){

    }

    IROperand::operandValue::operandValue(bool boolean) : boolean(boolean) {

    }

    IROperand::operandValue::operandValue(double decimal) : decimal(decimal) {

    }

    IROperand::operandValue::operandValue(yoi::wchar character) : character(character) {

    }

    IROperand::IROperand() : type(operandType::unknown), value((yoi::indexT)0) {

    }

    IROperand::IROperand(IROperand::operandType type, IROperand::operandValue value) : type(type), value(value) {

    }

    IROperand::IROperand(IROperand::operandType type, std::shared_ptr<IRValueType> lvalueType) : type(type), lvalueType(lvalueType) {

    }

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
            default:
                return L"unknown";
        }
    }

    IR::IR(IR::Opcode opcode, const vec<IROperand> &operands) : opcode(opcode), operands(operands) {

    }

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

    IRInterfaceImplementationDefinition::IRInterfaceImplementationDefinition(const yoi::wstr &name, yoi::indexT implStructIndex, yoi::indexT implInterfaceIndex, const yoi::vec<std::shared_ptr<IRValueType>> &virtualMethods, const std::map<yoi::wstr, yoi::indexT> &virtualMethodIndexMap) : name(name), implStructIndex(implStructIndex), virtualMethods(virtualMethods), virtualMethodIndexMap(virtualMethodIndexMap), implInterfaceIndex(implInterfaceIndex) {
    }

    IRInterfaceInstanceDefinition::IRInterfaceInstanceDefinition(const yoi::wstr &name, const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> &methodMap) : name(name), methodMap(methodMap) {
    }

    yoi::wstr IRModule::to_string(yoi::indexT indent)
    {
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

    IRBuilder::IRBuilder(std::shared_ptr<compilerContext> compilerCtx, std::shared_ptr<IRModule> currentModule,
                         std::shared_ptr<IRFunctionDefinition> currentFunction) : compilerCtx(compilerCtx), currentModule(currentModule), currentFunction(currentFunction), currentCodeBlockIndex(0) {
    }

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
            if (insPoint != getCurrentCodeBlock().getIRArray().end()) {
                insPoint++;
            }
            getCurrentCodeBlock().getIRArray().insert(insPoint, ir);
        }
    }



    IRValueType IRBuilder::getLocalVar(yoi::indexT index) {

    }

    const std::shared_ptr<IRValueType> & IRBuilder::getLhsFromTempVarStack() {
        yoi_assert(tempVarStack.size() > 1, 0, 0, "tempVarStack is empty.");
        return tempVarStack[tempVarStack.size() - 2];
    }

    const std::shared_ptr<IRValueType> & IRBuilder::getRhsFromTempVarStack() {
        yoi_assert(tempVarStack.size() > 0, 0, 0, "tempVarStack is empty.");
        return tempVarStack[tempVarStack.size() - 1];
    }

    void IRBuilder::basicCast(const std::shared_ptr<IRValueType> &valType, yoi::indexT insertionPoint, bool lhs) {
        switch(valType->type) {
            case IRValueType::valueType::integerObject:
                insert({IR::Opcode::basic_cast_int, {}}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) = compilerCtx->getIntObjectType();
                break;
            case IRValueType::valueType::decimalObject:
                insert({IR::Opcode::basic_cast_deci, {}}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) = compilerCtx->getDeciObjectType();
                break;
            case IRValueType::valueType::booleanObject:
                insert({IR::Opcode::basic_cast_bool, {}}, insertionPoint);
                tempVarStack.at(lhs ? tempVarStack.size() - 2 : tempVarStack.size() - 1) = compilerCtx->getBoolObjectType();
                break;
            default: {
                panic(0, 0, "Unsupported type for basicCast");
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
        insert({op, {}});
    }

    void IRBuilder::arithmeticOp(IR::Opcode op) {
        // fetch lhs and rhs from tempVarStack
        auto right = tempVarStack.back();
        tempVarStack.pop_back();
        auto left = tempVarStack.back();
        tempVarStack.pop_back();
        yoi_assert(left->type == right->type, 0, 0, "Type mismatch in multiplication operation.");
        // push result to tempVarStack
        switch (op) {
            case IR::Opcode::add:
            case IR::Opcode::sub:
            case IR::Opcode::mul:
            case IR::Opcode::div:
            case IR::Opcode::mod:
            case IR::Opcode::left_shift:
            case IR::Opcode::right_shift: {
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
                panic(0, 0, "Unsupported type for arithmetic operation.");
                break;
            }
        }
        insert({op, {}});
    }

    void IRBuilder::jumpOp(yoi::indexT target) {
        insert(IR(IR::Opcode::jump, {IROperand(IROperand::operandType::codeBlock, target)}));
    }

    void IRBuilder::jumpIfOp(IR::Opcode op, yoi::indexT target) {
        // fetch condition from tempVarStack
        auto condition = tempVarStack.back();
        tempVarStack.pop_back();
        yoi_assert(condition->type == IRValueType::valueType::booleanObject, 0, 0, "Type mismatch in jumpIf operation.");
        // insert jumpIf operation
        insert(IR(op, {IROperand(IROperand::operandType::codeBlock, target)}));
    }

    void IRBuilder::pushOp(IR::Opcode op, const yoi::IROperand &constV) {
        if (constV.type == IROperand::operandType::integer) {
            // tempVarStack.push_back()
            tempVarStack.emplace_back(compilerCtx->getIntObjectType());
        } else if (constV.type == IROperand::operandType::boolean) {
            tempVarStack.emplace_back(compilerCtx->getBoolObjectType());
        } else if (constV.type == IROperand::operandType::decimal) {
            tempVarStack.emplace_back(compilerCtx->getDeciObjectType());
        } else if (constV.type == IROperand::operandType::stringLiteral) {
            tempVarStack.emplace_back(compilerCtx->getStrObjectType());
        } else {
            panic(0, 0, "Unsupported constant type for pushOp");
        }
        insert({op, {constV}});
    }

    void IRBuilder::loadOp(IR::Opcode op, const yoi::IROperand &source, const std::shared_ptr<IRValueType>& expectedType) {
        switch (source.type) {
            case IROperand::operandType::localVar:
            case IROperand::operandType::globalVar:
            case IROperand::operandType::externVar: {
                tempVarStack.emplace_back(expectedType);
                break;
            }
            default: {
                panic(0, 0, "Unsupported operand type for loadOp");
                break;
            }
        }
        insert({op, {source}});
    }

    void IRBuilder::loadMemberOp(const yoi::IROperand &memberIndex,
                                 const std::shared_ptr<IRValueType> &memberType) {
        tempVarStack.pop_back();
        tempVarStack.emplace_back(memberType);
        insert({IR::Opcode::load_member, {memberIndex}});
    }

    void IRBuilder::storeOp(IR::Opcode op, const yoi::IROperand &operand) {
        switch (operand.type) {
            case IROperand::operandType::localVar:
            case IROperand::operandType::globalVar:
            case IROperand::operandType::externVar: {
                // fetch rhs from tempVarStack
                tempVarStack.pop_back();
                break;
            }
            default: {
                panic(0, 0, "Unsupported operand type for storeOp");
                break;
            }
        }
        insert({op, {operand}});
    }

    void IRBuilder::storeMemberOp(const yoi::IROperand &memberIndex) {
        // pop rhs and lhs from tempVarStack
        tempVarStack.pop_back();
        tempVarStack.pop_back();
        insert({IR::Opcode::store_member, {memberIndex}});
    }

    void IRBuilder::invokeOp(yoi::indexT funcIndex, yoi::indexT funcArgsCount,
                             const std::shared_ptr<IRValueType> &returnType, bool externalInvocation) {
        for (yoi::indexT i = 0; i < funcArgsCount; i++) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(returnType);
        insert(IR(externalInvocation ? IR::Opcode::invoke_extern : IR::Opcode::invoke, {
                      {IROperand::operandType::index, funcIndex}, {IROperand::operandType::index, funcArgsCount}
                  }));
    }

    void IRBuilder::invokeMethodOp(yoi::indexT funcIndex, yoi::indexT methodArgsCount,
        const std::shared_ptr<IRValueType> &returnType, bool externalInvocation) {
        // this pointer is popped from tempVarStack
        for (yoi::indexT i = 0; i < methodArgsCount + 1; i++) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(returnType);
        insert(IR(externalInvocation ? IR::Opcode::invoke_extern : IR::Opcode::invoke, {
                      {IROperand::operandType::index, funcIndex}, {IROperand::operandType::index, methodArgsCount + 1}
                  }));
    }

    void IRBuilder::invokeVirtualOp(yoi::indexT funcIndex, yoi::indexT methodArgsCount, const std::shared_ptr<IRValueType> &returnType, bool externalInvocation) {
        for (yoi::indexT i = 0; i < methodArgsCount + 1; i++) {
            tempVarStack.pop_back();
        }
        tempVarStack.push_back(returnType);
        insert(IR(externalInvocation ? IR::Opcode::invoke_virtual_extern : IR::Opcode::invoke_virtual, {
                      {IROperand::operandType::index, funcIndex}, {IROperand::operandType::index, methodArgsCount + 1}
                  }));
    }

    void IRBuilder::retOp(bool returnWithNone) {
        // fetch return value from tempVarStack
        if (returnWithNone) {
            insert(IR(IR::Opcode::ret_none, {}));
            return;
        }
        auto retValue = tempVarStack.back();
        tempVarStack.pop_back();
        insert(IR(IR::Opcode::ret, {}));
    }

    void IRBuilder::newStructOp(yoi::indexT structIndex, bool isExternal) {
        if (isExternal) {
            auto &entry = this->currentModule->externTable[structIndex];
            insert(IR{IR::Opcode::new_struct_extern, {IROperand(IROperand::operandType::index, structIndex)}});
            tempVarStack.emplace_back(managedPtr(IRValueType{IRValueType::valueType::structObject, entry->affiliateModule, entry->itemIndex}));
        } else {
            insert(IR{IR::Opcode::new_struct, {IROperand(IROperand::operandType::index, structIndex)}});
            tempVarStack.emplace_back(managedPtr(IRValueType{IRValueType::valueType::structObject, this->currentModule->identifier, structIndex}));
        }
    }

    void IRBuilder::newInterfaceOp(yoi::indexT interfaceIndex, bool isExternal)  {
        if (isExternal) {
            auto &entry = this->currentModule->externTable[interfaceIndex];
            insert(IR{IR::Opcode::new_interface_extern, {IROperand(IROperand::operandType::index, interfaceIndex)}});
            tempVarStack.emplace_back(managedPtr(IRValueType{IRValueType::valueType::interfaceObject, entry->affiliateModule, entry->itemIndex}));
        } else {
            insert(IR{IR::Opcode::new_interface, {IROperand(IROperand::operandType::index, interfaceIndex)}});
            tempVarStack.emplace_back(managedPtr(IRValueType{IRValueType::valueType::interfaceObject, this->currentModule->identifier, interfaceIndex}));
        }
    }

    void IRBuilder::constructInterfaceImplOp(yoi::indexT interfaceImplIndex, bool isExternal) {
        this->tempVarStack.pop_back(); // remove structObject from tempVarStack
        insert(IR{isExternal ? IR::Opcode::construct_interface_impl_extern : IR::Opcode::construct_interface_impl, {IROperand(IROperand::operandType::index, interfaceImplIndex)}});
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

    yoi::vec<IR> & IRCodeBlock::getIRArray() {
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

    IRFunctionDefinition::Builder & IRFunctionDefinition::Builder::setName(const yoi::wstr &name) {
        this->name = name;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &IRInterfaceImplementationDefinition::Builder::setName(const yoi::wstr &interfaceName) {
        this->name = interfaceName;
        return *this;
    }
    
    IRInterfaceImplementationDefinition::Builder &IRInterfaceImplementationDefinition::Builder::setImplStructIndex(yoi::indexT implStructIndex) {
        this->implStructIndex = implStructIndex;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &IRInterfaceImplementationDefinition::Builder::setImplInterfaceIndex(yoi::indexT implInterfaceIndex) {
        this->implInterfaceIndex = implInterfaceIndex;
        return *this;
    }

    IRInterfaceImplementationDefinition::Builder &IRInterfaceImplementationDefinition::Builder::addVirtualMethod(const yoi::wstr &methodName, const std::shared_ptr<IRValueType> &methodType) {
        this->virtualMethods.emplace_back(methodType);
        this->virtualMethodIndexMap[methodName] = this->virtualMethods.size() - 1;
        return *this;
    }

    std::shared_ptr<IRInterfaceImplementationDefinition> IRInterfaceImplementationDefinition::Builder::yield() {
        return std::make_shared<IRInterfaceImplementationDefinition>(IRInterfaceImplementationDefinition{std::move(name), implStructIndex, implInterfaceIndex, std::move(virtualMethods), std::move(virtualMethodIndexMap)});
    }

    IRInterfaceInstanceDefinition::Builder &IRInterfaceInstanceDefinition::Builder::setName(const yoi::wstr &interfaceName) {
        this->name = interfaceName;
        return *this;
    }

    IRInterfaceInstanceDefinition::Builder &IRInterfaceInstanceDefinition::Builder::addMethod(const yoi::wstr &methodName, const std::shared_ptr<IRFunctionDefinition> &methodSignature) {
        this->methodMap.put_create(methodName, methodSignature);
        return *this;
    }

    std::shared_ptr<IRInterfaceInstanceDefinition> IRInterfaceInstanceDefinition::Builder::yield() {
        return std::make_shared<IRInterfaceInstanceDefinition>(std::move(name), std::move(methodMap));
    }

    IRFunctionDefinition::Builder & IRFunctionDefinition::Builder::addArgument(const yoi::wstr &argumentName,
        const std::shared_ptr<IRValueType> &argumentType) {
        this->argumentTypes.emplace_back(argumentName, argumentType);
        return *this;
    }

    IRFunctionDefinition::Builder & IRFunctionDefinition::Builder::setReturnType(
        const std::shared_ptr<IRValueType> &returnType) {
        this->returnType = returnType;
        return *this;
    }

    std::shared_ptr<IRFunctionDefinition> IRFunctionDefinition::Builder::yield() {
        return std::make_shared<IRFunctionDefinition>(std::move(name), std::move(argumentTypes), std::move(returnType));
    }

    IRFunctionDefinition::IRFunctionDefinition(const yoi::wstr &name,
                                               const yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> &argumentTypes, const std::shared_ptr<IRValueType> &returnType):
        name(name), returnType(returnType), variableTable(), codeBlock() {
        variableTable.createScope();
        for (auto &i : argumentTypes) {
            variableTable.put(i.first, i.second);
            this->argumentTypes.push_back(i.second);
        }
    }

    IRVariableTable &IRFunctionDefinition::getVariableTable() {
        return variableTable;
    }

    IRValueType::IRValueType(IRValueType::valueType type, yoi::indexT typeAffiliateModule, yoi::indexT objectPrototypeIndex) : type(type), typeAffiliateModule(typeAffiliateModule), typeIndex(objectPrototypeIndex) {

    }

    IRValueType::IRValueType(IRValueType::valueType type) : type(type), typeIndex(0), typeAffiliateModule(0) {

    }

    bool IRValueType::isBasicType() const {
        return type == valueType::integerObject || type == valueType::decimalObject || type == valueType::booleanObject;
    }

    bool IRValueType::is1ByteType() const {
        return type == valueType::booleanRaw || type == valueType::charRaw;
    }

    yoi::wstr IRValueType::to_string() const {
        switch (type) {
            case valueType::integerRaw:
                return L"int_literal";
            case valueType::decimalRaw:
                return L"deci_literal";
            case valueType::booleanRaw:
                return L"bool_literal";
            case valueType::characterObject:
                return L"char";
            case valueType::stringLiteral:
                return L"string";
            case valueType::structObject:
                return L"struct#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
            case valueType::null:
                return L"null";
            case valueType::integerObject:
                return L"int";
            case valueType::booleanObject:
                return L"bool";
            case valueType::decimalObject:
                return L"decimal";
            case valueType::stringObject:
                return L"string";
            case valueType::none:
                return L"none";
            case valueType::interfaceObject:
                return L"interface#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
            case valueType::pointerObject:
                return L"pointer";
            case valueType::virtualMethod:
                return L"virtual_method#" + std::to_wstring(typeAffiliateModule) + L"#" + std::to_wstring(typeIndex);
            case valueType::incompleteTemplateType:
                return L"incomplete_template_type#" + std::to_wstring(typeIndex);
            default:
                return L"unknown";
        }
    }

    bool IRValueType::operator==(const yoi::IRValueType &rhs) const {
        return type == rhs.type && typeIndex == rhs.typeIndex && typeAffiliateModule == rhs.typeAffiliateModule;
    }

    IRStructDefinition::IRStructDefinition(const yoi::wstr &name, const std::map<yoi::wstr, nameInfo>& nameInfoMap, const vec <std::shared_ptr<IRValueType>> &fieldTypes) : name(name), nameIndexMap(nameInfoMap), fieldTypes(fieldTypes) {

    }

    yoi::wstr IRStructDefinition::to_string(yoi::indexT indent) {
        yoi::wstr r;
        r += yoi::wstr(indent, L' ') + L"struct " + name + L" {\n";
        for (auto &i : nameIndexMap) {
            if (i.second.type == nameInfo::nameType::field) {
                r += yoi::wstr(indent + 4, L' ')  + i.first + L" " + fieldTypes[i.second.index]->to_string() + L"\n";
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

    IRStructDefinition::Builder & IRStructDefinition::Builder::setName(const yoi::wstr &name) {
        this->name = name;
        return *this;
    }

    IRStructDefinition::Builder & IRStructDefinition::Builder::addField(const yoi::wstr &fieldName,
        const std::shared_ptr<IRValueType> &fieldType) {
        this->fieldTypes.push_back(fieldType);
        nameIndexMap[fieldName] = nameInfo{nameInfo::nameType::field, this->fieldTypes.size() - 1};
        return *this;
    }

    IRStructDefinition::Builder & IRStructDefinition::Builder::addMethod(const yoi::wstr &methodName, yoi::indexT index) {
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
            r += yoi::wstr(indent + 4, L' ') + L"#" + std::to_wstring(i) + L" " + reversedVariableNameMap[i] + L"(scope#" + std::to_wstring(variableScopeMap[i]) + L") : " + variables[i]->to_string() + L"\n";
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

    IRExternEntry::IRExternEntry(externType type, const yoi::wstr &name, yoi::indexT affiliateModule,
        yoi::indexT itemIndex): type(type), name(name), affiliateModule(affiliateModule), itemIndex(itemIndex) {

    }

    IRExternEntry::externType IRExternEntry::getExternType() const {
        return type;
    }
    IRBuildConfig::Builder &
    IRBuildConfig::Builder::setBuildType(BuildType buildType) {
      this->buildType = buildType;
      return *this;
    }
    IRBuildConfig::Builder &
    IRBuildConfig::Builder::setBuildPlatform(const yoi::wstr &buildPlatform) {
      this->buildPlatform = buildPlatform;
      return *this;
    }
    IRBuildConfig::Builder &
    IRBuildConfig::Builder::setBuildArch(const yoi::wstr &buildArch) {
      this->buildArch = buildArch;
      return *this;
    }
    std::shared_ptr<IRBuildConfig> IRBuildConfig::Builder::yield() {
      return managedPtr(IRBuildConfig{buildType, buildMode, useObjectLinker, buildPlatform, buildArch, preserveIntermediateFiles});
    }
    IRBuildConfig::Builder &IRBuildConfig::Builder::setBuildMode(BuildMode buildMode) {
        this->buildMode = buildMode;
        return *this;
    }
    IRBuildConfig::Builder &
    IRBuildConfig::Builder::setUseObjectLinker(UseObjectLinker useObjectLinker) {
        this->useObjectLinker = useObjectLinker;
        return *this;
    }
    IRBuildConfig::Builder &
    IRBuildConfig::Builder::setPreserveIntermediateFiles(bool preserveIntermediateFiles) {
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
        r += yoi::wstr(indent, L' ') + L"impl " + name + L" for struct#" + std::to_wstring(implStructIndex) + L" {\n";
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
    IRStructTemplate::Builder &IRStructTemplate::Builder::setTemplateDefinition(
        const std::shared_ptr<IRStructDefinition> &templateDefinition) {
        this->templateDefinition = templateDefinition;
        return *this;
    }
    std::shared_ptr<IRStructTemplate> IRStructTemplate::Builder::yield() {
        return std::make_shared<IRStructTemplate>(IRStructTemplate{templateDefinition, templateMethods, templateArguments});
    }
    IRTemplateBuilder &IRTemplateBuilder::addTemplateArgument(
        const yoi::wstr &templateName,
        const std::shared_ptr<IRValueType> &templateType,
        const std::pair<yoi::indexT, yoi::indexT> &interfaceType) {
        templateArguments.put_create(templateName, {templateType, interfaceType});
        return *this;
    }
    IRStructTemplate::Builder &IRStructTemplate::Builder::setTemplateMethod(
        const yoi::wstr &methodName, const std::shared_ptr<IRFunctionTemplate> &methodTemplate) {
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
} // namespace yoi
