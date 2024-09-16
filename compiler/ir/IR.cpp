//
// Created by XIaokang00010 on 2024/9/6.
//

#include <ranges>
#include "IR.h"

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

    IR::IR(IR::Opcode opcode, const vec<IROperand> &operands) : opcode(opcode), operands(operands) {

    }

    yoi::wstr IR::to_string() {
        // TODO
        return {};
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
        currentFunction->tempVars = std::move(tempVars);
        currentFunction = nullptr;
        codeBlocks.clear();
        tempVars.clear();
    }

    yoi::IROperand IRBuilder::createTempVar(const std::shared_ptr<yoi::IRValueType> &type) {
        tempVars.push_back(type);
        return {IROperand::operandType::tempVar, IROperand::operandValue{yoi::indexT{tempVars.size() - 1}}};
    }

    IRCodeBlock &IRBuilder::getCurrentCodeBlock() {
        return *codeBlocks[currentCodeBlockIndex];
    }

    IRValueType IRBuilder::getTempVar(yoi::indexT index) {
        return *tempVars[index];
    }

    yoi::IROperand IRBuilder::createLocalVar(const yoi::wstr &varName, const std::shared_ptr<IRValueType> &type) {
        auto idx = currentFunction->getVariableTable().put(varName, type);
        return {IROperand::operandType::localVar, {idx}};
    }

    void IRBuilder::insert(const IR &ir) {
        getCurrentCodeBlock().insert(ir);

    }

    IRValueType IRBuilder::getLocalVar(yoi::indexT index) {

    }

    IRBuilder::IRBuilder(std::shared_ptr<IRModule> currentModule, std::shared_ptr<IRFunctionDefinition> currentFunction) : currentModule(currentModule), currentFunction(currentFunction), currentCodeBlockIndex(0) {}

    std::shared_ptr<IRValueType> IRBuilder::extractValueType(const IROperand &operand) {
        if (operand.type == IROperand::operandType::localVar) {
            return currentFunction->getVariableTable().get(operand.value.symbolIndex);
        } else if (operand.type == IROperand::operandType::globalVar) {
            return currentModule->globalVariables[operand.value.symbolIndex];
        } else if (operand.type == IROperand::operandType::tempVar) {
            return tempVars[operand.value.symbolIndex];
        } else {
            switch (operand.type) {
                case IROperand::operandType::integer:
                    return std::make_shared<IRValueType>(IRValueType::valueType::integer);
                case IROperand::operandType::decimal:
                    return std::make_shared<IRValueType>(IRValueType::valueType::decimal);
                case IROperand::operandType::boolean:
                    return std::make_shared<IRValueType>(IRValueType::valueType::boolean);
                case IROperand::operandType::character:
                    return std::make_shared<IRValueType>(IRValueType::valueType::character);
                case IROperand::operandType::stringLiteral:
                    return std::make_shared<IRValueType>(IRValueType::valueType::stringLiteral);
                default:
                    return nullptr;
            }
        }
    }

    yoi::IROperand IRBuilder::deref(const IROperand &operand) {
        auto type = extractValueType(operand);
        if (operand.type == IROperand::operandType::tempVar && type->type == IRValueType::valueType::lvalue) {
            if ( type->lvalueType->isBasicType()) {
                auto rhs = createTempVar(operand.lvalueType);
                insert({IR::Opcode::deref, {operand, rhs}});
                return rhs;
            } else {
                panic(0, 0, "Cannot dereference a non-basic type.");
            }
        } else {
            return operand;
        }
    }

    yoi::IROperand IRBuilder::basicCast(const IROperand &operand, const std::shared_ptr<IRValueType> &type) {
        auto res = createTempVar(type);
        insert({IR::Opcode::basic_cast, {operand, res}});
        return res;
    }

    yoi::IROperand IRBuilder::arithmeticOp(IR::Opcode op, const IROperand &left, const IROperand &right) {
        assert(left.type == right.type, 0, 0, "Type mismatch in multiplication operation.");
        auto tempVar = createTempVar(left.lvalueType);
        insert(IR(op, {left, right, tempVar}));
        return tempVar;
    }

    void IRBuilder::jumpOp(yoi::indexT target) {
        insert(IR(IR::Opcode::jump, {IROperand(IROperand::operandType::codeBlock, target)}));
    }

    void IRBuilder::jumpIfOp(IR::Opcode op, const IROperand &condition, yoi::indexT target) {
        insert(IR(op, {condition, IROperand(IROperand::operandType::codeBlock, target)}));
    }

    void IRBuilder::switchCodeBlock(yoi::indexT index) {
        currentCodeBlockIndex = index;
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

    yoi::wstr IRCodeBlock::to_string() {
        // TODO
        return {};
    }

    IRFunctionDefinition::IRFunctionDefinition(const yoi::wstr &name, const yoi::vec <std::shared_ptr<IRCodeBlock>> &codeBlock,
                                               const yoi::vec <std::shared_ptr<IRValueType>> &argumentTypes) : name(name), argumentTypes(argumentTypes), codeBlock(codeBlock) {}

    yoi::wstr IRFunctionDefinition::to_string() {
        // TODO
        return {};
    }

    IRVariableTable &IRFunctionDefinition::getVariableTable() {
        return variableTable;
    }

    IRValueType::IRValueType(IRValueType::valueType type, yoi::indexT objectPrototypeIndex) : type(type), typeIndex(objectPrototypeIndex) {

    }

    IRValueType::IRValueType(IRValueType::valueType type) : type(type), typeIndex(0) {

    }

    bool IRValueType::isBasicType() const {
        return type == valueType::integer || type == valueType::decimal || type == valueType::boolean || type == valueType::character;
    }

    IRValueType::IRValueType(IRValueType::valueType type, std::shared_ptr<IRValueType> lvalueType) : lvalueType(lvalueType) {

    }

    IRStructDefinition::IRStructDefinition(const yoi::wstr &name, const std::map<yoi::wstr, nameInfo>& nameInfoMap, const vec <std::shared_ptr<IRValueType>> &fieldTypes,
                                           const vec <std::shared_ptr<IRFunctionDefinition>> &methodDefinitions) : name(name), nameIndexMap(nameInfoMap), fieldTypes(fieldTypes), methodDefinitions(methodDefinitions) {

    }

    yoi::wstr IRStructDefinition::to_string() {
        // TODO
        return {};
    }

    const IRStructDefinition::nameInfo &IRStructDefinition::lookupName(const wstr &name) {
        return nameIndexMap.at(name);
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

    std::shared_ptr<IRValueType> IRVariableTable::get(yoi::indexT index) {
        return variables[index];
    }

    std::shared_ptr<IRValueType> IRVariableTable::operator[](const wstr &name) {
        return variables[lookup(name)];
    }

    yoi::indexT IRVariableTable::put(const wstr &name, const std::shared_ptr<IRValueType> &type) {
        variables.emplace_back(type);
        variableNameIndexMap.back()[name] = variables.size() - 1;
        return variables.size() - 1;
    }
} // yoi