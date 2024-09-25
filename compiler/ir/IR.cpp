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
            getCurrentCodeBlock().getIRArray().insert(getCurrentCodeBlock().getIRArray().begin() + insertionPoint, ir);
        }
    }



    IRValueType IRBuilder::getLocalVar(yoi::indexT index) {

    }

    const std::shared_ptr<IRValueType> & IRBuilder::getLhsFromTempVarStack() {
        assert(tempVarStack.size() > 1, 0, 0, "tempVarStack is empty.");
        auto it = tempVarStack.rbegin();
        return *(--it);
    }

    const std::shared_ptr<IRValueType> & IRBuilder::getRhsFromTempVarStack() {
        assert(tempVarStack.size() > 0, 0, 0, "tempVarStack is empty.");
        auto it = tempVarStack.rbegin();
        return *it;
    }

    void IRBuilder::basicCast(const std::shared_ptr<IRValueType> &valType, yoi::indexT insertionPoint) {
        switch(valType->type) {
            case IRValueType::valueType::integerObject:
                insert({IR::Opcode::basic_cast_int, {}}, insertionPoint);
                tempVarStack.pop_back();
                tempVarStack.emplace_back(managedPtr(compilerCtx->getIntObjectType()));
                break;
            case IRValueType::valueType::decimalObject:
                insert({IR::Opcode::basic_cast_deci, {}}, insertionPoint);
                tempVarStack.pop_back();
                tempVarStack.emplace_back(managedPtr(compilerCtx->getIntObjectType()));
                break;
            case IRValueType::valueType::booleanObject:
                insert({IR::Opcode::basic_cast_bool, {}}, insertionPoint);
                tempVarStack.pop_back();
                tempVarStack.emplace_back(managedPtr(compilerCtx->getIntObjectType()));
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
        assert(left->type == right->type, 0, 0, "Type mismatch in multiplication operation.");
        // push result to tempVarStack
        switch (op) {
            case IR::Opcode::add:
            case IR::Opcode::sub:
            case IR::Opcode::mul:
            case IR::Opcode::div:
            case IR::Opcode::mod: {
                tempVarStack.emplace_back(left);
                break;
            }
            case IR::Opcode::less_than:
            case IR::Opcode::greater_than:
            case IR::Opcode::less_equal:
            case IR::Opcode::greater_equal:
            case IR::Opcode::equal:
            case IR::Opcode::not_equal: {
                tempVarStack.emplace_back(managedPtr(compilerCtx->getBoolObjectType()));
                break;
            }
            default: {
                panic(0, 0, "Unsupported type for arithmetic operation.");
                break;
            }
        }
    }

    void IRBuilder::jumpOp(yoi::indexT target) {
        insert(IR(IR::Opcode::jump, {IROperand(IROperand::operandType::codeBlock, target)}));
    }

    void IRBuilder::jumpIfOp(IR::Opcode op, yoi::indexT target) {
        // fetch condition from tempVarStack
        auto condition = tempVarStack.back();
        tempVarStack.pop_back();
        assert(condition->type == IRValueType::valueType::booleanObject, 0, 0, "Type mismatch in jumpIf operation.");
        // insert jumpIf operation
        insert(IR(op, {IROperand(IROperand::operandType::codeBlock, target)}));
    }

    void IRBuilder::pushOp(IR::Opcode op, const yoi::IROperand &constV) {
        if (constV.type == IROperand::operandType::integer) {
            // tempVarStack.push_back()
            tempVarStack.emplace_back(managedPtr(compilerCtx->getIntObjectType()));
        } else if (constV.type == IROperand::operandType::boolean) {
            tempVarStack.emplace_back(managedPtr(compilerCtx->getBoolObjectType()));
        } else if (constV.type == IROperand::operandType::decimal) {
            tempVarStack.emplace_back(managedPtr(compilerCtx->getDeciObjectType()));
        } else if (constV.type == IROperand::operandType::stringLiteral) {
            tempVarStack.emplace_back(managedPtr(compilerCtx->getStrObjectType()));
        } else {
            panic(0, 0, "Unsupported constant type for pushOp");
        }
        insert({op, {constV}});
    }

    void IRBuilder::loadOp(IR::Opcode op, const yoi::IROperand &operand) {
        switch (operand.type) {
            case IROperand::operandType::localVar:
            case IROperand::operandType::globalVar:
            case IROperand::operandType::externVar: {
                tempVarStack.emplace_back(operand.lvalueType);
                break;
            }
            default: {
                panic(0, 0, "Unsupported operand type for loadOp");
                break;
            }
        }
        insert({op, {operand}});
    }

    void IRBuilder::loadMemberOp(const yoi::IROperand &memberIndex,
                                 const std::shared_ptr<IRValueType> &memberType) {
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

    void IRBuilder::retOp() {
        // fetch return value from tempVarStack
        auto retValue = tempVarStack.back();
        tempVarStack.pop_back();
        insert(IR(IR::Opcode::ret, {}));
    }

    yoi::indexT IRBuilder::getCurrentInsertionPoint() {
        return (yoi::indexT)getCurrentCodeBlock().getIRArray().size();
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

    yoi::wstr IRCodeBlock::to_string() {
        // TODO
        return {};
    }

    yoi::vec<IR> & IRCodeBlock::getIRArray() {
        return codeBlock;
    }


    yoi::wstr IRFunctionDefinition::to_string() {
        // TODO
        return {};
    }

    IRFunctionDefinition::IRFunctionDefinition(const yoi::wstr &name,
        const yoi::vec<std::shared_ptr<IRValueType>> &argumentTypes, const std::shared_ptr<IRValueType> &returnType):
        name(name), argumentTypes(argumentTypes), returnType(returnType), variableTable(), codeBlock() {

    }

    IRVariableTable &IRFunctionDefinition::getVariableTable() {
        return variableTable;
    }

    IRValueType::IRValueType(IRValueType::valueType type, yoi::indexT objectPrototypeIndex) : type(type), typeIndex(objectPrototypeIndex) {

    }

    IRValueType::IRValueType(IRValueType::valueType type) : type(type), typeIndex(0) {

    }

    bool IRValueType::isBasicType() const {
        return type == valueType::integerObject || type == valueType::decimalObject || type == valueType::booleanObject;
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

    IRExternEntry::IRExternEntry(externType type, const yoi::wstr &name, yoi::indexT affiliateModule,
        yoi::indexT itemIndex): type(type), name(name), affiliateModule(affiliateModule), itemIndex(itemIndex) {

    }

    IRExternEntry::externType IRExternEntry::getExternType() const {
        return type;
    }
} // yoi