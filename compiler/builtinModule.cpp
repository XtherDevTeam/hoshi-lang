#include "builtinModule.hpp"
#include "share/def.hpp"

namespace yoi {
    yoi::IRValueType BuiltinModuleBuilder::getIntObject() {
        return {
            IRValueType::valueType::integerObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType BuiltinModuleBuilder::getBoolObject() {
        return {
            IRValueType::valueType::booleanObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType BuiltinModuleBuilder::getDeciObject() {
        return {
            IRValueType::valueType::decimalObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType BuiltinModuleBuilder::getStrObject() {
        return {
            IRValueType::valueType::stringObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType BuiltinModuleBuilder::getNoneObject() {
        return {IRValueType::valueType::none, static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST), {}};
    }

    yoi::IRValueType BuiltinModuleBuilder::getCharObject() {
        return {
            IRValueType::valueType::characterObject,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType BuiltinModuleBuilder::getForeignInt32Object() {
        return {
            IRValueType::valueType::foreignInt32Type,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    yoi::IRValueType BuiltinModuleBuilder::getForeignFloatObject() {
        return {
            IRValueType::valueType::foreignFloatType,
            static_cast<yoi::indexT>(HOSHI_COMPILER_CTX_GLOB_ID_CONST),
            {}
        };
    }

    void BuiltinModuleBuilder::initializeSharedObjectDefinitions() {
        module->interfaceTable.put_create(L"NullInterface", managedPtr(getNullInterfaceInstanceDefinition()));
    }

    void BuiltinModuleBuilder::initializeSharedObjects() {
        sharedValueType.put(L"int", managedPtr(getIntObject()));
        sharedValueType.put(L"bool", managedPtr(getBoolObject()));
        sharedValueType.put(L"deci", managedPtr(getDeciObject()));
        sharedValueType.put(L"string", managedPtr(getStrObject()));
        sharedValueType.put(L"char", managedPtr(getCharObject()));
        sharedValueType.put(L"none", managedPtr(getNoneObject()));

        sharedValueType.put(L"foreignInt32Type", managedPtr(getForeignInt32Object()));
        sharedValueType.put(L"foreignFloatType", managedPtr(getForeignFloatObject()));

        for (auto &sharedValue : sharedValueType) {
            if (sharedValue.second->isForeignBasicType())
                continue;
            auto nullInterface = std::make_pair(HOSHI_COMPILER_CTX_GLOB_ID_CONST, 0);
            auto nullImplName = L"interfaceImpl#" + std::to_wstring(HOSHI_COMPILER_CTX_GLOB_ID_CONST) + L"#0#" + sharedValue.second->to_string();
            module->interfaceTable[0]->implementations.emplace_back(
                sharedValue.second->type, sharedValue.second->typeAffiliateModule, sharedValue.second->typeIndex);
            auto nullImpl = managedPtr(IRInterfaceImplementationDefinition{
                nullImplName, 
                {sharedValue.second->type, sharedValue.second->typeAffiliateModule, sharedValue.second->typeIndex},
                0,
                {},
                {}});
            module->interfaceImplementationTable.put_create(nullImplName, nullImpl);
        }
    }

    yoi::IRInterfaceInstanceDefinition BuiltinModuleBuilder::getNullInterfaceInstanceDefinition() {
        return {L"NullInterface", {}};
    }

    BuiltinModuleBuilder::BuiltinModuleBuilder(std::shared_ptr<IRModule> module) : module(std::move(module)) {}

    void BuiltinModuleBuilder::build() {
        module->modulePath = L"builtin";
        module->identifier = HOSHI_COMPILER_CTX_GLOB_ID_CONST;

        initializeSharedObjectDefinitions();
        initializeSharedObjects();
    }
} // namespace yoi