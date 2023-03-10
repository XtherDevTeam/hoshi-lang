//
// Created by XIaokang00010 on 2023/3/3.
//

#include "ir.hpp"

namespace hoshi {

    int64_t irConstantPool::putDeci(vdeci d) {
        return deciPool.put(d);
    }

    int64_t irStructType::setFieldType(const wstr &name, const irValueType &value) {
        return fields.put(name, value);
    }

    irValueType &irStructType::getFieldType(int64_t i) {
        return fields[i];
    }

    int64_t irStructType::getFieldIndex(const wstr &name) {
        return fields.getIndex(name);
    }

    int64_t irStructTable::putStructType(const wstr &name, const irStructType &value) {
        return structTypes.put(name, value);
    }

    irStructType &irStructTable::getStructType(int64_t i) {
        return structTypes[i];
    }

    void irFuncType::setResultType(const irValueType &value) {
        resultType = std::make_shared<irValueType>(value);
    }

    irValueType &irFuncType::getParamType(int64_t i) {
        return argsType[i];
    }

    irValueType &irFuncType::getResultType() {
        if (resultType)
            return *resultType;
        else
            throw std::runtime_error("hoshi::irFuncType: resultType is nullptr");
    }

    int64_t irFuncType::setArgsType(const wstr &name, const irValueType &value) {
        return argsType.put(name, value);
    }

    int64_t irFuncTable::putFunc(const wstr &name, const irFunc &func) {
        return funcs.put(name, func);
    }

    irFunc &irFuncTable::getFunc(int64_t i) {
        return funcs[i];
    }

    void irFunc::setFuncType(const irFuncType &v) {
        funcType = std::make_shared<irFuncType>(v);
    }

    irFuncType &irFunc::getFuncType() {
        if (funcType)
            return *funcType;
        else
            throw std::runtime_error("hoshi::irFunc: funcType is nullptr");
    }

    int64_t irFunc::newBasicBlock(const wstr &name) {
        entryBlock = 0;
        return basicBlocks.put(name, {});
    }

    irBasicBlock &irFunc::getBasicBlock(int64_t i) {
        return basicBlocks[i];
    }

    void irFunc::setEntryBlock(int64_t i) {
        entryBlock = i;
    }

    int64_t irFunc::getEntryBlock() const {
        return entryBlock;
    }

    void irValueType::set(irValueType::type t, int64_t idx) {
        tp = t, typeIndex = idx;
    }

    void irValueType::set(const std::shared_ptr<irValueType> &v) {
        ptrVal = v;
    }

    int64_t irVarTable::setFieldType(const wstr &name, const irValueType &value) {
        return fields.put(name, value);
    }

    irValueType &irVarTable::getFieldType(int64_t i) {
        return fields[i];
    }

    int64_t irVarTable::getFieldIndex(const wstr &name) {
        return fields.getIndex(name);
    }

    void irConstVal::set(irConstVal::type t, int64_t val) {
        tp = t;
        v = val;
    }

    void irConstVal::set(irConstVal::type t, bool val) {
        tp = t;
        v = (int64_t) val;
    }

    bool irConstVal::getBool() {
        return (bool) v;
    }

    int64_t irConstVal::getDeciIdx() {
        return v;
    }

    int64_t irConstVal::getInt() {
        return v;
    }

    bool irConstVal::operator==(const irConstVal &rhs) const {
        return tp == rhs.tp && v == rhs.v;
    }

    void irBasicBlock::putInst(const irInstruction &ins) {
        inst.push_back(ins);
    }

    int64_t irTempVarTable::put(const irValueType &v) {
        fields.push_back(v);
        return (int64_t) fields.size() - 1;
    }

    irValueType &irTempVarTable::get(int64_t v) {
        if (v < fields.size())
            return fields[v];
        else
            throw std::runtime_error("hoshi::irTempVarTable: invalid index");
    }

    int64_t irFuncConstantPool::put(const irConstVal &v) {
        return fields.put(v);
    }

    irConstVal &irFuncConstantPool::get(int64_t v) {
        return fields[v];
    }

    void irBuilder::setModule(int64_t v) {
        moduleId = v;
    }

    void irBuilder::setInsertPoint(int64_t f, int64_t b) {
        funcId = f;
        bbId = b;
    }

    irBuilder::irBuilder(irContext *cxt) : cxt(cxt) {

    }

    void irBuilder::putInst(const irInstruction &ins) {
        cxt->getModule(moduleId).funcTable.getFunc(funcId).getBasicBlock(bbId).putInst(ins);
    }

    irFunc &irBuilder::getFunc() {
        return cxt->getModule(moduleId).funcTable.getFunc(funcId);
    }

    irModule &irBuilder::getModule() {
        return cxt->getModule(moduleId);
    }

    irModule &irContext::getModule(int64_t idx) {
        return modules[idx];
    }

    int64_t irContext::getModuleIdx(const wstr &name) {
        return modules.getIndex(name);
    }

    int64_t irContext::newModule(const wstr &name) {
        return modules.put(name, {});
    }
}