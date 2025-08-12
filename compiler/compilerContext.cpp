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
#include "share/def.hpp"
#include "visitor/visitor.h"
#include <stdexcept>

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
    
    const std::map<indexT, std::shared_ptr<IRModule>> &compilerContext::getCompiledModules() const {
        return moduleImported;
    }


    yoi::indexT compilerContext::getModuleIndexByRealPath(const yoi::wstr &modRealPath) {
        return modules.getIndex(modRealPath);
    }

    yoi::indexT compilerContext::compileModule(const yoi::wstr &filepath) {
        yoi::wstr rFilepath;
        for (auto &prep : buildConfig->searchPaths) {
            try {
                std::filesystem::path final = prep / std::filesystem::path(filepath);
                rFilepath = realpath(final.wstring());
                break;
            } catch (std::runtime_error &e) {
                continue;
            }
        }
        try {
            return modules.getIndex(rFilepath);
        } catch (const std::out_of_range &e) {
            auto fp = fopen(wstring2string(rFilepath).c_str(), "r");
            if (!fp)
                throw std::runtime_error("invalid filename: " + wstring2string(rFilepath));
            fseek(fp, 0, SEEK_END);
            auto size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            auto *a = new std::string(size, 0);
            fread(a->data(), size, 1, fp);
            auto *b = new yoi::wstr{yoi::string2wstring(*a)};
            delete a;
            fclose(fp);

            yoi::lexer l{std::wstringstream(*b)};
            l.scan();
            delete b;
            hoshiModule *mod;
            yoi::parse(mod, l);
            std::shared_ptr<moduleContext> modCtx = std::make_shared<moduleContext>(shared_from_this(), rFilepath, mod);
            std::shared_ptr<IRModule> irMod = std::make_shared<IRModule>();
            irMod->modulePath = rFilepath;
            auto idx = modules.put(rFilepath, modCtx);
            irMod->identifier = idx;
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

        sharedValueType.put(L"foreignInt32Type", managedPtr(getForeignInt32Object()));
        sharedValueType.put(L"foreignFloatType", managedPtr(getForeignFloatObject()));

        irFFITable = std::make_shared<IRFFITable>();
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
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {sharedObjectDefinition.getIndex(L"int")}
        };
    }

    yoi::IRValueType compilerContext::getBoolObject() {
        return {
            IRValueType::valueType::booleanObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {sharedObjectDefinition.getIndex(L"bool")}
        };
    }

    yoi::IRValueType compilerContext::getDeciObject() {
        return {
            IRValueType::valueType::decimalObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {sharedObjectDefinition.getIndex(L"deci")}
        };
    }

    yoi::IRValueType compilerContext::getStrObject() {
        return {
            IRValueType::valueType::stringObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {sharedObjectDefinition.getIndex(L"string")}
        };
    }

    yoi::IRValueType compilerContext::getNoneObject() {
        return {IRValueType::valueType::none, static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST), {}};
    }

    yoi::IRValueType compilerContext::getCharObject() {
        return {
            IRValueType::valueType::characterObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {sharedObjectDefinition.getIndex(L"char")}
        };
    }

    void compilerContext::setIRObjectFile(
        const std::shared_ptr<IRObjectFile> &irObjectFile) {
      this->irObjectFile = irObjectFile;
    }

    std::shared_ptr<IRBuildConfig> compilerContext::getBuildConfig() const {
      return buildConfig;
    }

    void compilerContext::setBuildConfig(
        const std::shared_ptr<IRBuildConfig> &buildConfig) {
      this->buildConfig = buildConfig;
    }

    std::shared_ptr<IRFFITable> compilerContext::getIRFFITable() {
        return irFFITable;
    }

    yoi::IRValueType compilerContext::getForeignInt32Object() {
        return {
            IRValueType::valueType::foreignInt32Type,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType compilerContext::getForeignFloatObject() {
        return {
            IRValueType::valueType::foreignFloatType,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getForeignInt32ObjectType() {
        return sharedValueType[L"foreignInt32Type"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getForeignFloatObjectType() {
        return sharedValueType[L"foreignFloatType"];
    }
} // namespace yoi