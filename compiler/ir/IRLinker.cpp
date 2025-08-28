//
// Created by XIaokang00010 on 2024/10/12.
//

#include "IRLinker.hpp"
#include "compiler/compilerContext.h"
#include "compiler/ir/IR.h"
#include "share/def.hpp"
#include <iomanip>

namespace yoi {

    IRLinker::IRLinker() = default;

    std::shared_ptr<IRObjectFile> IRLinker::link(const std::shared_ptr<compilerContext>& context, indexT entryId) {
        this->compilerCtx = context;
        this->entryModuleId = entryId;
        this->finalModule = std::make_shared<IRModule>();
        finalModule->identifier = ENTRY_MODULE_ID_CONST; // Unified module has no specific ID, elysia here

        auto objectFile = std::make_shared<IRObjectFile>();
        objectFile->compiledModule = this->finalModule;
        objectFile->entryModule = this->entryModuleId;
        
        // The order is important:
        // 1. Strings, Structs, Interfaces: Define types first.
        // 2. Globals: Depend on types.
        // 3. Functions: Depend on everything.
        linkStringLiterals();
        linkStructsAndInterfaces();
        linkGlobals();
        linkFunctions();
        linkInterfaceImplementations();
        patchIRFFITable();
        createEntryFunction();

        return objectFile;
    }

    wstr IRLinker::mangleName(indexT moduleId, const wstr& originalName) {
        // only when it's not a main function, we need to mangle the name
        if (moduleId == entryModuleId && originalName == L"main#") {
            return L"yoi_main";
        }
        std::wstringstream ss;
        ss << std::hex << moduleId;
        return ss.str() + L"_" + originalName;
    }

    void IRLinker::linkStringLiterals() {
        for (const auto& modPair : compilerCtx->getCompiledModules()) {
            indexT modId = modPair.first;
            const auto& srcModule = modPair.second;
            for (indexT oldIdx = 0; oldIdx < srcModule->stringLiteralPool.pool.size(); ++oldIdx) {
                const auto& str = srcModule->stringLiteralPool.pool[oldIdx];
                indexT newIdx = finalModule->stringLiteralPool.addStringLiteral(str);
                stringRemapping[modId][oldIdx] = newIdx;
            }
        }
    }

    void IRLinker::linkStructsAndInterfaces() {
        for (const auto& modPair : compilerCtx->getCompiledModules()) {
            indexT modId = modPair.first;
            const auto& srcModule = modPair.second;

            // Link Structs
            for (const auto& structPair : srcModule->structTable) {
                indexT oldIdx = srcModule->structTable.getIndex(structPair.first);
                auto newName = mangleName(modId, structPair.second->name);
                // Just copy the definition, field types will be patched later if needed
                indexT newIdx = finalModule->structTable.put_create(newName, structPair.second);
                finalModule->structTable[newIdx]->name = newName;
                for (auto &type : finalModule->structTable[newIdx]->fieldTypes) {
                    *type = *patchType(type);
                }
                structRemapping[modId][oldIdx] = newIdx;
            }

            // Link Interfaces
            for (const auto& ifacePair : srcModule->interfaceTable) {
                indexT oldIdx = srcModule->interfaceTable.getIndex(ifacePair.first);
                auto newName = mangleName(modId, ifacePair.second->name);
                indexT newIdx = finalModule->interfaceTable.put_create(newName, ifacePair.second);
                finalModule->interfaceTable[newIdx]->name = newName;

                for (auto &method : finalModule->interfaceTable[newIdx]->methodMap) {
                    for (auto &param : method.second->argumentTypes) {
                        *param = *patchType(param);
                    }
                    for (auto &ret : method.second->variableTable.getVariables()) {
                        *ret = *patchType(ret);
                    }
                    method.second->returnType = patchType(method.second->returnType);
                }

                interfaceRemapping[modId][oldIdx] = newIdx;
            }
            
            // Link Interface Implementations
            for (const auto& implPair : srcModule->interfaceImplementationTable) {
                indexT oldIdx = srcModule->interfaceImplementationTable.getIndex(implPair.first);
                auto newName = mangleName(modId, implPair.second->name);
                indexT newIdx = finalModule->interfaceImplementationTable.put_create(newName, implPair.second);
                finalModule->interfaceImplementationTable[newIdx]->name = newName;
                interfaceImplRemapping[modId][oldIdx] = newIdx;
            }
        }
    }

