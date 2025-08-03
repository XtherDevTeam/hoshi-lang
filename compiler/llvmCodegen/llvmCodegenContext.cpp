//
// Created by XIaokang00010 on 2024/10/9.
//

#include "llvmCodegenContext.hpp"
#include "compiler/ir/IR.h"
#include "share/def.hpp"
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <string>
#include <tuple>

namespace yoi {

    LLVMCodegen::LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, std::shared_ptr<IRModule> yoiModule)
        : TheContext(std::make_unique<llvm::LLVMContext>()),
        Builder(std::make_unique<llvm::IRBuilder<>>(*TheContext)),
        compilerCtx(std::move(compilerCtx)),
        yoiModule(std::move(yoiModule)) {
        TheModule = std::make_unique<llvm::Module>("yoi.module", *TheContext);
    }

    void LLVMCodegen::declareRuntimeFunctions() {
        // void* runtime_object_alloc(unsigned long long sizeOfObject) -> i8* (i64)
        llvm::Type* i8PtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
        llvm::Type* sizeTy = Builder->getInt64Ty();
        llvm::FunctionType* allocType = llvm::FunctionType::get(i8PtrTy, {sizeTy}, false);
        runtimeObjectAllocFunc = llvm::Function::Create(allocType, llvm::Function::ExternalLinkage, "runtime_object_alloc", TheModule.get());

        // void runtime_finalize_object(void* objectPtr) -> void (i8*)
        llvm::FunctionType* finalizeType = llvm::FunctionType::get(Builder->getVoidTy(), {i8PtrTy}, false);
        runtimeFinalizeObjectFunc = llvm::Function::Create(finalizeType, llvm::Function::ExternalLinkage, "runtime_finalize_object", TheModule.get());

        // void runtime_debug_report_current_function(const char *function_name);
        llvm::Type* constCharPtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
        llvm::FunctionType* debugReportType = llvm::FunctionType::get(Builder->getVoidTy(), {constCharPtrTy}, false);
        runtimeDebugReportCurrentFunctionFunc = llvm::Function::Create(debugReportType, llvm::Function::ExternalLinkage, "runtime_debug_report_current_function", TheModule.get());
        runtimeDebugReportCurrentFunctionFunc->setCallingConv(llvm::CallingConv::C);


        // void runtime_debug_print(const char *message);
        llvm::FunctionType* debugPrintType = llvm::FunctionType::get(Builder->getVoidTy(), {constCharPtrTy}, false);
        runtimeDebugPrintFunc = llvm::Function::Create(debugPrintType, llvm::Function::ExternalLinkage, "runtime_debug_print", TheModule.get());
        runtimeDebugPrintFunc->setCallingConv(llvm::CallingConv::C);

        // void runtime_debug_print_address(void *address);
        llvm::FunctionType* debugPrintAddressType = llvm::FunctionType::get(Builder->getVoidTy(), {i8PtrTy}, false);
        runtimeDebugPrintAddressFunc = llvm::Function::Create(debugPrintAddressType, llvm::Function::ExternalLinkage, "runtime_debug_print_address", TheModule.get());
        runtimeDebugPrintAddressFunc->setCallingConv(llvm::CallingConv::C);
    }

    void LLVMCodegen::generate() {
        declareRuntimeFunctions();
        generateBasicTypesAndFunctions();
        generateDeclarations();
        generateImplementations();
        generateDescription();
    }

    llvm::Module* LLVMCodegen::getModule() {
        return TheModule.get();
    }

    void LLVMCodegen::generateBasicTypesAndFunctions() {
        // --- Declare Basic Object Struct Types ---
        std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Type*>> basicTypes = {
            {compilerCtx->getIntObjectType(), Builder->getInt64Ty()},
            {compilerCtx->getDeciObjectType(), Builder->getDoubleTy()},
            {compilerCtx->getBoolObjectType(), Builder->getInt1Ty()},
            {compilerCtx->getCharObjectType(), Builder->getInt8Ty()},
            {compilerCtx->getStrObjectType(), llvm::PointerType::get(Builder->getInt8Ty(), 0)} // Assuming string object holds a char*
        };

        for (const auto& pair : basicTypes) {
            auto yoiType = pair.first;
            auto rawType = pair.second;
            auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
            auto name = "yoi.basic." + wstring2string(yoiType->to_string());
            auto* structType = llvm::StructType::create(*TheContext, {Builder->getInt64Ty(), rawType}, name);
            structTypeMap[key] = structType;
        }

        // --- Handle 'none' type as a special singleton object ---
        auto noneYoiType = compilerCtx->getNoneObjectType();
        auto noneKey = std::make_tuple(noneYoiType->type, noneYoiType->typeAffiliateModule, noneYoiType->typeIndex);
        auto* noneStructType = llvm::StructType::create(*TheContext, {Builder->getInt64Ty()}, "yoi.basic.none");
        structTypeMap[noneKey] = noneStructType;
        
        // Create the global singleton instance for noneObject
        auto* noneInitializer = llvm::ConstantStruct::get(noneStructType, {
            llvm::ConstantInt::get(Builder->getInt64Ty(), -1) // Special refcount, never collected
        });
        noneObjectSingleton = new llvm::GlobalVariable(
            *TheModule, 
            noneStructType, 
            true, // isConstant
            llvm::GlobalValue::InternalLinkage, 
            noneInitializer, 
            "YoiNoneObject"
        );

        // --- Generate GC Functions for 'none' type (no-ops) ---
        {
            auto* llvmStructPtrType = llvm::PointerType::get(noneStructType, 0);
            // Increase
            auto incFuncName = "basic_none_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::InternalLinkage, incFuncName, TheModule.get());
            functionMap[string2wstring(incFuncName)] = incFunction;
            auto* incBlock = llvm::BasicBlock::Create(*TheContext, "entry", incFunction);
            Builder->SetInsertPoint(incBlock);
            Builder->CreateRetVoid();
            // Decrease
            auto decFuncName = "basic_none_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::InternalLinkage, decFuncName, TheModule.get());
            functionMap[string2wstring(decFuncName)] = decFunction;
            auto* decBlock = llvm::BasicBlock::Create(*TheContext, "entry", decFunction);
            Builder->SetInsertPoint(decBlock);
            Builder->CreateRetVoid();
        }

        // --- Generate GC Functions for Other Basic Types ---
        for (const auto& pair : basicTypes) {
            auto yoiType = pair.first;
            auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
            auto* llvmStructType = structTypeMap.at(key);
            auto* llvmStructPtrType = llvm::PointerType::get(llvmStructType, 0);
            auto typeName = wstring2string(yoiType->to_string());

            // --- Generate gc_refcount_increase ---
            auto incFuncName = "basic_" + typeName + "_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::ExternalLinkage, incFuncName, TheModule.get());
            functionMap[string2wstring(incFuncName)] = incFunction;

            auto* incBlock = llvm::BasicBlock::Create(*TheContext, "entry", incFunction);
            Builder->SetInsertPoint(incBlock);
            llvm::Value* thisPtr = incFunction->arg_begin();

            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                std::string debugStr = "Increasing refcount of " + typeName + " object";
                auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, debugStr, true);
                auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalValue::PrivateLinkage, debugStrConst, "debug_str");
                auto* debugStrPtr = Builder->CreateBitCast(debugStrGlobal, llvm::PointerType::get(Builder->getInt8Ty(), 0));
                Builder->CreateCall(runtimeDebugPrintFunc, debugStrPtr);
                // address
                auto* castedPtr = Builder->CreateBitCast(thisPtr, llvm::PointerType::get(Builder->getInt8Ty(), 0));
                Builder->CreateCall(runtimeDebugPrintAddressFunc, castedPtr);
            }

            llvm::Value* incRefCountPtr = Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            llvm::Value* incOldRefCount = Builder->CreateLoad(Builder->getInt64Ty(), incRefCountPtr, "old_refcount");
            llvm::Value* incNewRefCount = Builder->CreateAdd(incOldRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "new_refcount");
            Builder->CreateStore(incNewRefCount, incRefCountPtr);
            Builder->CreateRetVoid();

            // --- Generate gc_refcount_decrease ---
            auto decFuncName = "basic_" + typeName + "_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::ExternalLinkage, decFuncName, TheModule.get());
            functionMap[string2wstring(decFuncName)] = decFunction;
            
            auto* entryBlock = llvm::BasicBlock::Create(*TheContext, "entry", decFunction);
            auto* returnEarlyBlock = llvm::BasicBlock::Create(*TheContext, "return_early", decFunction); // New block
            auto* continueDecrementBlock = llvm::BasicBlock::Create(*TheContext, "continue_decrement", decFunction); // New block
            auto* finalizeBlock = llvm::BasicBlock::Create(*TheContext, "finalize", decFunction);
            auto* continueBlock = llvm::BasicBlock::Create(*TheContext, "continue", decFunction);
            
            Builder->SetInsertPoint(entryBlock);
            thisPtr = decFunction->arg_begin();
            llvm::Value* isNull = Builder->CreateICmpEQ(thisPtr, llvm::ConstantPointerNull::get(llvmStructPtrType), "is_null");
            Builder->CreateCondBr(isNull, returnEarlyBlock, continueDecrementBlock); // Conditional branch

            Builder->SetInsertPoint(returnEarlyBlock);
            Builder->CreateRetVoid(); // Return early for null

            Builder->SetInsertPoint(continueDecrementBlock); // Continue with existing logic here
            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                std::string debugStr = "Decreasing refcount of " + typeName + " object";
                auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, debugStr, true);
                auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalValue::PrivateLinkage, debugStrConst, "debug_str");
                auto* debugStrPtr = Builder->CreateBitCast(debugStrGlobal, llvm::PointerType::get(Builder->getInt8Ty(), 0));
                Builder->CreateCall(runtimeDebugPrintFunc, debugStrPtr);
                // address
                auto* castedPtr = Builder->CreateBitCast(thisPtr, llvm::PointerType::get(Builder->getInt8Ty(), 0));
                Builder->CreateCall(runtimeDebugPrintAddressFunc, castedPtr);
            }
            llvm::Value* decRefCountPtr = Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            llvm::Value* decOldRefCount = Builder->CreateLoad(Builder->getInt64Ty(), decRefCountPtr, "old_refcount");
            llvm::Value* decNewRefCount = Builder->CreateSub(decOldRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "new_refcount");
            Builder->CreateStore(decNewRefCount, decRefCountPtr);

            llvm::Value* shouldFinalize = Builder->CreateICmpSLE(decNewRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 0), "should_finalize");
            Builder->CreateCondBr(shouldFinalize, finalizeBlock, continueBlock);

            Builder->SetInsertPoint(finalizeBlock);
            llvm::Value* castedPtr = Builder->CreateBitCast(thisPtr, llvm::PointerType::get(Builder->getInt8Ty(), 0));
            Builder->CreateCall(runtimeFinalizeObjectFunc, castedPtr);
            Builder->CreateBr(continueBlock);

            Builder->SetInsertPoint(continueBlock);
            Builder->CreateRetVoid();
        }
    }


    // --- DECLARATION PHASE ---

    void LLVMCodegen::generateDeclarations() {
        generateStructDeclarations();
        generateGlobalDeclarations();
        generateFunctionDeclarations();
    }

    void LLVMCodegen::generateStructDeclarations() {
        for (auto& structDefPair : yoiModule->structTable) {
            auto structDef = structDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, yoiModule->structTable.getIndex(structDef->name));
            auto structName = "struct." + std::to_string(yoiModule->identifier) + "." + wstring2string(structDef->name);
            structTypeMap[key] = llvm::StructType::create(*TheContext, structName);
        }
        for (auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, yoiModule->interfaceTable.getIndex(interfaceDef->name));
            auto interfaceName = "interface." + std::to_string(yoiModule->identifier) + "." + wstring2string(interfaceDef->name);
            structTypeMap[key] = llvm::StructType::create(*TheContext, interfaceName);
        }
    }

    void LLVMCodegen::generateGlobalDeclarations() {
        for (auto& globalPair : yoiModule->globalVariables) {
            auto globalName = wstring2string(globalPair.first);
            // All globals are pointers to objects.
            auto globalType = yoiTypeToLLVMType(globalPair.second);
            auto initializer = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(globalType));
            auto* globalVar = new llvm::GlobalVariable(*TheModule, globalType, false, llvm::GlobalValue::CommonLinkage, initializer, globalName);
            globalValues[yoiModule->globalVariables.getIndex(globalPair.first)] = globalVar;
        }
    }

    void LLVMCodegen::generateFunctionDeclarations() {
        for (auto& funcPair : yoiModule->functionTable) {
            auto funcDef = funcPair.second;
            auto funcName = wstring2string(funcDef->name);
            auto* funcType = getFunctionType(funcDef);
            auto* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, TheModule.get());
            functionMap[funcDef->name] = function;
        }
    }

    // --- IMPLEMENTATION PHASE ---

    void LLVMCodegen::generateImplementations() {
        generateStructImplementations();
        generateStructGCFunctions();
        generateInterfaceGCWrappers();
        generateFunctionImplementations();
    }

    void LLVMCodegen::generateStructImplementations() {
        for (auto& structDefPair : yoiModule->structTable) {
            auto structDef = structDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, yoiModule->structTable.getIndex(structDef->name));
            auto* llvmStructType = structTypeMap.at(key);

            std::vector<llvm::Type*> fieldTypes;
            // Per README, first field is gc_refcount
            fieldTypes.push_back(Builder->getInt64Ty());

            for (const auto& fieldType : structDef->fieldTypes) {
                fieldTypes.push_back(yoiTypeToLLVMType(fieldType));
            }
            if (llvmStructType->isOpaque()) {
                llvmStructType->setBody(fieldTypes);
            }
        }
        for (auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, yoiModule->interfaceTable.getIndex(interfaceDef->name));
            auto* llvmInterfaceType = structTypeMap.at(key);

            std::vector<llvm::Type*> memberTypes;
            // Layout as per README.md
            // 0. gc_refcount
            memberTypes.push_back(Builder->getInt64Ty());
            // 1. this pointer (to concrete struct)
            memberTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0));
            // 2. & 3. GC function pointers
            auto* gcFuncType = llvm::FunctionType::get(Builder->getVoidTy(), { llvm::PointerType::get(Builder->getInt8Ty(), 0) }, false);
            auto* gcFuncPtrType = llvm::PointerType::get(gcFuncType, 0);
            memberTypes.push_back(gcFuncPtrType); // gc_refcount_increase vptr
            memberTypes.push_back(gcFuncPtrType); // gc_refcount_decrease vptr

            // 4... Method pointers
            for (const auto& methodPair : interfaceDef->methodMap) {
                auto funcType = getFunctionType(methodPair.second);
                std::vector<llvm::Type*> virtualArgTypes;
                virtualArgTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0)); // 'this' is always i8*
                for(size_t i = 1; i < funcType->getNumParams(); ++i) {
                    virtualArgTypes.push_back(funcType->getParamType(i));
                }
                auto virtualFuncType = llvm::FunctionType::get(funcType->getReturnType(), virtualArgTypes, false);
                memberTypes.push_back(llvm::PointerType::get(virtualFuncType, 0));
            }

            if (llvmInterfaceType->isOpaque()) {
                llvmInterfaceType->setBody(memberTypes);
            }
        }
    }

    void LLVMCodegen::generateStructGCFunctions() {
        for (auto& structDefPair : yoiModule->structTable) {
            auto structDef = structDefPair.second;
            auto structIdx = yoiModule->structTable.getIndex(structDef->name);
            auto moduleID = yoiModule->identifier;
            auto key = std::make_tuple(IRValueType::valueType::structObject, moduleID, structIdx);
            auto* llvmStructType = structTypeMap.at(key);
            auto* llvmStructPtrType = llvm::PointerType::get(llvmStructType, 0);

            // --- Generate gc_refcount_increase ---
            auto incFuncName = "struct_" + std::to_string(moduleID) + "_" + std::to_string(structIdx) + "_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::InternalLinkage, incFuncName, TheModule.get());
            functionMap[string2wstring(incFuncName)] = incFunction;

            auto* incBlock = llvm::BasicBlock::Create(*TheContext, "entry", incFunction);
            Builder->SetInsertPoint(incBlock);
            llvm::Value* thisPtr = incFunction->arg_begin();
            llvm::Value* refCountPtr = Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            llvm::Value* oldRefCount = Builder->CreateLoad(Builder->getInt64Ty(), refCountPtr, "old_refcount");
            llvm::Value* newRefCount = Builder->CreateAdd(oldRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "new_refcount");
            Builder->CreateStore(newRefCount, refCountPtr);
            Builder->CreateRetVoid();

            // --- Generate gc_refcount_decrease ---
            auto decFuncName = "struct_" + std::to_string(moduleID) + "_" + std::to_string(structIdx) + "_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::InternalLinkage, decFuncName, TheModule.get());
            functionMap[string2wstring(decFuncName)] = decFunction;

            auto* entryBlock = llvm::BasicBlock::Create(*TheContext, "entry", decFunction);
            auto* returnEarlyBlock = llvm::BasicBlock::Create(*TheContext, "return_early", decFunction); // New block
            auto* continueDecrementBlock = llvm::BasicBlock::Create(*TheContext, "continue_decrement", decFunction); // New block
            auto* finalizeBlock = llvm::BasicBlock::Create(*TheContext, "finalize", decFunction);
            auto* continueBlock = llvm::BasicBlock::Create(*TheContext, "continue", decFunction);

            Builder->SetInsertPoint(entryBlock);
            thisPtr = decFunction->arg_begin();
            llvm::Value* isNull = Builder->CreateICmpEQ(thisPtr, llvm::ConstantPointerNull::get(llvmStructPtrType), "is_null");
            Builder->CreateCondBr(isNull, returnEarlyBlock, continueDecrementBlock); // Conditional branch

            Builder->SetInsertPoint(returnEarlyBlock);
            Builder->CreateRetVoid(); // Return early for null

            Builder->SetInsertPoint(continueDecrementBlock); // Continue with existing logic here
            refCountPtr = Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            oldRefCount = Builder->CreateLoad(Builder->getInt64Ty(), refCountPtr, "old_refcount");
            newRefCount = Builder->CreateSub(oldRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "new_refcount");
            Builder->CreateStore(newRefCount, refCountPtr);

            llvm::Value* shouldFinalize = Builder->CreateICmpSLE(newRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 0), "should_finalize");
            Builder->CreateCondBr(shouldFinalize, finalizeBlock, continueBlock);

            Builder->SetInsertPoint(finalizeBlock);
            llvm::Value* castedPtr = Builder->CreateBitCast(thisPtr, llvm::PointerType::get(Builder->getInt8Ty(), 0));
            Builder->CreateCall(runtimeFinalizeObjectFunc, castedPtr);
            Builder->CreateBr(continueBlock);

            Builder->SetInsertPoint(continueBlock);
            Builder->CreateRetVoid();
        }
    }

    void LLVMCodegen::generateInterfaceGCWrappers() {
        for (const auto& implPair : yoiModule->interfaceImplementationTable) {
            const auto& implDef = implPair.second;
            
            auto structModuleId = yoiModule->identifier;
            auto structKey = std::make_tuple(IRValueType::valueType::structObject, structModuleId, implDef->implStructIndex);
            auto* structType = structTypeMap.at(structKey);
            auto* structPtrType = llvm::PointerType::get(structType, 0);

            auto structIncName = "struct_" + std::to_string(structModuleId) + "_" + std::to_string(implDef->implStructIndex) + "_gc_refcount_increase";
            auto* structIncFunc = functionMap.at(string2wstring(structIncName));
            auto structDecName = "struct_" + std::to_string(structModuleId) + "_" + std::to_string(implDef->implStructIndex) + "_gc_refcount_decrease";
            auto* structDecFunc = functionMap.at(string2wstring(structDecName));

            auto wrapperBaseName = wstring2string(implDef->name);

            // --- Generate Increase Wrapper ---
            auto incWrapperName = wrapperBaseName + "_gc_refcount_increase";
            auto* wrapperFuncType = llvm::FunctionType::get(Builder->getVoidTy(), { llvm::PointerType::get(Builder->getInt8Ty(), 0) }, false);
            auto* incWrapperFunc = llvm::Function::Create(wrapperFuncType, llvm::Function::InternalLinkage, incWrapperName, TheModule.get());
            functionMap[string2wstring(incWrapperName)] = incWrapperFunc;
            
            auto* incEntryBlock = llvm::BasicBlock::Create(*TheContext, "entry", incWrapperFunc);
            // No specific null check needed here as increase on null is generally a no-op,
            // and a concrete increase function (struct_X_gc_refcount_increase) will be called,
            // which now also has null checks in the basic type definitions, or it's implicitly handled
            // if the underlying struct_X_gc_refcount_increase is guaranteed to not be called with null.
            // However, for consistency and robustness, it's safer to add it here.
            auto* incReturnEarlyBlock = llvm::BasicBlock::Create(*TheContext, "return_early", incWrapperFunc);
            auto* incContinueBlock = llvm::BasicBlock::Create(*TheContext, "continue_wrapper", incWrapperFunc);

            Builder->SetInsertPoint(incEntryBlock);
            llvm::Value* thisAsI8_inc = incWrapperFunc->arg_begin();
            llvm::Value* isNull_inc = Builder->CreateICmpEQ(thisAsI8_inc, llvm::ConstantPointerNull::get(llvm::PointerType::get(Builder->getInt8Ty(), 0)), "is_null");
            Builder->CreateCondBr(isNull_inc, incReturnEarlyBlock, incContinueBlock);

            Builder->SetInsertPoint(incReturnEarlyBlock);
            Builder->CreateRetVoid();

            Builder->SetInsertPoint(incContinueBlock);
            llvm::Value* castedThis_inc = Builder->CreateBitCast(thisAsI8_inc, structPtrType, "casted_this");
            Builder->CreateCall(structIncFunc, castedThis_inc);
            Builder->CreateRetVoid();


            // --- Generate Decrease Wrapper ---
            auto decWrapperName = wrapperBaseName + "_gc_refcount_decrease";
            auto* decWrapperFunc = llvm::Function::Create(wrapperFuncType, llvm::Function::InternalLinkage, decWrapperName, TheModule.get());
            functionMap[string2wstring(decWrapperName)] = decWrapperFunc;
            
            auto* decEntryBlock = llvm::BasicBlock::Create(*TheContext, "entry", decWrapperFunc);
            auto* decReturnEarlyBlock = llvm::BasicBlock::Create(*TheContext, "return_early", decWrapperFunc);
            auto* decContinueBlock = llvm::BasicBlock::Create(*TheContext, "continue_wrapper", decWrapperFunc);

            Builder->SetInsertPoint(decEntryBlock);
            llvm::Value* thisAsI8_dec = decWrapperFunc->arg_begin();
            llvm::Value* isNull_dec = Builder->CreateICmpEQ(thisAsI8_dec, llvm::ConstantPointerNull::get(llvm::PointerType::get(Builder->getInt8Ty(), 0)), "is_null");
            Builder->CreateCondBr(isNull_dec, decReturnEarlyBlock, decContinueBlock);

            Builder->SetInsertPoint(decReturnEarlyBlock);
            Builder->CreateRetVoid();

            Builder->SetInsertPoint(decContinueBlock);
            llvm::Value* castedThis_dec = Builder->CreateBitCast(thisAsI8_dec, structPtrType, "casted_this");
            Builder->CreateCall(structDecFunc, castedThis_dec);
            Builder->CreateRetVoid();
        }
    }


    void LLVMCodegen::generateFunctionImplementations() {
        for (auto& funcPair : yoiModule->functionTable) {
            if (!funcPair.second->codeBlock.empty()) {
                currentFunctionDef = funcPair.second;
                generateFunction(*funcPair.second);
            }
        }
    }

    void LLVMCodegen::generateFunction(IRFunctionDefinition& funcDef) {
        currentFunction = functionMap.at(funcDef.name);
        if (funcDef.codeBlock.empty()) return;

        blockMap.clear();
        for (yoi::indexT i = 0; i < funcDef.codeBlock.size(); ++i) {
            blockMap[i] = llvm::BasicBlock::Create(*TheContext, "block" + std::to_string(i), currentFunction);
        }

        auto* entryBlock = blockMap[0];
        Builder->SetInsertPoint(entryBlock);

        // invoke runtime_debug_report_current_function
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug){
            std::string funcName = wstring2string(funcDef.name);
            auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, funcName, true);
            auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
            auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
            Builder->CreateCall(runtimeDebugReportCurrentFunctionFunc, llvm::ArrayRef<llvm::Value*>(debugArgs));
        }


        namedValues.clear();
        auto& varTableRef = funcDef.getVariableTable();

        // Allocate space for all local variables (args + locals) and init to null
        const auto& vars = varTableRef.getVariables();
        const auto& names = varTableRef.getReversedVariableNameMap();
        for (yoi::indexT i = 0; i < vars.size(); ++i) {
            auto* llvmType = yoiTypeToLLVMType(vars[i]);
            auto* alloca = Builder->CreateAlloca(llvmType, nullptr, wstring2string(names.at(i)));
            Builder->CreateStore(llvm::Constant::getNullValue(llvmType), alloca);
            namedValues[i] = alloca;
        }

        // Store incoming arguments into their allocas, handling reference counts
        auto arg_it = currentFunction->arg_begin();
        for (yoi::indexT i = 0; i < funcDef.argumentTypes.size(); ++i, ++arg_it) {
            auto* alloca = namedValues.at(i);
            // Arguments are considered "retained" by the callee
            Builder->CreateStore(arg_it, alloca);
        }

        for (yoi::indexT i = 0; i < funcDef.codeBlock.size(); ++i) {
            generateCodeBlock(*funcDef.codeBlock[i], i);
        }

        if (llvm::verifyFunction(*currentFunction, &llvm::errs())) {
            TheModule->print(llvm::errs(), nullptr);
            panic(0, 0, "LLVM function verification failed for: " + wstring2string(funcDef.name));
        }
    }

    void LLVMCodegen::generateFunctionExitCleanup() {
        for (const auto& pair : namedValues) {
            auto varIndex = pair.first;
            auto* alloca = pair.second;
            auto varYoiType = currentFunctionDef->variableTable.get(varIndex);
            
            // Load the final pointer value from the local variable
            auto* objPtr = Builder->CreateLoad(alloca->getAllocatedType(), alloca, "cleanup_load");
            
            // Decrease its reference count
            callGcFunction(objPtr, varYoiType, false);
        }
    }

    void LLVMCodegen::generateCodeBlock(IRCodeBlock& block, yoi::indexT blockIdx) {
        Builder->SetInsertPoint(blockMap.at(blockIdx));
        if (Builder->GetInsertBlock()->getTerminator()) return;

        for (const auto& instr : block.getIRArray()) {
            generateInstruction(instr);
            if (Builder->GetInsertBlock()->getTerminator()) break;
        }
    }

    void LLVMCodegen::generateInstruction(const IR& instr) {
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            // insert call to runtime_debug_print extern func
            std::string debugStr = "Performing: " + yoi::wstring2string(instr.to_string());
            auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, debugStr, true);
            auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
            auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
            Builder->CreateCall(runtimeDebugPrintFunc, llvm::ArrayRef<llvm::Value*>(debugArgs));
        }
        switch(instr.opcode) {
            case IR::Opcode::push_integer: {
                auto val = llvm::ConstantInt::get(Builder->getInt64Ty(), instr.operands[0].value.integer, true);
                auto objPtr = createBasicObject(compilerCtx->getIntObjectType(), val);
                valueStack.push_back({objPtr, compilerCtx->getIntObjectType()});
                break;
            }
            case IR::Opcode::push_decimal: {
                auto val = llvm::ConstantFP::get(Builder->getDoubleTy(), instr.operands[0].value.decimal);
                auto objPtr = createBasicObject(compilerCtx->getDeciObjectType(), val);
                valueStack.push_back({objPtr, compilerCtx->getDeciObjectType()});
                break;
            }
            case IR::Opcode::push_boolean: {
                auto val = llvm::ConstantInt::get(Builder->getInt1Ty(), instr.operands[0].value.boolean);
                auto objPtr = createBasicObject(compilerCtx->getBoolObjectType(), val);
                valueStack.push_back({objPtr, compilerCtx->getBoolObjectType()});
                break;
            }
            case IR::Opcode::push_string: {
                auto& str = yoiModule->stringLiteralPool.getStringLiteral(instr.operands[0].value.stringLiteralIndex);
                auto* globalStr = Builder->CreateGlobalStringPtr(wstring2string(str));
                auto objPtr = createBasicObject(compilerCtx->getStrObjectType(), globalStr);
                valueStack.push_back({objPtr, compilerCtx->getStrObjectType()});
                break;
            }

            // Basic Type Casting
            case IR::Opcode::basic_cast_int: {
                auto val = valueStack.back(); valueStack.pop_back();
                llvm::Value* rawVal = unboxValue(val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isDoubleTy()) {
                    castedVal = Builder->CreateFPToSI(rawVal, Builder->getInt64Ty(), "deci_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = Builder->CreateZExt(rawVal, Builder->getInt64Ty(), "bool_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = Builder->CreateSExt(rawVal, Builder->getInt64Ty(), "char_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(64)) { // int (no-op)
                    castedVal = rawVal;
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_int");
                }
                
                auto* resultObj = createBasicObject(compilerCtx->getIntObjectType(), castedVal);
                valueStack.push_back({resultObj, compilerCtx->getIntObjectType()});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_deci: {
                auto val = valueStack.back(); valueStack.pop_back();
                llvm::Value* rawVal = unboxValue(val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isIntegerTy(64)) { // int
                    castedVal = Builder->CreateSIToFP(rawVal, Builder->getDoubleTy(), "int_to_deci_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = Builder->CreateUIToFP(rawVal, Builder->getDoubleTy(), "bool_to_deci_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = Builder->CreateSIToFP(rawVal, Builder->getDoubleTy(), "char_to_deci_cast");
                } else if (rawVal->getType()->isDoubleTy()) { // deci (no-op)
                    castedVal = rawVal;
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_deci");
                }
                
                auto* resultObj = createBasicObject(compilerCtx->getDeciObjectType(), castedVal);
                valueStack.push_back({resultObj, compilerCtx->getDeciObjectType()});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_bool: {
                auto val = valueStack.back(); valueStack.pop_back();
                llvm::Value* rawVal = unboxValue(val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isIntegerTy(64)) { // int
                    castedVal = Builder->CreateICmpNE(rawVal, llvm::ConstantInt::get(Builder->getInt64Ty(), 0), "int_to_bool_cast");
                } else if (rawVal->getType()->isDoubleTy()) { // deci
                    castedVal = Builder->CreateFCmpONE(rawVal, llvm::ConstantFP::get(Builder->getDoubleTy(), 0.0), "deci_to_bool_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = Builder->CreateICmpNE(rawVal, llvm::ConstantInt::get(Builder->getInt8Ty(), 0), "char_to_bool_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool (no-op)
                    castedVal = rawVal;
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_bool");
                }
                
                auto* resultObj = createBasicObject(compilerCtx->getBoolObjectType(), castedVal);
                valueStack.push_back({resultObj, compilerCtx->getBoolObjectType()});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }

            // Arithmetic
            case IR::Opcode::add: handleBinaryOp(llvm::Instruction::Add, false); break;
            case IR::Opcode::sub: handleBinaryOp(llvm::Instruction::Sub, false); break;
            case IR::Opcode::mul: handleBinaryOp(llvm::Instruction::Mul, false); break;
            case IR::Opcode::div: handleBinaryOp(llvm::Instruction::SDiv, false); break;
            case IR::Opcode::mod: handleBinaryOp(llvm::Instruction::SRem, false); break;

            // Unary
            case IR::Opcode::negate: {
                auto val = valueStack.back(); valueStack.pop_back();
                auto* rawVal = unboxValue(val.llvmValue, val.yoiType);
                auto* negatedRaw = Builder->CreateNeg(rawVal, "negtmp");
                auto* resultObj = createBasicObject(val.yoiType, negatedRaw);
                valueStack.push_back({resultObj, val.yoiType});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::bitwise_not: {
                auto val = valueStack.back(); valueStack.pop_back();
                auto* rawVal = unboxValue(val.llvmValue, val.yoiType);
                auto* notRaw = Builder->CreateNot(rawVal, "nottmp");
                auto* resultObj = createBasicObject(val.yoiType, notRaw);
                valueStack.push_back({resultObj, val.yoiType});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }

            // Comparison
            case IR::Opcode::equal: handleComparison(llvm::CmpInst::ICMP_EQ, false); break;
            case IR::Opcode::not_equal: handleComparison(llvm::CmpInst::ICMP_NE, false); break;
            case IR::Opcode::less_than: handleComparison(llvm::CmpInst::ICMP_SLT, false); break;
            case IR::Opcode::less_equal: handleComparison(llvm::CmpInst::ICMP_SLE, false); break;
            case IR::Opcode::greater_than: handleComparison(llvm::CmpInst::ICMP_SGT, false); break;
            case IR::Opcode::greater_equal: handleComparison(llvm::CmpInst::ICMP_SGE, false); break;

            // Memory
            case IR::Opcode::load_local: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* alloca = namedValues.at(varIndex);
                auto yoiType = currentFunctionDef->variableTable.get(varIndex);
                auto loadedPtr = Builder->CreateLoad(alloca->getAllocatedType(), alloca, "loadtmp");
                callGcFunction(loadedPtr, yoiType, true); // Loading creates a new reference
                valueStack.push_back({loadedPtr, yoiType});
                break;
            }
            case IR::Opcode::store_local: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* alloca = namedValues.at(varIndex);
                auto yoiType = currentFunctionDef->variableTable.get(varIndex);
                auto valToStore = valueStack.back(); valueStack.pop_back();
                
                // Retain new value
                callGcFunction(valToStore.llvmValue, valToStore.yoiType, true);
                // Release old value
                auto* oldPtr = Builder->CreateLoad(alloca->getAllocatedType(), alloca, "old_ptr_for_store");
                callGcFunction(oldPtr, yoiType, false);
                // Store new value
                Builder->CreateStore(valToStore.llvmValue, alloca);
                // The value from stack is now owned by the variable, so we release the stack's reference
                callGcFunction(valToStore.llvmValue, valToStore.yoiType, false);
                break;
            }
            case IR::Opcode::load_global: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* global = globalValues.at(varIndex);
                auto yoiType = yoiModule->globalVariables[varIndex];
                auto loadedPtr = Builder->CreateLoad(global->getValueType(), global, "loadglobaltmp");
                callGcFunction(loadedPtr, yoiType, true);
                valueStack.push_back({loadedPtr, yoiType});
                break;
            }
            case IR::Opcode::store_global: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* global = globalValues.at(varIndex);
                auto yoiType = yoiModule->globalVariables[varIndex];
                auto valToStore = valueStack.back(); valueStack.pop_back();

                callGcFunction(valToStore.llvmValue, valToStore.yoiType, true);
                auto* oldPtr = Builder->CreateLoad(global->getValueType(), global, "old_global_ptr");
                callGcFunction(oldPtr, yoiType, false);
                Builder->CreateStore(valToStore.llvmValue, global);
                callGcFunction(valToStore.llvmValue, valToStore.yoiType, false);
                break;
            }
            case IR::Opcode::load_member: {
                auto structVal = valueStack.back(); valueStack.pop_back();
                auto memberIndex = instr.operands[0].value.symbolIndex;
                auto llvmMemberIndex = memberIndex + 1; // +1 to skip gc_refcount header

                auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
                auto* llvmStructType = structTypeMap.at(key);
                auto* gep = Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, llvmMemberIndex, "memberptr");
                
                auto yoiStructDef = compilerCtx->getIRObjectFile()->compiledModule->structTable[std::get<2>(key)];
                auto memberYoiType = yoiStructDef->fieldTypes[memberIndex];
                llvm::Type* loadedType = yoiTypeToLLVMType(memberYoiType);
                auto* loadedMember = Builder->CreateLoad(loadedType, gep, "loadmember");
                
                callGcFunction(loadedMember, memberYoiType, true); // Create new reference for the loaded member
                valueStack.push_back({loadedMember, memberYoiType});
                
                callGcFunction(structVal.llvmValue, structVal.yoiType, false); // Consume the struct reference from the stack
                break;
            }
            case IR::Opcode::store_member: {
                auto structVal = valueStack.back(); valueStack.pop_back();
                auto valueToStore = valueStack.back(); valueStack.pop_back();
                
                auto memberIndex = instr.operands[0].value.symbolIndex;
                auto llvmMemberIndex = memberIndex + 1; // +1 to skip gc_refcount header

                auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
                auto* llvmStructType = structTypeMap.at(key);
                auto* gep = Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, llvmMemberIndex, "memberptr");

                auto yoiStructDef = compilerCtx->getIRObjectFile()->compiledModule->structTable[std::get<2>(key)];
                auto memberYoiType = yoiStructDef->fieldTypes[memberIndex];

                callGcFunction(valueToStore.llvmValue, valueToStore.yoiType, true);
                auto* oldMemberPtr = Builder->CreateLoad(yoiTypeToLLVMType(memberYoiType), gep, "old_member_ptr");
                callGcFunction(oldMemberPtr, memberYoiType, false);
                Builder->CreateStore(valueToStore.llvmValue, gep);

                callGcFunction(valueToStore.llvmValue, valueToStore.yoiType, false);
                callGcFunction(structVal.llvmValue, structVal.yoiType, false);
                break;
            }

            // Control Flow
            case IR::Opcode::jump: {
                Builder->CreateBr(blockMap.at(instr.operands[0].value.codeBlockIndex));
                break;
            }
            case IR::Opcode::jump_if_true:
            case IR::Opcode::jump_if_false: {
                auto condObj = valueStack.back(); valueStack.pop_back();
                auto* condRaw = unboxValue(condObj.llvmValue, condObj.yoiType);
                callGcFunction(condObj.llvmValue, condObj.yoiType, false);

                auto* destBlock = blockMap.at(instr.operands[0].value.codeBlockIndex);
                auto* nextBlock = llvm::BasicBlock::Create(*TheContext, "fallthrough", currentFunction);

                if (instr.opcode == IR::Opcode::jump_if_true) {
                    Builder->CreateCondBr(condRaw, destBlock, nextBlock);
                } else { // jump_if_false
                    Builder->CreateCondBr(condRaw, nextBlock, destBlock);
                }
                Builder->SetInsertPoint(nextBlock);
                break;
            }

            case IR::Opcode::ret: {
                generateFunctionExitCleanup();
                auto retVal = valueStack.back(); valueStack.pop_back();
                // The caller receives ownership, so we don't decrease the ref count here.
                Builder->CreateRet(retVal.llvmValue);
                break;
            }
            case IR::Opcode::ret_none: {
                generateFunctionExitCleanup();
                // Return the global singleton none object
                Builder->CreateRet(noneObjectSingleton);
                break;
            }

            // Functions
            case IR::Opcode::invoke: {
                auto funcIndex = instr.operands[0].value.symbolIndex;
                auto argCount = instr.operands[1].value.symbolIndex;

                auto funcDef = yoiModule->functionTable[funcIndex];
                auto* function = functionMap.at(funcDef->name);

                std::vector<llvm::Value*> args;
                for(size_t i = 0; i < argCount; ++i) {
                    auto arg = valueStack.back();
                    valueStack.pop_back();
                    args.push_back(arg.llvmValue);
                    // Callee will retain, so we release the stack's reference
                    // callGcFunction(arg.llvmValue, arg.yoiType, false);
                }
                std::reverse(args.begin(), args.end());

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    auto* call = Builder->CreateCall(function, args, "calltmp");
                    // The returned value is the singleton, but we still put it on the stack.
                    // It doesn't need a ref count increase.
                    valueStack.push_back({call, funcDef->returnType});
                } else {
                    auto* call = Builder->CreateCall(function, args, "calltmp");
                    // The returned value comes with a reference count for us to own.
                    valueStack.push_back({call, funcDef->returnType});
                }
                break;
            }
            case IR::Opcode::new_struct: {
                auto structIndex = instr.operands[0].value.symbolIndex;
                auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
                auto* structType = structTypeMap.at(key);

                auto size = TheModule->getDataLayout().getTypeAllocSize(structType);
                auto* sizeVal = llvm::ConstantInt::get(Builder->getInt64Ty(), size);

                auto* allocCall = Builder->CreateCall(runtimeObjectAllocFunc, sizeVal, "newtmp_alloc");
                auto* bitcast = Builder->CreateBitCast(allocCall, llvm::PointerType::get(structType, 0), "casttmp");
                
                auto* refCountPtr = Builder->CreateStructGEP(structType, bitcast, 0, "refcount_ptr");
                Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt64Ty(), 1), refCountPtr);
                
                auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
                valueStack.push_back({bitcast, yoiType});
                break;
            }
            case IR::Opcode::new_interface: {
                auto interfaceIndex = instr.operands[0].value.symbolIndex;
                auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, interfaceIndex);
                auto* interfaceLLVMType = structTypeMap.at(key);
                
                auto size = TheModule->getDataLayout().getTypeAllocSize(interfaceLLVMType);
                auto* sizeVal = llvm::ConstantInt::get(Builder->getInt64Ty(), size);

                auto* allocCall = Builder->CreateCall(runtimeObjectAllocFunc, sizeVal, "newinterface_alloc");
                auto* bitcast = Builder->CreateBitCast(allocCall, llvm::PointerType::get(interfaceLLVMType, 0), "casttmp");
                
                auto* refCountPtr = Builder->CreateStructGEP(interfaceLLVMType, bitcast, 0, "refcount_ptr");
                Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt64Ty(), 1), refCountPtr);

                auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::interfaceObject, yoiModule->identifier, interfaceIndex);
                valueStack.push_back({bitcast, yoiType});
                break;
            }
            case IR::Opcode::construct_interface_impl: {
                auto structInstanceVal = valueStack.back(); valueStack.pop_back();
                auto interfaceShellVal = valueStack.back(); valueStack.pop_back();
                
                auto interfaceImplIndex = instr.operands[0].value.symbolIndex;
                auto implDef = yoiModule->interfaceImplementationTable[interfaceImplIndex];

                auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject, interfaceShellVal.yoiType->typeAffiliateModule, interfaceShellVal.yoiType->typeIndex);
                auto* interfaceLLVMType = structTypeMap.at(interfaceKey);

                // Store `this` pointer at index 1
                auto* thisPtrField = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 1, "this_ptr_field");
                auto* castedStructPtr = Builder->CreateBitCast(structInstanceVal.llvmValue, llvm::PointerType::get(Builder->getInt8Ty(), 0), "casted_this");
                Builder->CreateStore(castedStructPtr, thisPtrField);
                // The interface now holds a reference to the struct.
                callGcFunction(structInstanceVal.llvmValue, structInstanceVal.yoiType, true);
                // We are done with the struct reference on the stack.
                callGcFunction(structInstanceVal.llvmValue, structInstanceVal.yoiType, false);

                // Populate GC function pointers at indices 2 and 3
                auto incWrapperName = wstring2string(implDef->name) + "_gc_refcount_increase";
                auto decWrapperName = wstring2string(implDef->name) + "_gc_refcount_decrease";
                auto* incWrapperFunc = functionMap.at(string2wstring(incWrapperName));
                auto* decWrapperFunc = functionMap.at(string2wstring(decWrapperName));

                auto* incVTableSlot = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 2, "gc_inc_slot");
                Builder->CreateStore(incWrapperFunc, incVTableSlot);
                auto* decVTableSlot = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 3, "gc_dec_slot");
                Builder->CreateStore(decWrapperFunc, decVTableSlot);

                // Populate user method pointers starting at index 4
                for (size_t i = 0; i < implDef->virtualMethods.size(); ++i) {
                    auto& methodYoiType = implDef->virtualMethods[i];
                    yoi_assert(methodYoiType->type == IRValueType::valueType::virtualMethod, 0, 0, "Expected virtual method type in impl definition");
                    auto funcIndex = methodYoiType->typeIndex;
                    auto funcDef = yoiModule->functionTable[funcIndex];
                    auto* llvmFunction = functionMap.at(funcDef->name);

                    auto* vtableSlotPtr = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, i + 4, "vtable_slot");
                    Builder->CreateStore(llvmFunction, vtableSlotPtr);
                }
                
                valueStack.push_back(interfaceShellVal); // Put the constructed interface back
                break;
            }
            case IR::Opcode::invoke_virtual: {
                auto methodVTableIndex = instr.operands[0].value.symbolIndex;
                auto userArgCount = instr.operands[1].value.symbolIndex;

                std::vector<StackValue> userArgs;
                for (size_t i = 0; i < userArgCount - 1; ++i) { // userArgCount includes 'this'
                    userArgs.push_back(valueStack.back());
                    valueStack.pop_back();
                }
                std::reverse(userArgs.begin(), userArgs.end());
                
                auto interfaceShellVal = valueStack.back();
                valueStack.pop_back();

                auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject, interfaceShellVal.yoiType->typeAffiliateModule, interfaceShellVal.yoiType->typeIndex);
                auto* interfaceLLVMType = structTypeMap.at(interfaceKey);

                // Load the concrete `this` pointer from index 1
                auto* thisPtrField = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 1, "this_ptr_field");
                auto* concreteThisPtrRaw = Builder->CreateLoad(llvm::PointerType::get(Builder->getInt8Ty(), 0), thisPtrField, "concrete_this_raw");

                // Load the function pointer to call from the v-table. User methods start at index 4.
                auto vtableSlotIndex = methodVTableIndex + 4;
                auto* vtableSlotPtr = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, vtableSlotIndex, "vtable_slot_ptr");
                
                auto interfaceDef = compilerCtx->getIRObjectFile()->compiledModule->interfaceTable[std::get<2>(interfaceKey)];
                auto methodDef = interfaceDef->methodMap[methodVTableIndex];
                auto* funcType = getFunctionType(methodDef);
                
                std::vector<llvm::Type*> virtualArgTypes;
                virtualArgTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0));
                for (size_t i = 1; i < funcType->getNumParams(); ++i) {
                    virtualArgTypes.push_back(funcType->getParamType(i));
                }
                auto* virtualFuncType = llvm::FunctionType::get(funcType->getReturnType(), virtualArgTypes, false);
                auto* virtualFuncPtrType = llvm::PointerType::get(virtualFuncType, 0);

                auto* funcPtrToCall = Builder->CreateLoad(virtualFuncPtrType, vtableSlotPtr, "func_ptr");

                std::vector<llvm::Value*> finalArgs;
                finalArgs.push_back(concreteThisPtrRaw);
                for(const auto& arg : userArgs) {
                    finalArgs.push_back(arg.llvmValue);
                    // callGcFunction(arg.llvmValue, arg.yoiType, false);
                }
                // callGcFunction(interfaceShellVal.llvmValue, interfaceShellVal.yoiType, false);

                llvm::CallInst* call = Builder->CreateCall(virtualFuncType, funcPtrToCall, finalArgs, "virtcall");

                valueStack.push_back({call, methodDef->returnType});
                break;
            }
            case IR::Opcode::nop:
                break;
                
            default:
                panic(0, 0, "LLVM Codegen: Unhandled yoi::IR opcode: " + std::string(magic_enum::enum_name(instr.opcode)));
        }
    }

    llvm::Type* LLVMCodegen::yoiTypeToLLVMType(const std::shared_ptr<IRValueType>& type) {
        // Per README, all objects are pointers on the stack.
        auto key = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);
        if (structTypeMap.count(key)) {
            return llvm::PointerType::get(structTypeMap.at(key), 0);
        }
        
        // Fallback for non-object types or errors
        switch (type->type) {
            case IRValueType::valueType::integerRaw:
                return Builder->getInt64Ty();
            case IRValueType::valueType::decimalRaw:
                return Builder->getDoubleTy();
            case IRValueType::valueType::booleanRaw:
                return Builder->getInt1Ty();
            case IRValueType::valueType::charRaw:
                return Builder->getInt8Ty();
            case IRValueType::valueType::pointerObject: // generic pointer
                return llvm::PointerType::get(Builder->getInt8Ty(), 0);
            default:
                panic(0, 0, "LLVM Codegen: Unhandled or unmapped yoi::IRValueType: " + std::string(magic_enum::enum_name(type->type)));
                return nullptr;
        }
    }

    llvm::FunctionType* LLVMCodegen::getFunctionType(const std::shared_ptr<IRFunctionDefinition>& funcDef) {
        auto* returnType = yoiTypeToLLVMType(funcDef->returnType);
        if (funcDef->returnType->type == IRValueType::valueType::none && returnType->isVoidTy()) {
            panic(0,0, "Function returning none should not map to void return type.");
        }

        std::vector<llvm::Type*> argTypes;
        for (const auto& argType : funcDef->argumentTypes) {
            argTypes.push_back(yoiTypeToLLVMType(argType));
        }
        return llvm::FunctionType::get(returnType, argTypes, false);
    }

    llvm::Constant* LLVMCodegen::getGlobalInitializer(const std::shared_ptr<IRValueType>& type) {
        auto* llvmType = yoiTypeToLLVMType(type);
        return llvm::Constant::getNullValue(llvmType);
    }

    void LLVMCodegen::handleBinaryOp(llvm::Instruction::BinaryOps op, bool isFloat) {
        auto R = valueStack.back(); valueStack.pop_back();
        auto L = valueStack.back(); valueStack.pop_back();

        llvm::Value* lValRaw = unboxValue(L.llvmValue, L.yoiType);
        llvm::Value* rValRaw = unboxValue(R.llvmValue, R.yoiType);

        bool typesAreFloats = lValRaw->getType()->isDoubleTy() || rValRaw->getType()->isDoubleTy();
        auto resultYoiType = typesAreFloats ? compilerCtx->getDeciObjectType() : compilerCtx->getIntObjectType();
        
        llvm::Value* resultRaw;

        if (typesAreFloats) {
            if (lValRaw->getType()->isIntegerTy()) lValRaw = Builder->CreateSIToFP(lValRaw, Builder->getDoubleTy(), "inttofp");
            if (rValRaw->getType()->isIntegerTy()) rValRaw = Builder->CreateSIToFP(rValRaw, Builder->getDoubleTy(), "inttofp");
            auto fop = op;
            switch(op) {
                case llvm::Instruction::Add: fop = llvm::Instruction::FAdd; break;
                case llvm::Instruction::Sub: fop = llvm::Instruction::FSub; break;
                case llvm::Instruction::Mul: fop = llvm::Instruction::FMul; break;
                case llvm::Instruction::SDiv: fop = llvm::Instruction::FDiv; break;
                case llvm::Instruction::SRem: fop = llvm::Instruction::FRem; break;
                default: panic(0,0, "Unsupported float binary op");
            }
            resultRaw = Builder->CreateBinOp(fop, lValRaw, rValRaw, "fbinop");
        } else {
            resultRaw = Builder->CreateBinOp(op, lValRaw, rValRaw, "ibinop");
        }
        
        auto* resultObj = createBasicObject(resultYoiType, resultRaw);
        valueStack.push_back({resultObj, resultYoiType});

        // Consume operands
        callGcFunction(L.llvmValue, L.yoiType, false);
        callGcFunction(R.llvmValue, R.yoiType, false);
    }

    void LLVMCodegen::handleComparison(llvm::CmpInst::Predicate pred, bool isFloat) {
        auto R = valueStack.back(); valueStack.pop_back();
        auto L = valueStack.back(); valueStack.pop_back();

        llvm::Value* lValRaw = unboxValue(L.llvmValue, L.yoiType);
        llvm::Value* rValRaw = unboxValue(R.llvmValue, R.yoiType);

        bool typesAreFloats = lValRaw->getType()->isDoubleTy() || rValRaw->getType()->isDoubleTy();
        
        llvm::Value* resultRaw;
        if (typesAreFloats) {
            if (lValRaw->getType()->isIntegerTy()) lValRaw = Builder->CreateSIToFP(lValRaw, Builder->getDoubleTy(), "inttofp");
            if (rValRaw->getType()->isIntegerTy()) rValRaw = Builder->CreateSIToFP(rValRaw, Builder->getDoubleTy(), "inttofp");

            auto fpred = llvm::CmpInst::FCMP_OEQ;
            switch(pred) {
                case llvm::CmpInst::ICMP_EQ:  fpred = llvm::CmpInst::FCMP_OEQ; break;
                case llvm::CmpInst::ICMP_NE:  fpred = llvm::CmpInst::FCMP_ONE; break;
                case llvm::CmpInst::ICMP_SLT: fpred = llvm::CmpInst::FCMP_OLT; break;
                case llvm::CmpInst::ICMP_SLE: fpred = llvm::CmpInst::FCMP_OLE; break;
                case llvm::CmpInst::ICMP_SGT: fpred = llvm::CmpInst::FCMP_OGT; break;
                case llvm::CmpInst::ICMP_SGE: fpred = llvm::CmpInst::FCMP_OGE; break;
                default: panic(0,0, "Unsupported float comparison op");
            }
            resultRaw = Builder->CreateFCmp(fpred, lValRaw, rValRaw, "fcmp");
        } else {
            resultRaw = Builder->CreateICmp(pred, lValRaw, rValRaw, "icmp");
        }

        auto* resultObj = createBasicObject(compilerCtx->getBoolObjectType(), resultRaw);
        valueStack.push_back({resultObj, compilerCtx->getBoolObjectType()});

        // Consume operands
        callGcFunction(L.llvmValue, L.yoiType, false);
        callGcFunction(R.llvmValue, R.yoiType, false);
    }

    llvm::Value* LLVMCodegen::createBasicObject(const std::shared_ptr<IRValueType>& yoiType, llvm::Value* rawValue) {
        auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
        auto* objType = structTypeMap.at(key);

        auto size = TheModule->getDataLayout().getTypeAllocSize(objType);
        auto* sizeVal = llvm::ConstantInt::get(Builder->getInt64Ty(), size);

        auto* allocCall = Builder->CreateCall(runtimeObjectAllocFunc, sizeVal, "new_obj_alloc");
        auto* newObjPtr = Builder->CreateBitCast(allocCall, llvm::PointerType::get(objType, 0), "new_obj_ptr");

        auto* refCountPtr = Builder->CreateStructGEP(objType, newObjPtr, 0, "refcount_ptr");
        Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt64Ty(), 1), refCountPtr);

        auto* valuePtr = Builder->CreateStructGEP(objType, newObjPtr, 1, "value_ptr");
        Builder->CreateStore(rawValue, valuePtr);
        
        return newObjPtr;
    }

    llvm::Value* LLVMCodegen::unboxValue(llvm::Value* objectPtr, const std::shared_ptr<IRValueType>& yoiType) {
        auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
        auto* objType = structTypeMap.at(key);
        auto* valuePtr = Builder->CreateStructGEP(objType, objectPtr, 1, "value_ptr");
        return Builder->CreateLoad(objType->getElementType(1), valuePtr, "unboxed_val");
    }

    void LLVMCodegen::callGcFunction(llvm::Value* objectPtr, const std::shared_ptr<IRValueType>& yoiType, bool isIncrease) {
        // The null check for runtime values is now handled within the generated GC functions themselves.
        // This 'isa<llvm::ConstantPointerNull>' check only catches compile-time null constants.
        if (llvm::isa<llvm::ConstantPointerNull>(objectPtr)) {
            return;
        }

        // No GC for the none object singleton
        if (yoiType->type == IRValueType::valueType::none) {
            return;
        }

        std::string funcNameBase;
        switch(yoiType->type) {
            case IRValueType::valueType::integerObject: funcNameBase = "basic_int"; break;
            case IRValueType::valueType::decimalObject: funcNameBase = "basic_decimal"; break;
            case IRValueType::valueType::booleanObject: funcNameBase = "basic_bool"; break;
            case IRValueType::valueType::stringObject: funcNameBase = "basic_string"; break;
            case IRValueType::valueType::characterObject: funcNameBase = "basic_char"; break;
            case IRValueType::valueType::structObject:
                funcNameBase = "struct_" + std::to_string(yoiType->typeAffiliateModule) + "_" + std::to_string(yoiType->typeIndex);
                break;
            case IRValueType::valueType::interfaceObject: {
                auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
                auto* interfaceType = structTypeMap.at(key);
                auto vtableSlotIndex = isIncrease ? 2 : 3;

                // --- runtime null check for interface objects ---
                llvm::Type* ptrType = llvm::PointerType::get(interfaceType, 0); 
                llvm::Value* isNull = Builder->CreateICmpEQ(objectPtr, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ptrType)), "is_null_interface_obj");

                llvm::BasicBlock* currentBlock = Builder->GetInsertBlock();
                llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(*TheContext, "continue_gc_interface", currentFunction);
                llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(*TheContext, "end_gc_interface", currentFunction);

                Builder->CreateCondBr(isNull, endBlock, continueBlock);
                Builder->SetInsertPoint(continueBlock);

                auto* i8PtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
                auto* gcFuncType = llvm::FunctionType::get(Builder->getVoidTy(), { i8PtrTy }, false);
                auto* gcFuncPtrType = llvm::PointerType::get(gcFuncType, 0);

                auto* vtableSlotPtr = Builder->CreateStructGEP(interfaceType, objectPtr, vtableSlotIndex, "gc_vslot_ptr");
                auto* funcPtr = Builder->CreateLoad(gcFuncPtrType, vtableSlotPtr, "gc_func_ptr");
                auto* thisPtr = Builder->CreateStructGEP(interfaceType, objectPtr, 1, "this_ptr_field");
                auto* concreteThis = Builder->CreateLoad(i8PtrTy, thisPtr, "concrete_this");

                Builder->CreateCall(gcFuncType, funcPtr, {concreteThis});
                Builder->CreateBr(endBlock);

                Builder->SetInsertPoint(endBlock);
                return;
            }
            default: return; // No GC needed for raw types or unknown types
        }

        auto funcName = funcNameBase + (isIncrease ? "_gc_refcount_increase" : "_gc_refcount_decrease");
        auto* gcFunc = functionMap.at(string2wstring(funcName));
        
        auto* ptrArg = Builder->CreateBitCast(objectPtr, gcFunc->getFunctionType()->getParamType(0));
        Builder->CreateCall(gcFunc, ptrArg);
    }

    void LLVMCodegen::generateDescription() {
        auto* descStr = llvm::ConstantDataArray::getString(*TheContext, 
            std::string("yoi-lang-") 
            + yoi::wstring2string(compilerCtx->getBuildConfig()->buildPlatform) 
            + "-" 
            + yoi::wstring2string(compilerCtx->getBuildConfig()->buildArch), 
            true);
        auto* descGlobal = new llvm::GlobalVariable(*TheModule, descStr->getType(), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, descStr, "yoi_desc");

        auto* buildTypeStr = llvm::ConstantDataArray::getIntegerValue(llvm::Type::getInt64Ty(*TheContext), llvm::APInt(64, static_cast<uint64_t>(compilerCtx->getBuildConfig()->buildType)));
        auto* buildTypeGlobal = new llvm::GlobalVariable(*TheModule, buildTypeStr->getType(), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, buildTypeStr, "yoi_build_type");
    }
    void LLVMCodegen::generateTargetObjectCode(const yoi::wstr &pathToOutput) {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        TheModule->setTargetTriple(TargetTriple);
        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
        if (!Target) {
            panic(0, 0, "Could not create target for " + TargetTriple + " (" + Error + ")");
        }

        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions Opt;
        auto RM = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
        llvm::CodeGenOptLevel OptLevel = compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::release ? llvm::CodeGenOptLevel::Aggressive : llvm::CodeGenOptLevel::Default;

        std::unique_ptr<llvm::TargetMachine> TM(
        Target->createTargetMachine(TargetTriple, CPU, Features, Opt, RM, std::optional<llvm::CodeModel::Model>(), OptLevel));

        if (!TM) {
            panic(0, 0, "Could not create TargetMachine for " + TargetTriple);
        }

        TheModule->setDataLayout(TM->createDataLayout());

        std::error_code EC;
        llvm::raw_fd_ostream Dest(yoi::wstring2string(pathToOutput), EC, llvm::sys::fs::OF_None);
        if (EC) {
            panic(0, 0, "Could not open file for writing: " + yoi::wstring2string(pathToOutput) + " (" + EC.message() + ")");
        }

        llvm::legacy::PassManager Pass;
        llvm::CodeGenFileType FileType = llvm::CodeGenFileType::ObjectFile; // To emit a .o file

        if (TM->addPassesToEmitFile(Pass, Dest, nullptr, FileType)) {
            panic(0, 0, "TargetMachine can't emit a file of this type");
        }

        Pass.run(*TheModule);
        Dest.flush();
    }
} // namespace yoi