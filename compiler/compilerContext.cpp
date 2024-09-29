//
// Created by XIaokang00010 on 2024/9/6.
//

#include "compilerContext.h"

#include "moduleContext.h"
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "ir/IR.h"
#include "visitor/visitor.h"

namespace yoi {
    void compilerContext::compileModule(const yoi::wstr &filepath) {
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
        auto idx = modules.put(filepath, modCtx);
        std::shared_ptr<visitor> vis = std::make_shared<visitor>(modCtx);
        vis->visit();
        isModuleImported[idx] = true;
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
            {managedPtr(valType)},
            {}
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
            {managedPtr(valType)},
            {}
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
            {managedPtr(valType)},
            {}
        };
        return def;
    }

    yoi::IRStructDefinition compilerContext::getStringObjectDefinition() {
        IRValueType valType{
            IRValueType::valueType::integerRaw,
        };
        IRStructDefinition::nameInfo info{
            IRStructDefinition::nameInfo::nameType::field,

        };
        yoi::IRStructDefinition def{
            L"string",
            {{L"ptr", info}},
            {managedPtr(valType)},
            {}
        };
        return def;
    }

    void compilerContext::initializeSharedObjects() {
        sharedObjectDefinition.put(L"int", managedPtr(getIntObjectDefinition()));
        sharedObjectDefinition.put(L"bool", managedPtr(getBooleanObjectDefinition()));
        sharedObjectDefinition.put(L"deci", managedPtr(getDecimalObjectDefinition()));
        sharedObjectDefinition.put(L"string", managedPtr(getStringObjectDefinition()));
    }

    yoi::IRValueType compilerContext::getIntObjectType() {
        return {
            IRValueType::valueType::integerObject,
            {sharedObjectDefinition.getIndex(L"int")}
        };
    }

    yoi::IRValueType compilerContext::getBoolObjectType() {
        return {
            IRValueType::valueType::booleanObject,
            {sharedObjectDefinition.getIndex(L"bool")}
        };
    }

    yoi::IRValueType compilerContext::getDeciObjectType() {
        return {
            IRValueType::valueType::decimalObject,
            {sharedObjectDefinition.getIndex(L"deci")}
        };
    }

    yoi::IRValueType compilerContext::getStrObjectType() {
        return {
            IRValueType::valueType::stringObject,
            {sharedObjectDefinition.getIndex(L"string")}
        };
    }
} // yoi