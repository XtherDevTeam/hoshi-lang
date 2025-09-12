//
// Created by XIaokang00010 on 2024/9/6.
//

#include "compilerContext.h"
#include "moduleContext.h"
#include "builtinModule.hpp"
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "ir/IR.h"
#include "ir/IROptimizer.hpp"
#include "share/def.hpp"
#include "visitor/visitor.h"
#include <filesystem>
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
        if (filepath != L"builtin") {
            for (auto &prep : buildConfig->searchPaths) {
                std::filesystem::path final = prep / std::filesystem::path(filepath);
                rFilepath = realpath(final.wstring());
                if (std::filesystem::exists(rFilepath) && std::filesystem::is_regular_file(rFilepath)) {
                    break;
                } else if (std::filesystem::exists(rFilepath + L".hoshi") && std::filesystem::is_regular_file(rFilepath + L".hoshi")) {
                    rFilepath += L".hoshi";
                    break;
                } else {
                    continue;
                }
            }
        } else {
            return HOSHI_COMPILER_CTX_GLOB_ID_CONST;
        }

        if (auto path = std::filesystem::path(rFilepath) / "index.hoshi"; std::filesystem::is_directory(rFilepath) && std::filesystem::exists(path)) {
            return compileModule(path.wstring());
        }
        
        try {
            return modules.getIndex(rFilepath);
        } catch (const std::out_of_range &e) {
            auto fp = fopen(wstring2string(rFilepath).c_str(), "rb");
            if (!fp)
                throw std::runtime_error("invalid filename: " + wstring2string(rFilepath));

            // temporarily add current directory to search path
            buildConfig->searchPaths.push_back(std::filesystem::path(filepath).parent_path().wstring());

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
            auto current_file = __current_file_path;
            set_current_file_path(rFilepath);
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
            
            astToFinalize.insert(mod);
            set_current_file_path(current_file);

            // pop current directory from search path
            buildConfig->searchPaths.pop_back();
            
            return idx;
        }
    }

    const std::shared_ptr<IRObjectFile> &compilerContext::getIRObjectFile() const {
        return irObjectFile;
    }

    void compilerContext::initializeSharedObjects() {
        auto builtinModule = std::make_shared<IRModule>();
        
        moduleImported[HOSHI_COMPILER_CTX_GLOB_ID_CONST] = builtinModule;

        builtinModuleBuilder = std::make_shared<BuiltinModuleBuilder>(builtinModule);
        builtinModuleBuilder->build();
        irFFITable = std::make_shared<IRFFITable>();

        lexer l{std::wstringstream(yoi::string2wstring(__yoi_builtin_module_hoshi))};
        l.scan();
        hoshiModule *mod;
        yoi::parse(mod, l);
        builtinModuleContext = std::make_shared<moduleContext>(shared_from_this(), L"builtin", mod);
        std::shared_ptr<visitor> vis = std::make_shared<visitor>(builtinModuleContext, builtinModule, HOSHI_COMPILER_CTX_GLOB_ID_CONST);
        vis->visit();
        astToFinalize.insert(mod);
        
        // pre-specialize the types required for runtime to get determined type ids
        vis->specializeStructTemplate(L"Result", {getUnsignedObjectType(), getIntObjectType()}, vis->irModule->templateImplAsts.at(L"Result"), HOSHI_COMPILER_CTX_GLOB_ID_CONST);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getIntObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getIntObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"int"]);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getBoolObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getBoolObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"bool"]);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getDeciObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getDeciObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"deci"]);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getStrObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getStrObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"string"]);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getCharObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getCharObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"char"]);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getNoneObjectType() {
        return builtinModuleBuilder->sharedValueType[L"none"];
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

    std::shared_ptr<yoi::IRValueType> compilerContext::getForeignInt32ObjectType() {
        return builtinModuleBuilder->sharedValueType[L"foreignInt32Type"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getForeignFloatObjectType() {
        return builtinModuleBuilder->sharedValueType[L"foreignFloatType"];
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getNullInterfaceType() {
        return builtinModuleBuilder->sharedValueType[L"NullInterface"];
    }
    
    std::shared_ptr<yoi::moduleContext> compilerContext::getModuleContext(yoi::indexT index) {
        return index == HOSHI_COMPILER_CTX_GLOB_ID_CONST ? builtinModuleContext : modules[index];
    }

    compilerContext::~compilerContext() {
        for (auto &i : astToFinalize) {
            finalizeAST(i);
        }
    }

    void compilerContext::runOptimizer() {
        IROptimizer opt{shared_from_this(), 0};
        opt.buildCallGraph();
        opt.optimize();
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getPointerType() {
        return builtinModuleBuilder->sharedValueType[L"ptr"];
    }
    yoi::IRValueType compilerContext::normalizeForeignBasicType(const std::shared_ptr<yoi::IRValueType> &type) {
        if (type->isForeignBasicType()) {
            switch (type->type) {
                case IRValueType::valueType::foreignInt32Type:
                    return *getIntObjectType();
                case IRValueType::valueType::pointer:
                    return *getUnsignedObjectType();
                case IRValueType::valueType::foreignFloatType:
                    return *getDeciObjectType();
                default:
                    throw std::runtime_error("unknown foreign basic type");
            }
        }
        return *type;
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getShortObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getShortObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"short"]);
    }

    std::shared_ptr<yoi::IRValueType> compilerContext::getUnsignedObjectType(bool forceRawAttr) {
        return forceRawAttr ? managedPtr(builtinModuleBuilder->getUnsignedObject()) : managedPtr(*builtinModuleBuilder->sharedValueType[L"unsigned"]);
    }
} // namespace yoi