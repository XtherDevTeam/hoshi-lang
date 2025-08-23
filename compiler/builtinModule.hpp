//
// Created by XIaokang00010 on 2025/8/14.
//

#ifndef HOSHI_LANG_BUILTIN_MODULE_HPP
#define HOSHI_LANG_BUILTIN_MODULE_HPP

#define HOSHI_COMPILER_CTX_GLOB_ID_CONST 0xe1751aff

#include <compiler/ir/IR.h>

#include <utility>

namespace yoi {
    class BuiltinModuleBuilder {
        std::shared_ptr<IRModule> module;
        static yoi::IRStructDefinition getIntObjectDefinition();

        static yoi::IRStructDefinition getBooleanObjectDefinition();

        static yoi::IRStructDefinition getDecimalObjectDefinition();

        static yoi::IRStructDefinition getStringObjectDefinition();

        static yoi::IRStructDefinition getCharObjectDefinition();

        static yoi::IRInterfaceInstanceDefinition getNullInterfaceInstanceDefinition();

        void initializeSharedObjectDefinitions();

        void initializeSharedObjects();

      public:
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> sharedValueType;

        BuiltinModuleBuilder() = default;

        BuiltinModuleBuilder(std::shared_ptr<IRModule> module);

        void build();

        yoi::IRValueType getIntObject();

        yoi::IRValueType getBoolObject();

        yoi::IRValueType getDeciObject();

        yoi::IRValueType getStrObject();

        yoi::IRValueType getNoneObject();

        yoi::IRValueType getCharObject();

        yoi::IRValueType getForeignInt32Object();

        yoi::IRValueType getForeignFloatObject();

        yoi::IRValueType getPointerObject();
    };
}

#endif // HOSHI_LANG_BUILTIN_MODULE_HPP