    void IRLinker::linkGlobals() {
        for (const auto& modPair : compilerCtx->getCompiledModules()) {
            indexT modId = modPair.first;
            const auto& srcModule = modPair.second;
            for (const auto& globalPair : srcModule->globalVariables) {
                indexT oldIdx = srcModule->globalVariables.getIndex(globalPair.first);
                auto newName = mangleName(modId, globalPair.first);
                // Type might need patching later, but for now just copy
                indexT newIdx = finalModule->globalVariables.put_create(newName, patchType(globalPair.second));
                globalRemapping[modId][oldIdx] = newIdx;
            }
        }
    }

    void IRLinker::linkFunctions() {
        // Pass 1: Create declarations for all functions to handle forward calls
        for (const auto& modPair : compilerCtx->getCompiledModules()) {
            indexT modId = modPair.first;
            const auto& srcModule = modPair.second;
            for (const auto& funcPair : srcModule->functionTable) {
                indexT oldIdx = srcModule->functionTable.getIndex(funcPair.first);
                auto newName = mangleName(modId, funcPair.second->name);

                // Create a shell definition. The body will be filled in Pass 2.
                auto newFuncDef = std::make_shared<IRFunctionDefinition>(*funcPair.second);
                newFuncDef->name = newName;
                newFuncDef->codeBlock.clear();

                for (auto &var : newFuncDef->variableTable.getVariables()) {
                    *var = *patchType(var);
                }
                for (auto &param : newFuncDef->argumentTypes) {
                    *param = *patchType(param);
                }

                indexT newIdx = finalModule->functionTable.put_create(newName, newFuncDef);
                functionRemapping[modId][oldIdx] = newIdx;

                if (funcPair.first == L"yoimiya_glob_initializer") {
                    globInitializerIndexes.emplace_back(newIdx);
                }
            }
        }

        // Pass 2: Link bodies and patch instructions
        for (const auto& modPair : compilerCtx->getCompiledModules()) {
            indexT modId = modPair.first;
            const auto& srcModule = modPair.second;
            for (const auto& funcPair : srcModule->functionTable) {
                indexT oldIdx = srcModule->functionTable.getIndex(funcPair.first);
                indexT newIdx = functionRemapping.at(modId).at(oldIdx);

                const auto& srcFunc = funcPair.second;
                auto& destFunc = finalModule->functionTable[newIdx];
                
                // Copy variable table, local vars don't need remapping
                destFunc->returnType = patchType(srcFunc->returnType);
                destFunc->variableTable = srcFunc->variableTable;

                // Copy and patch code blocks
                for (const auto& srcBlock : srcFunc->codeBlock) {
                    auto newBlock = std::make_shared<IRCodeBlock>();
                    for (const auto& srcInstr : srcBlock->getIRArray()) {
                        newBlock->insert(patchInstruction(srcInstr, modId));
                    }
                    destFunc->codeBlock.push_back(newBlock);
                }
            }
        }
    }

