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

    class compilerContext : public std::enable_shared_from_this<compilerContext> {
        yoi::indexTable<yoi::wstr, std::shared_ptr<yoi::moduleContext>> modules;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRStructDefinition>> sharedObjectDefinition;
        std::map<yoi::indexT, bool> isModuleImported;
        std::shared_ptr<IRObjectFile> irObjectFile;

    public:
        compilerContext() = default;

        compilerContext(const compilerContext& context) = default;

        void compileModule(const yoi::wstr &filepath);

        const std::shared_ptr<IRObjectFile>& getIRObjectFile() const;

        static yoi::IRStructDefinition getIntObjectDefinition();

        static yoi::IRStructDefinition getBooleanObjectDefinition();

        static yoi::IRStructDefinition getDecimalObjectDefinition();

        static yoi::IRStructDefinition getStringObjectDefinition();

        void initializeSharedObjects();

        yoi::IRValueType getIntObjectType();

        yoi::IRValueType getBoolObjectType();

        yoi::IRValueType getDeciObjectType();

        yoi::IRValueType getStrObjectType();
    };

} // yoi

#endif //HOSHI_LANG_COMPILERCONTEXT_H
