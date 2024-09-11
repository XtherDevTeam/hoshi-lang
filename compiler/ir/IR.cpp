//
// Created by XIaokang00010 on 2024/9/6.
//

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

    IR::IR(IR::Opcode opcode, const vec<IROperand> &operands) : opcode(opcode), operands(operands) {

    }

    yoi::wstr IR::to_string() {
        // TODO
        return {};
    }

    yoi::indexT IRBuilder::createCodeBlock() {
        codeBlocks.emplace_back(IRCodeBlock{});
        return (yoi::indexT)codeBlocks.size() - 1;
    }

    IRCodeBlock &IRBuilder::getCodeBlock(yoi::indexT index) {
        return codeBlocks[index];
    }

    std::tuple<std::vector<IRCodeBlock>, std::vector<IRValueType>> IRBuilder::yield() {
        return {std::move(codeBlocks), std::move(tempVars)};
    }

    yoi::IROperand IRBuilder::createTempVar(const yoi::IRValueType &type) {
        tempVars.push_back(type);
        return {IROperand::operandType::tempVar, {yoi::indexT{tempVars.size() - 1}}};
    }

    IRCodeBlock &IRBuilder::getCurrentCodeBlock() {
        return codeBlocks.back();
    }

    void IRBuilder::popCodeBlock() {
        codeBlocks.pop_back();
    }

    IRValueType IRBuilder::getTempVar(yoi::indexT index) {
        return tempVars[index];
    }

    void IRCodeBlock::insert(const IR &ir) {
        codeBlock.emplace_back(ir);
    }

    yoi::wstr IRCodeBlock::to_string() {
        // TODO
        return {};
    }

    IRFunctionDefinition::IRFunctionDefinition(const yoi::wstr &name, const yoi::vec <IRCodeBlock> &codeBlock,
                                               const yoi::vec <IRValueType> &argumentTypes) : name(name), argumentTypes(argumentTypes), codeBlock(codeBlock) {}

    yoi::wstr IRFunctionDefinition::to_string() {
        // TODO
        return {};
    }

    IRVariableTable &IRFunctionDefinition::getVariableTable() {
        return variableTable;
    }

    IRValueType::IRValueType(IRValueType::valueType type, yoi::indexT objectPrototypeIndex) : type(type), objectPrototypeIndex(objectPrototypeIndex) {

    }

    IRValueType::IRValueType(IRValueType::valueType type) : type(type), objectPrototypeIndex(0) {

    }

    IRStructDefinition::IRStructDefinition(const yoi::wstr &name, const vec <IRValueType> &fieldTypes,
                                           const vec <IRFunctionDefinition> &methodDefinitions) : name(name), fieldTypes(fieldTypes), methodDefinitions(methodDefinitions) {

    }

    yoi::wstr IRStructDefinition::to_string() {
        // TODO
        return {};
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

    IRValueType &IRVariableTable::get(yoi::indexT index) {
        return variables[index];
    }

    IRValueType &IRVariableTable::operator[](const wstr &name) {
        return variables[lookup(name)];
    }
} // yoi