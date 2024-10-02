//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_COMPILERCONTEXT_H
#define HOSHI_LANG_COMPILERCONTEXT_H

#include "share/def.hpp"
#include <map>

namespace yoi {
    class moduleContext;

    class IRObjectFile;

    class IRStructDefinition;

    class IRValueType;

    class IRModule;

    class compilerContext : public std::enable_shared_from_this<compilerContext> {
        yoi::indexTable<yoi::wstr, std::shared_ptr<yoi::moduleContext>> modules;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRStructDefinition>> sharedObjectDefinition;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> sharedValueType;
        std::map<yoi::indexT, std::shared_ptr<IRModule>> moduleImported;
        std::shared_ptr<IRObjectFile> irObjectFile;

        yoi::IRValueType getIntObject();

        yoi::IRValueType getBoolObject();

        yoi::IRValueType getDeciObject();

        yoi::IRValueType getStrObject();

        yoi::IRValueType getNoneObject();

        yoi::IRValueType getCharObject();

    public:
        compilerContext() = default;

        compilerContext(const compilerContext& context) = default;

        /**
         * Get the IRModule by module name.
         * @param index The index of the module.
         * @return The IRModule.
         */
        std::shared_ptr<IRModule> getImportedModule(yoi::indexT index);

        /**
         * Get the IRModule by module real path.
         * @param modRealPath The real path of the module.
         * @return The IRModule, nullptr if not found.
         */
        std::shared_ptr<IRModule> getImportedModule(const yoi::wstr &modRealPath);

        /**
         * Get the index of the module by module real path.
         * @param modRealPath The real path of the module.
         * @return The index of the module.
         * @throws std::runtime_error if the module is not found.
         */
        yoi::indexT getModuleIndexByRealPath(const yoi::wstr &modRealPath);

        yoi::indexT compileModule(const yoi::wstr &filepath);

        const std::shared_ptr<IRObjectFile>& getIRObjectFile() const;

        static yoi::IRStructDefinition getIntObjectDefinition();

        static yoi::IRStructDefinition getBooleanObjectDefinition();

        static yoi::IRStructDefinition getDecimalObjectDefinition();

        static yoi::IRStructDefinition getStringObjectDefinition();

        static yoi::IRStructDefinition getCharObjectDefinition();

        void initializeSharedObjects();

        std::shared_ptr<yoi::IRValueType> getIntObjectType();

        std::shared_ptr<yoi::IRValueType> getBoolObjectType();

        std::shared_ptr<yoi::IRValueType> getDeciObjectType();

        std::shared_ptr<yoi::IRValueType> getStrObjectType();

        std::shared_ptr<yoi::IRValueType> getCharObjectType();

        std::shared_ptr<yoi::IRValueType> getNoneObjectType();
    };

} // yoi

#endif //HOSHI_LANG_COMPILERCONTEXT_H