    IR IRLinker::patchInstruction(const IR& instr, indexT currentModuleId) {
        IR newInstr = instr;
        switch (instr.opcode) {
            case IR::Opcode::push_string: {
                auto moduleId = instr.operands[0].value.symbolIndex;
                auto stringIndex = instr.operands[1].value.symbolIndex;
                auto newStringIndex = stringRemapping.at(moduleId).at(stringIndex);
                newInstr.operands[1].value.symbolIndex = newStringIndex;
                break;
            }
            case IR::Opcode::invoke:
            case IR::Opcode::invoke_virtual:
            case IR::Opcode::load_global:
            case IR::Opcode::new_struct:
            case IR::Opcode::new_interface:
            case IR::Opcode::new_array_struct:
            case IR::Opcode::new_array_interface:
            case IR::Opcode::new_dynamic_array_struct:
            case IR::Opcode::new_dynamic_array_interface:
            case IR::Opcode::construct_interface_impl: 
            case IR::Opcode::typeid_struct:
            case IR::Opcode::typeid_interface:
            case IR::Opcode::dyn_cast_struct: {
                auto moduleId = instr.operands[0].value.symbolIndex;
                auto symbolIndex = instr.operands[1].value.symbolIndex;
                newInstr.operands[0].value.symbolIndex = ENTRY_MODULE_ID_CONST;
                switch (instr.opcode) {
                    case IR::Opcode::invoke:
                        newInstr.operands[1].value.symbolIndex = functionRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::load_global:
                        newInstr.operands[1].value.symbolIndex = globalRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::new_struct:
                    case IR::Opcode::new_array_struct:
                    case IR::Opcode::new_dynamic_array_struct:
                        newInstr.operands[1].value.symbolIndex = structRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::new_interface:
                    case IR::Opcode::invoke_virtual:
                    case IR::Opcode::new_array_interface:
                    case IR::Opcode::new_dynamic_array_interface:
                        newInstr.operands[1].value.symbolIndex = interfaceRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::construct_interface_impl:
                        newInstr.operands[1].value.symbolIndex = interfaceImplRemapping.at(moduleId).at(symbolIndex);
                        break;
                    default: break;
                }
            }
            case IR::Opcode::typeid_object_non_stack:
            case IR::Opcode::dyn_cast_any: {
                auto valueType = static_cast<IRValueType::valueType>(instr.operands[0].value.symbolIndex);
                switch (valueType) {
                    case IRValueType::valueType::structObject:
                        newInstr.operands[1].value.symbolIndex = ENTRY_MODULE_ID_CONST;
                        newInstr.operands[2].value.symbolIndex = structRemapping.at(instr.operands[1].value.symbolIndex).at(instr.operands[2].value.symbolIndex);
                        break;
                    case IRValueType::valueType::interfaceObject:
                        newInstr.operands[1].value.symbolIndex = ENTRY_MODULE_ID_CONST;
                        newInstr.operands[2].value.symbolIndex = interfaceRemapping.at(instr.operands[1].value.symbolIndex).at(instr.operands[2].value.symbolIndex);
                        break;
                    default: 
                        break;
                }
                break;
            }
            default:
                break;
        }

        return newInstr;
    }

