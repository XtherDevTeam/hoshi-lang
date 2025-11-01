//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_COMPILERCONTEXT_H
#define HOSHI_LANG_COMPILERCONTEXT_H

static const char *__yoi_builtin_module_hoshi =
#include <compiler/builtinModule.hoshi>
    ;

#include "share/def.hpp"
#include <compiler/frontend/ast.hpp>
#include <map>
#include <memory>
#include <set>

namespace yoi {
    class moduleContext;

    class IRObjectFile;

    class IRStructDefinition;

    class IRValueType;

    class IRModule;

    class IRBuildConfig;

    class IRFFITable;

    class BuiltinModuleBuilder;

    class compilerContext : public std::enable_shared_from_this<compilerContext> {
        yoi::indexTable<yoi::wstr, std::shared_ptr<yoi::moduleContext>> modules;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> sharedValueType;

        std::map<yoi::indexT, std::shared_ptr<IRModule>> moduleImported;
        std::shared_ptr<IRObjectFile> irObjectFile;
        std::shared_ptr<IRBuildConfig> buildConfig;
        std::shared_ptr<IRFFITable> irFFITable;
        std::shared_ptr<BuiltinModuleBuilder> builtinModuleBuilder;
        std::set<hoshiModule *> astToFinalize;
        std::shared_ptr<moduleContext> builtinModuleContext;

    public:
        compilerContext() = default;

        compilerContext(const compilerContext &context) = default;

        /**
         * Get the IRModule by module name.
         * @param index The index of the module.
         * @return The IRModule.
         */
        std::shared_ptr<IRModule> getImportedModule(yoi::indexT index);

        /**
         * @brief get module context by index
         *
         * @param index The index of the module.
         */
        std::shared_ptr<yoi::moduleContext> getModuleContext(yoi::indexT index);

        /**
         * Get the IRModule by module real path.
         * @param modRealPath The real path of the module.
         * @return The IRModule, nullptr if not found.
         */
        std::shared_ptr<IRModule> getImportedModule(const yoi::wstr &modRealPath);

        /**
         * Get all compiled IR modules.
         * @return A map from module ID to the IRModule.
         */
        const std::map<yoi::indexT, std::shared_ptr<IRModule>> &getCompiledModules() const;

        /**
         * Get the index of the module by module real path.
         * @param modRealPath The real path of the module.
         * @return The index of the module.
         * @throws std::runtime_error if the module is not found.
         */
        yoi::indexT getModuleIndexByRealPath(const yoi::wstr &modRealPath);

        yoi::indexT compileModule(const yoi::wstr &filepath);

        const std::shared_ptr<IRObjectFile> &getIRObjectFile() const;

        void setIRObjectFile(const std::shared_ptr<IRObjectFile> &irObjectFile);

        void initializeSharedObjects();

        std::shared_ptr<yoi::IRValueType> getIntObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getBoolObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getDeciObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getStrObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getCharObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getShortObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getUnsignedObjectType(bool forceRawAttr = false);

        std::shared_ptr<yoi::IRValueType> getNoneObjectType();

        std::shared_ptr<yoi::IRValueType> getForeignInt32ObjectType();

        std::shared_ptr<yoi::IRValueType> getForeignFloatObjectType();

        std::shared_ptr<yoi::IRValueType> getPointerType();

        std::shared_ptr<yoi::IRValueType> getNullInterfaceType();

        yoi::IRValueType normalizeForeignBasicType(const std::shared_ptr<yoi::IRValueType> &type, bool handlePointer = true);

        // i figured it out, all wrapper struct type should save in IRFFITable independently, they are not import type
        // or export type. they are just wrapper types. as for import function wrapper and export function wrapper, ofc
        // we need to treat it differently or in another word, the differentiation of import and export struct type
        // should not even exist. cuz you can declare a struct object and export it as foreign type which can be used in
        // import function wrapper too.

        std::shared_ptr<IRBuildConfig> getBuildConfig() const;

        void setBuildConfig(const std::shared_ptr<IRBuildConfig> &buildConfig);

        std::shared_ptr<IRFFITable> getIRFFITable();

        void runOptimizer();
        
        ~compilerContext();
    };

} // namespace yoi

#endif // HOSHI_LANG_COMPILERCONTEXT_H