//
// Created by XIaokang00010 on 2024/9/6.
//

#include "compilerContext.h"

#include "moduleContext.h"
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "ir/IR.h"
#include "ir/IROptimizer.hpp"
#include "visitor/visitor.h"

namespace yoi {
    std::shared_ptr<IRModule> compilerContext::getImportedModule(yoi::indexT index) {
        return moduleImported[index];
    }

    std::shared_ptr<IRModule> compilerContext::getImportedModule(const yoi::wstr &modRealPath) {
        try {
            auto it = moduleImported.find(getModuleIndexByRealPath( modRealPath ));
            if (it == moduleImported.end()) {
                return nullptr;
            }
            return it->second;
        } catch (const std::runtime_error &e) {
            return nullptr;
        }
    }

    yoi::indexT compilerContext::getModuleIndexByRealPath(const yoi::wstr &modRealPath) {
        return modules.getIndex(modRealPath);
    }

    yoi::indexT compilerContext::compileModule(const yoi::wstr &filepath) {
        auto fp = fopen(wstring2string(realpath(filepath)).c_str(), "r+");
        if (!fp)
            throw std::runtime_error("invalid filename");
        fseek(fp, 0, SEEK_END);
        auto size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto *a = new std::string(size, 0);
        fread(a->data(), size, 1, fp);
        auto *b = new yoi::wstr{yoi::string2wstring(*a)};
        delete a;

        yoi::lexer l{std::wstringstream(*b)};
        l.scan();
        delete b;
        hoshiModule *mod;
        yoi::parse(mod, l);
        std::shared_ptr<moduleContext> modCtx = std::make_shared<moduleContext>(shared_from_this(), filepath, mod);
        std::shared_ptr<IRModule> irMod = std::make_shared<IRModule>();
        auto idx = modules.put(filepath, modCtx);
        moduleImported[idx] = irMod;
        std::shared_ptr<visitor> vis = std::make_shared<visitor>(modCtx, irMod, idx);
        vis->visit();
        for (auto &i : irMod->functionTable) {
            IROptimizer optimizer{shared_from_this(), irMod};
            optimizer.setTargetFunction(i.second).doOptimizationForCurrentFunction();
        }
        finalizeAST(mod);
        return idx;
    }

    const std::shared_ptr<IRObjectFile> &compilerContext::getIRObjectFile() const {
        return irObjectFile;
    }

    yoi::IRStructDefinition compilerContext::getIntObjectDefinition() {
        IRValueType valType{
            IRValueType::valueType::integerRaw,
        };
        IRStructDefinition::nameInfo info{
            IRStructDefinition::nameInfo::nameType::field,

        };
        yoi::IRStructDefinition def{
            L"int",
            {{L"ptr", info}},
            {managedPtr(valType)}
        };
        return def;
    }

    yoi::IRStructDefinition compilerContext::getBooleanObjectDefinition() {
        IRValueType valType{
            IRValueType::valueType::booleanRaw,
        };
        IRStructDefinition::nameInfo info{
            IRStructDefinition::nameInfo::nameType::field,

        };
        yoi::IRStructDefinition def{
            L"bool",
            {{L"ptr", info}},
            {managedPtr(valType)}
        };
        return def;
    }

    yoi::IRStructDefinition compilerContext::getDecimalObjectDefinition() {
        IRValueType valType{
            IRValueType::valueType::decimalRaw,
        };
        IRStructDefinition::nameInfo info{
            IRStructDefinition::nameInfo::nameType::field,

        };
        yoi::IRStructDefinition def{
            L"deci",
            {{L"ptr", info}},
            {managedPtr(valType)}
        };
        return def;
    }

    yoi::IRStructDefinition compilerContext::getStringObjectDefinition() {
        IRValueType valType{
            IRValueType::valueType::stringLiteral,
        };
        IRStructDefinition::nameInfo info{
            IRStructDefinition::nameInfo::nameType::field,

        };
        yoi::IRStructDefinition def{
            L"string",
            {{L"ptr", info}},
            {managedPtr(valType)}
        };
        return def;
    }

    yoi::IRStructDefinition compilerContext::getCharObjectDefinition() {
        IRValueType valType{
            IRValueType::valueType::charRaw,
        };
        IRStructDefinition::nameInfo info{
            IRStructDefinition::nameInfo::nameType::field,
        };
        yoi::IRStructDefinition def{
            L"char",
            {{L"ptr", info}},
            {managedPtr(valType)}
        };
        return def;
    }

    void compilerContext::initializeSharedObjects() {
        sharedObjectDefinition.put(L"int", managedPtr(getIntObjectDefinition()));
        sharedObjectDefinition.put(L"bool", managedPtr(getBooleanObjectDefinition()));
        sharedObjectDefinition.put(L"deci", managedPtr(getDecimalObjectDefinition()));
        sharedObjectDefinition.put(L"string", managedPtr(getStringObjectDefinition()));
        sharedObjectDefinition.put(L"char", managedPtr(getCharObjectDefinition()));

        sharedValueType.put(L"int", managedPtr(getIntObject()));
        sharedValueType.put(L"bool", managedPtr(getBoolObject()));
        sharedValueType.put(L"deci", managedPtr(getDeciObject()));
        sharedValueType.put(L"string", managedPtr(getStrObject()));
        sharedValueType.put(L"char", managedPtr(getCharObject()));
        sharedValueType.put(L"none", managedPtr(getNoneObject()));
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getIntObjectType() {
        return sharedValueType[L"int"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getBoolObjectType() {
        return sharedValueType[L"bool"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getDeciObjectType() {
        return sharedValueType[L"deci"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getStrObjectType() {
        return sharedValueType[L"string"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getCharObjectType() {
        return sharedValueType[L"char"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getNoneObjectType() {
        return sharedValueType[L"none"];
    }

    yoi::IRValueType compilerContext::getIntObject() {
        return {
            IRValueType::valueType::integerObject,
            static_cast<yoi::indexT>(-1),
            {sharedObjectDefinition.getIndex(L"int")}
        };
    }

    yoi::IRValueType compilerContext::getBoolObject() {
        return {
            IRValueType::valueType::booleanObject,
            static_cast<yoi::indexT>(-1),
            {sharedObjectDefinition.getIndex(L"bool")}
        };
    }

    yoi::IRValueType compilerContext::getDeciObject() {
        return {
            IRValueType::valueType::decimalObject,
            static_cast<yoi::indexT>(-1),
            {sharedObjectDefinition.getIndex(L"deci")}
        };
    }

    yoi::IRValueType compilerContext::getStrObject() {
        return {
            IRValueType::valueType::stringObject,
            static_cast<yoi::indexT>(-1),
            {sharedObjectDefinition.getIndex(L"string")}
        };
    }

    yoi::IRValueType compilerContext::getNoneObject() {
        return {IRValueType::valueType::none, static_cast<yoi::indexT>(-1), {}};
    }

    yoi::IRValueType compilerContext::getCharObject() {
        return {
            IRValueType::valueType::characterObject,
            static_cast<yoi::indexT>(-1),
            {sharedObjectDefinition.getIndex(L"char")}
        };
    }
} // yoi