    std::shared_ptr<IRValueType> IRLinker::patchType(const std::shared_ptr<IRValueType> &oldType) {
        // map the old type to the new type
        if (oldType->typeAffiliateModule == ENTRY_MODULE_ID_CONST) {
            return oldType;
        }
        std::shared_ptr<IRValueType> newType = managedPtr(*oldType);
        switch (oldType->type) {
            case IRValueType::valueType::structObject: {
                auto newIndex = structRemapping[oldType->typeAffiliateModule][oldType->typeIndex];
                newType->typeAffiliateModule = ENTRY_MODULE_ID_CONST;
                newType->typeIndex = newIndex;
                break;
            }
            case IRValueType::valueType::interfaceObject: {
                auto newIndex = interfaceRemapping[oldType->typeAffiliateModule][oldType->typeIndex];
                newType->typeAffiliateModule = ENTRY_MODULE_ID_CONST;
                newType->typeIndex = newIndex;
                break;
            }
            case IRValueType::valueType::virtualMethod: {
                auto newIndex = functionRemapping[oldType->typeAffiliateModule][oldType->typeIndex];
                newType->typeAffiliateModule = ENTRY_MODULE_ID_CONST;
                newType->typeIndex = newIndex;
                break;
            }
            default: {
                // Other types don't need patching
                break;
            }
        }
        return newType;
    }
    void IRLinker::createEntryFunction() {
        IRFunctionDefinition::Builder entryBuilder;
        auto entry = entryBuilder.setName(L"yoimiya_entry").setReturnType(compilerCtx->getIntObjectType()).setDebugInfo({L"<entry>", 0, 0}).yield();
        auto entryIndex = this->finalModule->functionTable.put_create(L"yoimiya_entry", entry);
        IRBuilder builder(compilerCtx, finalModule, entry);
        builder.switchCodeBlock(builder.createCodeBlock());
        for (auto &initIdx: globInitializerIndexes) {
            if (finalModule->functionTable[initIdx]->hasAttribute(IRFunctionDefinition::FunctionAttrs::Unreachable))
                continue;
            builder.invokeOp(initIdx, 0, compilerCtx->getIntObjectType());
        }
        if (compilerCtx->getBuildConfig()->buildType == IRBuildConfig::BuildType::executable) {
            try {
                builder.invokeOp(finalModule->functionTable.getIndex(L"yoi_main"), 0, compilerCtx->getIntObjectType());
                builder.retOp();
            } catch (const std::out_of_range&) {
                panic(0, 0, "Entry function not found for executable build!");
                return;
            }
        } else {
            builder.pushOp(IR::Opcode::push_integer, IROperand{IROperand::operandType::integer, IROperand::operandValue{(int64_t)0}});
            builder.retOp();
        }
        builder.yield();
    }
    void IRLinker::linkInterfaceImplementations() {
        for (auto &implPair : finalModule->interfaceImplementationTable) {
            implPair.second->implStructIndex = patchUniqueKey(implPair.second->implStructIndex);
            for (auto &virtualMethod : implPair.second->virtualMethods) {
                *virtualMethod = *patchType(virtualMethod);
            }
        }
    }
    void IRLinker::patchIRFFITable() {
        for (auto &funcPair : compilerCtx->getIRFFITable()->exportedFunctionTable) {
            funcPair.second = {ENTRY_MODULE_ID_CONST, functionRemapping.at(std::get<0>(funcPair.second)).at(std::get<1>(funcPair.second)), std::get<2>(funcPair.second)};
        }
        for (auto &libPair : compilerCtx->getIRFFITable()->importedLibraries) {
            for (auto &funcPair : libPair.second.importedFunctionTable) {
                funcPair.second->returnType = patchType(funcPair.second->returnType);
                for (auto &param : funcPair.second->argumentTypes) {
                    *param = *patchType(param);
                }
                for (auto &ret : funcPair.second->variableTable.getVariables()) {
                    *ret = *patchType(ret);
                }
            }
        }
        for (auto &foreignTypePair : compilerCtx->getIRFFITable()->foreignTypeTable) {
            foreignTypePair.second = patchType(foreignTypePair.second);
        }
    }

    std::tuple<IRValueType::valueType, indexT, indexT>
    IRLinker::patchUniqueKey(const std::tuple<IRValueType::valueType, indexT, indexT> &key) {
        switch (std::get<0>(key)) {
            case IRValueType::valueType::structObject: {
                auto newIndex = structRemapping[std::get<1>(key)][std::get<2>(key)];
                return std::make_tuple(IRValueType::valueType::structObject, ENTRY_MODULE_ID_CONST, newIndex);
            }
            case IRValueType::valueType::interfaceObject: {
                auto newIndex = interfaceRemapping[std::get<1>(key)][std::get<2>(key)];
                return std::make_tuple(IRValueType::valueType::interfaceObject, ENTRY_MODULE_ID_CONST, newIndex);
            }
            case IRValueType::valueType::virtualMethod: {
                auto newIndex = functionRemapping[std::get<1>(key)][std::get<2>(key)];
                return std::make_tuple(IRValueType::valueType::virtualMethod, ENTRY_MODULE_ID_CONST, newIndex);
            }
            default: {
                // Other types don't need patching
                return key;
            }
        }
    }
} // namespace yoi