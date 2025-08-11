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
        /*for (auto& operand : newInstr.operands) {
            indexT symbolModuleId = currentModuleId;
            indexT symbolIndex = operand.value.symbolIndex;
            bool isExtern = false;

            // Resolve externs first
            if (instr.opcode == IR::Opcode::invoke ||
                instr.opcode == IR::Opcode::load ||
                instr.opcode == IR::Opcode::store_extern ||
                instr.opcode == IR::Opcode::new_struct_extern ||
                instr.opcode == IR::Opcode::new_interface_extern ||
                instr.opcode == IR::Opcode::construct_interface_impl_extern) {
                isExtern = true;
                const auto& externEntry = compilerCtx->getImportedModule(currentModuleId)->externTable[symbolIndex];
                symbolModuleId = externEntry->affiliateModule;
                symbolIndex = externEntry->itemIndex;
            }

            switch (operand.type) {
                case IROperand::operandType::globalVar:
                    operand.value.symbolIndex = globalRemapping.at(symbolModuleId).at(symbolIndex);
                    break;
                case IROperand::operandType::stringLiteral:
                    operand.value.stringLiteralIndex = stringRemapping.at(symbolModuleId).at(symbolIndex);
                    break;
                case IROperand::operandType::index:
                    switch (instr.opcode) {
                        case IR::Opcode::invoke: case IR::Opcode::invoke_extern:
                            operand.value.symbolIndex = functionRemapping.at(symbolModuleId).at(symbolIndex);
                            break;
                        case IR::Opcode::new_struct: case IR::Opcode::new_struct_extern:
                            operand.value.symbolIndex = structRemapping.at(symbolModuleId).at(symbolIndex);
                            break;
                        case IR::Opcode::new_interface: case IR::Opcode::new_interface_extern:
                            operand.value.symbolIndex = interfaceRemapping.at(symbolModuleId).at(symbolIndex);
                            break;
                        case IR::Opcode::construct_interface_impl: case IR::Opcode::construct_interface_impl_extern:
                            operand.value.symbolIndex = interfaceImplRemapping.at(symbolModuleId).at(symbolIndex);
                            break;
                        default: break; // Other indices might not need patching (e.g., member index)
                    }
                    break;
                default:
                    break; // Local vars, literals, etc., don't need patching
            }
        }*/

        switch (instr.opcode) {
            case IR::Opcode::invoke:
            case IR::Opcode::invoke_virtual:
            case IR::Opcode::load_global:
            case IR::Opcode::new_struct:
            case IR::Opcode::new_interface:
            case IR::Opcode::construct_interface_impl: {
                auto moduleId = instr.operands[0].value.symbolIndex;
                auto symbolIndex = instr.operands[1].value.symbolIndex;
                newInstr.operands[0].value.symbolIndex = ENTRY_MODULE_ID_CONST;
                switch (instr.opcode) {
                    case IR::Opcode::invoke:
                    case IR::Opcode::invoke_virtual:
                        newInstr.operands[1].value.symbolIndex = functionRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::load_global:
                        newInstr.operands[1].value.symbolIndex = globalRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::new_struct:
                        newInstr.operands[1].value.symbolIndex = structRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::new_interface:
                        newInstr.operands[1].value.symbolIndex = interfaceRemapping.at(moduleId).at(symbolIndex);
                        break;
                    case IR::Opcode::construct_interface_impl:
                        newInstr.operands[1].value.symbolIndex = interfaceImplRemapping.at(moduleId).at(symbolIndex);
                        break;
                    default: break;
                }
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
            builder.invokeOp(initIdx, 0, compilerCtx->getIntObjectType());
        }
        if (compilerCtx->getBuildConfig()->buildType == IRBuildConfig::BuildType::executable) {
            builder.invokeOp(finalModule->functionTable.getIndex(L"yoi_main"), 0, compilerCtx->getIntObjectType());
            builder.retOp();
        } else {
            builder.pushOp(IR::Opcode::push_integer, IROperand{IROperand::operandType::integer, IROperand::operandValue{(int64_t)0}});
            builder.retOp();
        }
        builder.yield();
    }
    void IRLinker::linkInterfaceImplementations() {
        for (auto &implPair : finalModule->interfaceImplementationTable) {
            for (auto &virtualMethod : implPair.second->virtualMethods) {
                *virtualMethod = *patchType(virtualMethod);
            }
        }
    }
    void IRLinker::patchIRFFITable() {
        for (auto &funcPair : compilerCtx->getIRFFITable()->exportedFunctionTable) {
            funcPair.second = {ENTRY_MODULE_ID_CONST, functionRemapping.at(funcPair.second.first).at(funcPair.second.second)};
        }
        for (auto &foreignTypePair : compilerCtx->getIRFFITable()->foreignTypeTable) {
            foreignTypePair.second = patchType(foreignTypePair.second);
        }
    }
} // namespace yoi