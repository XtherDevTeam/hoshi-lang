//
// Created by XIaokang00010 on 2024/10/9.
//

#include "llvmCodegenContext.hpp"
#include "compiler/compilerContext.h"
#include "compiler/ir/IR.h"
#include "share/def.hpp"
#include <algorithm>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>
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
#include <memory>
#include <queue>
#include <string>
#include <tuple>

namespace yoi {

    LLVMCodegen::LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, std::shared_ptr<IRModule> yoiModule)
        : TheContext(std::make_unique<llvm::LLVMContext>()),
        Builder(std::make_unique<llvm::IRBuilder<>>(*TheContext)),
        compilerCtx(std::move(compilerCtx)),
        yoiModule(std::move(yoiModule)), 
        controlFlowAnalysis({}) {
        TheModule = std::make_unique<llvm::Module>("yoi.module", *TheContext);
    }

    void LLVMCodegen::declareRuntimeFunctions() {
        // void* runtime_object_alloc(unsigned long long sizeOfObject) -> i8* (i64)
        llvm::Type* i8PtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
        llvm::Type* sizeTy = Builder->getInt64Ty();

        llvm::FunctionType *mallocFuncType = llvm::FunctionType::get(llvm::PointerType::get(Builder->getInt8Ty(), 0), {sizeTy}, false);
        runtimeMalloc = llvm::Function::Create(mallocFuncType, llvm::Function::ExternalLinkage, "malloc", TheModule.get());
        runtimeMalloc->setCallingConv(llvm::CallingConv::C);


        llvm::FunctionType* allocType = llvm::FunctionType::get(i8PtrTy, {sizeTy, i8PtrTy}, false);
        llvm::FunctionType* funcType = llvm::FunctionType::get(i8PtrTy, {sizeTy}, false);
        runtimeObjectAllocReportFunc = llvm::Function::Create(allocType, llvm::Function::ExternalLinkage, "runtime_object_alloc_report", TheModule.get());
        runtimeObjectAllocFunc = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "object_alloc", TheModule.get());
        runtimeObjectAllocFunc->addFnAttr(llvm::Attribute::AlwaysInline);

        llvm::BasicBlock *entryOA = llvm::BasicBlock::Create(*TheContext, "entry", runtimeObjectAllocFunc);
        Builder->SetInsertPoint(entryOA);
        llvm::Value* sizeOfObject = runtimeObjectAllocFunc->arg_begin();
        llvm::Value *mem = Builder->CreateCall(runtimeMalloc, {sizeOfObject});
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            Builder->CreateCall(runtimeObjectAllocReportFunc, {sizeOfObject, mem});
        }
        Builder->CreateRet(mem);

        // void runtime_finalize_object(void* objectPtr) -> void (i8*)
        llvm::FunctionType* finalizeType = llvm::FunctionType::get(Builder->getVoidTy(), {i8PtrTy}, false);
        runtimeFinalizeObjectReportFunc = llvm::Function::Create(finalizeType, llvm::Function::ExternalLinkage, "runtime_finalize_object_report", TheModule.get());
        runtimeFinalizeObjectFunc = llvm::Function::Create(finalizeType, llvm::Function::InternalLinkage, "finalize_object", TheModule.get());

        runtimeFinalizeObjectFunc->addFnAttr(llvm::Attribute::AlwaysInline);

        llvm::BasicBlock *entryFO = llvm::BasicBlock::Create(*TheContext, "entry", runtimeFinalizeObjectFunc);
        Builder->SetInsertPoint(entryFO);
        llvm::Value* objectPtr = runtimeFinalizeObjectFunc->arg_begin();
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            Builder->CreateCall(runtimeFinalizeObjectReportFunc, {objectPtr});
        }
        Builder->CreateFree(objectPtr);
        Builder->CreateRetVoid();

        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
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

            // void runtime_debug_print_int(int value);
            llvm::FunctionType* debugPrintIntType = llvm::FunctionType::get(Builder->getVoidTy(), {Builder->getInt64Ty()}, false);
            runtimeDebugPrintIntFunc = llvm::Function::Create(debugPrintIntType, llvm::Function::ExternalLinkage, "runtime_debug_print_int", TheModule.get());
            runtimeDebugPrintIntFunc->setCallingConv(llvm::CallingConv::C); 

            // void runtime_debug_print_deci(double value);
            llvm::FunctionType* debugPrintDeciType = llvm::FunctionType::get(Builder->getVoidTy(), {Builder->getDoubleTy()}, false);
            runtimeDebugPrintDeciFunc = llvm::Function::Create(debugPrintDeciType, llvm::Function::ExternalLinkage, "runtime_debug_print_deci", TheModule.get());
            runtimeDebugPrintDeciFunc->setCallingConv(llvm::CallingConv::C);

            llvm::FunctionType *debugPrintCurrentAllocatedMemoryType = llvm::FunctionType::get(Builder->getVoidTy(), {}, false);
            runtimeDebugPrintCurrentAllocatedMemoryFunc = llvm::Function::Create(debugPrintCurrentAllocatedMemoryType, llvm::Function::ExternalLinkage, "runtime_debug_print_current_allocated_memory", TheModule.get());
            runtimeDebugPrintCurrentAllocatedMemoryFunc->setCallingConv(llvm::CallingConv::C);
        }
    }

    void LLVMCodegen::generate() {
        declareRuntimeFunctions();
        generateBasicTypesAndFunctions();
        generateDeclarations();
        generateForeignStructTypes();
        generateImportFunctionImplementations();
        generateImplementations();
        generateDescription();
        generateExportFunctionDecls();
        generateMainFunction();
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
            {compilerCtx->getStrObjectType(), llvm::PointerType::get(Builder->getInt8Ty(), 0)}
        };

        for (const auto& pair : basicTypes) {
            auto yoiType = pair.first;
            auto rawType = pair.second;
            auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
            auto name = "yoi.basic." + wstring2string(yoiType->to_string());
            auto* structType = llvm::StructType::create(*TheContext, {Builder->getInt64Ty(), rawType}, name);
            structTypeMap[key] = structType;

            foreignTypeMap[key] = rawType;
        }

        // --- Handle 'none' type as a special singleton object ---
        auto noneYoiType = compilerCtx->getNoneObjectType();
        auto noneKey = std::make_tuple(noneYoiType->type, noneYoiType->typeAffiliateModule, noneYoiType->typeIndex);
        auto* noneStructType = llvm::StructType::create(*TheContext, {Builder->getInt64Ty()}, "yoi.basic.none");
        structTypeMap[noneKey] = noneStructType;
        foreignTypeMap[noneKey] = llvm::Type::getVoidTy(*TheContext);
        
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
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
            functionMap[string2wstring(incFuncName)] = incFunction;
            auto* incBlock = llvm::BasicBlock::Create(*TheContext, "entry", incFunction);
            Builder->SetInsertPoint(incBlock);
            Builder->CreateRetVoid();
            // Decrease
            auto decFuncName = "basic_none_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::InternalLinkage, decFuncName, TheModule.get());
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
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
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::InternalLinkage, incFuncName, TheModule.get());
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
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
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::InternalLinkage, decFuncName, TheModule.get());
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
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
        generateImportFunctionDeclarations();
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
        generateInterfaceImplementationGCFunctions(); // Generates wrappers for specific interface implementations
        generateInterfaceObjectGCFunctions();       // NEW: Generates top-level wrappers for interface objects
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
            memberTypes.push_back(Builder->getInt64Ty());
            memberTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0));
            auto* gcFuncType = llvm::FunctionType::get(Builder->getVoidTy(), { llvm::PointerType::get(Builder->getInt8Ty(), 0) }, false);
            auto* gcFuncPtrType = llvm::PointerType::get(gcFuncType, 0);
            memberTypes.push_back(gcFuncPtrType); // gc_refcount_increase vptr
            memberTypes.push_back(gcFuncPtrType); // gc_refcount_decrease vptr

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
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
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
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
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
            // call dec for inner object (if any)
            for (yoi::indexT innerIdx = 0; innerIdx < structDef->fieldTypes.size(); ++innerIdx) {
                // create gep
                auto fieldPtr = Builder->CreateStructGEP(llvmStructType, thisPtr, innerIdx + 1, "field_ptr"); // skip refcount at index 0
                auto fieldType = structDef->fieldTypes[innerIdx];
                // Load the field value before calling its GC function
                llvm::Value* loadedField = Builder->CreateLoad(yoiTypeToLLVMType(fieldType), fieldPtr, "loaded_field_for_gc");
                callGcFunction(loadedField, fieldType, false); // Decrease refcount of member
            }
            Builder->CreateCall(runtimeFinalizeObjectFunc, castedPtr);
            Builder->CreateBr(continueBlock);

            Builder->SetInsertPoint(continueBlock);
            Builder->CreateRetVoid();
        }
    }

    void LLVMCodegen::generateInterfaceImplementationGCFunctions() {
        // These are the "interfaceImpl" wrappers, taking an i8* (the concrete object)
        // and calling the concrete struct's actual GC function.
        // These are placed into the interface object's GC function slots (indices 2 and 3).
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

            // Using implDef->name as part of the wrapper name for uniqueness
            auto wrapperBaseName = wstring2string(implDef->name);

            // --- Generate Increase Wrapper ---
            auto incWrapperName = wrapperBaseName + "_gc_refcount_increase";
            // Takes i8* as the concrete object pointer
            auto* wrapperFuncType = llvm::FunctionType::get(Builder->getVoidTy(), { llvm::PointerType::get(Builder->getInt8Ty(), 0) }, false);
            auto* incWrapperFunc = llvm::Function::Create(wrapperFuncType, llvm::Function::InternalLinkage, incWrapperName, TheModule.get());
            // incWrapperFunc->addFnAttr(llvm::Attribute::AlwaysInline); // no line for implementation functions
            functionMap[string2wstring(incWrapperName)] = incWrapperFunc;
            
            auto* incEntryBlock = llvm::BasicBlock::Create(*TheContext, "entry", incWrapperFunc);
            Builder->SetInsertPoint(incEntryBlock);
            llvm::Value* thisAsI8_inc = incWrapperFunc->arg_begin();
            // Cast i8* to the specific struct pointer type
            llvm::Value* castedThis_inc = Builder->CreateBitCast(thisAsI8_inc, structPtrType, "casted_this");
            // Call the concrete struct's increase function
            Builder->CreateCall(structIncFunc, castedThis_inc);
            Builder->CreateRetVoid();


            // --- Generate Decrease Wrapper ---
            auto decWrapperName = wrapperBaseName + "_gc_refcount_decrease";
            auto* decWrapperFunc = llvm::Function::Create(wrapperFuncType, llvm::Function::InternalLinkage, decWrapperName, TheModule.get());
            // decWrapperFunc->addFnAttr(llvm::Attribute::AlwaysInline);
            functionMap[string2wstring(decWrapperName)] = decWrapperFunc;
            
            auto* decEntryBlock = llvm::BasicBlock::Create(*TheContext, "entry", decWrapperFunc);
            Builder->SetInsertPoint(decEntryBlock);
            llvm::Value* thisAsI8_dec = decWrapperFunc->arg_begin();
            
            llvm::Value* castedThis_dec = Builder->CreateBitCast(thisAsI8_dec, structPtrType, "casted_this");
            Builder->CreateCall(structDecFunc, castedThis_dec);
            Builder->CreateRetVoid();
        }
    }

    void LLVMCodegen::generateInterfaceObjectGCFunctions() {
        // These are the top-level GC wrappers for the interface objects themselves.
        // They manage the interface object's own refcount and dispatch to the interfaceImpl wrappers.
        for (const auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            auto interfaceIdx = yoiModule->interfaceTable.getIndex(interfaceDef->name);
            auto moduleID = yoiModule->identifier;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, moduleID, interfaceIdx);
            auto* llvmInterfaceType = structTypeMap.at(key);
            auto* llvmInterfacePtrType = llvm::PointerType::get(llvmInterfaceType, 0);
            auto* i8PtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
            auto* gcFuncTypeForDispatch = llvm::FunctionType::get(Builder->getVoidTy(), { i8PtrTy }, false);
            auto* gcFuncPtrTypeForDispatch = llvm::PointerType::get(gcFuncTypeForDispatch, 0);


            // --- Generate interface_X_gc_refcount_increase ---
            auto incFuncName = "interface_" + std::to_string(moduleID) + "_" + std::to_string(interfaceIdx) + "_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmInterfacePtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::InternalLinkage, incFuncName, TheModule.get());
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
            functionMap[string2wstring(incFuncName)] = incFunction;
            

            auto* incEntryBlock = llvm::BasicBlock::Create(*TheContext, "entry", incFunction);
            auto* incReturnEarlyBlock = llvm::BasicBlock::Create(*TheContext, "return_early", incFunction);
            auto* incContinueBlock = llvm::BasicBlock::Create(*TheContext, "continue_wrapper", incFunction);

            Builder->SetInsertPoint(incEntryBlock);
            llvm::Value* thisPtr = incFunction->arg_begin();
            llvm::Value* isNull = Builder->CreateICmpEQ(thisPtr, llvm::ConstantPointerNull::get(llvmInterfacePtrType), "is_null");
            Builder->CreateCondBr(isNull, incReturnEarlyBlock, incContinueBlock);

            Builder->SetInsertPoint(incReturnEarlyBlock);
            Builder->CreateRetVoid();

            Builder->SetInsertPoint(incContinueBlock);
            
            llvm::Value* incRefCountPtr = Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 0, "refcount_ptr");
            llvm::Value* incOldRefCount = Builder->CreateLoad(Builder->getInt64Ty(), incRefCountPtr, "old_refcount");
            llvm::Value* incNewRefCount = Builder->CreateAdd(incOldRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "new_refcount");
            Builder->CreateStore(incNewRefCount, incRefCountPtr);

            llvm::Value* concreteThisPtr = Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 1, "this_ptr_field");
            llvm::Value* loadedConcreteThis = Builder->CreateLoad(i8PtrTy, concreteThisPtr, "concrete_this");

            llvm::Value* gcIncSlotPtr = Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 2, "gc_inc_slot");
            llvm::Value* gcIncFuncPtr = Builder->CreateLoad(gcFuncPtrTypeForDispatch, gcIncSlotPtr, "gc_func_ptr");
            Builder->CreateCall(gcFuncTypeForDispatch, gcIncFuncPtr, {loadedConcreteThis});

            Builder->CreateRetVoid();


            auto decFuncName = "interface_" + std::to_string(moduleID) + "_" + std::to_string(interfaceIdx) + "_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(Builder->getVoidTy(), {llvmInterfacePtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::InternalLinkage, decFuncName, TheModule.get());
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
            functionMap[string2wstring(decFuncName)] = decFunction;

            auto* decEntryBlock = llvm::BasicBlock::Create(*TheContext, "entry", decFunction);
            auto* decReturnEarlyBlock = llvm::BasicBlock::Create(*TheContext, "return_early", decFunction);
            auto* decContinueDecrementBlock = llvm::BasicBlock::Create(*TheContext, "continue_decrement", decFunction);
            auto* decFinalizeBlock = llvm::BasicBlock::Create(*TheContext, "finalize", decFunction);
            auto* decContinueBlock = llvm::BasicBlock::Create(*TheContext, "continue", decFunction);

            Builder->SetInsertPoint(decEntryBlock);
            thisPtr = decFunction->arg_begin();
            isNull = Builder->CreateICmpEQ(thisPtr, llvm::ConstantPointerNull::get(llvmInterfacePtrType), "is_null");
            Builder->CreateCondBr(isNull, decReturnEarlyBlock, decContinueDecrementBlock);

            Builder->SetInsertPoint(decReturnEarlyBlock);
            Builder->CreateRetVoid();

            Builder->SetInsertPoint(decContinueDecrementBlock);
            
            llvm::Value* decRefCountPtr = Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 0, "refcount_ptr");
            llvm::Value* decOldRefCount = Builder->CreateLoad(Builder->getInt64Ty(), decRefCountPtr, "old_refcount");
            llvm::Value* decNewRefCount = Builder->CreateSub(decOldRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "new_refcount");
            Builder->CreateStore(decNewRefCount, decRefCountPtr);

            llvm::Value* shouldFinalize = Builder->CreateICmpSLE(decNewRefCount, llvm::ConstantInt::get(Builder->getInt64Ty(), 0), "should_finalize");
            Builder->CreateCondBr(shouldFinalize, decFinalizeBlock, decContinueBlock);

            Builder->SetInsertPoint(decFinalizeBlock);

            concreteThisPtr = Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 1, "this_ptr_field");
            loadedConcreteThis = Builder->CreateLoad(i8PtrTy, concreteThisPtr, "concrete_this");

            llvm::Value* gcDecSlotPtr = Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 3, "gc_dec_slot");
            llvm::Value* gcDecFuncPtr = Builder->CreateLoad(gcFuncPtrTypeForDispatch, gcDecSlotPtr, "gc_func_ptr");
            Builder->CreateCall(gcFuncTypeForDispatch, gcDecFuncPtr, {loadedConcreteThis});

            llvm::Value* castedInterfacePtr = Builder->CreateBitCast(thisPtr, i8PtrTy);
            Builder->CreateCall(runtimeFinalizeObjectFunc, castedInterfacePtr);
            Builder->CreateBr(decContinueBlock);

            Builder->SetInsertPoint(decContinueBlock);
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

        controlFlowAnalysis = ControlFlowAnalysis{funcDef.codeBlock};
        valueStackMap.clear();
        basicBlockMap.clear();
        basicBlockVisited.clear();

        basicBlockMap[0][0] = llvm::BasicBlock::Create(*TheContext, "entry", currentFunction);
        auto* entryBlock = basicBlockMap[0][0];
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

        generateCodeBlock(*funcDef.codeBlock[0], 0, 0);

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

    void LLVMCodegen::generateCodeBlock(IRCodeBlock& block, yoi::indexT fromBlock, yoi::indexT toBlock) {
        // check whether generated
        if (basicBlockVisited[fromBlock].contains(toBlock) && toBlock != 0) {
            return;
        }
        basicBlockVisited[fromBlock][toBlock] = true;

        Builder->SetInsertPoint(basicBlockMap[fromBlock].at(toBlock));
        if (Builder->GetInsertBlock()->getTerminator()) return;

        for (const auto& succ : controlFlowAnalysis.G[toBlock]) {
            if (!basicBlockMap[toBlock].contains(succ)) {
                basicBlockMap[toBlock][succ] = llvm::BasicBlock::Create(*TheContext, "block_" + std::to_string(toBlock) + "_" + std::to_string(succ), currentFunction);
            }
        }

        for (const auto& instr : block.getIRArray()) {
            generateInstruction(instr, fromBlock, toBlock);
            if (Builder->GetInsertBlock()->getTerminator()) break;
        }

        for (const auto& succ : controlFlowAnalysis.G[toBlock]) {
            // prepare the value stack for the next block
            valueStackMap[toBlock][succ] = valueStackMap[fromBlock][toBlock];
            generateCodeBlock(*currentFunctionDef->codeBlock[succ], toBlock, succ);
        }
    }

    void LLVMCodegen::generateInstruction(const IR& instr, yoi::indexT fromBlock, yoi::indexT toBlock) {
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
                valueStackMap[fromBlock][toBlock].push_back({objPtr, compilerCtx->getIntObjectType()});
                break;
            }
            case IR::Opcode::push_decimal: {
                auto val = llvm::ConstantFP::get(Builder->getDoubleTy(), instr.operands[0].value.decimal);
                auto objPtr = createBasicObject(compilerCtx->getDeciObjectType(), val);
                valueStackMap[fromBlock][toBlock].push_back({objPtr, compilerCtx->getDeciObjectType()});
                break;
            }
            case IR::Opcode::push_boolean: {
                auto val = llvm::ConstantInt::get(Builder->getInt1Ty(), instr.operands[0].value.boolean);
                auto objPtr = createBasicObject(compilerCtx->getBoolObjectType(), val);
                valueStackMap[fromBlock][toBlock].push_back({objPtr, compilerCtx->getBoolObjectType()});
                break;
            }
            case IR::Opcode::push_string: {
                auto& str = yoiModule->stringLiteralPool.getStringLiteral(instr.operands[0].value.stringLiteralIndex);
                // Create a global string literal for this string
                auto *literal = llvm::ConstantDataArray::getString(*TheContext, yoi::wstring2string(str), true);
                auto *globalStr = Builder->CreateGlobalString(wstring2string(str), "global_string_literal");
                auto objPtr = createBasicObject(compilerCtx->getStrObjectType(), globalStr);
                valueStackMap[fromBlock][toBlock].push_back({objPtr, compilerCtx->getStrObjectType()});
                break;
            }

            // Basic Type Casting
            case IR::Opcode::basic_cast_int: {
                auto val = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
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
                valueStackMap[fromBlock][toBlock].push_back({resultObj, compilerCtx->getIntObjectType()});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_deci: {
                auto val = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
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
                valueStackMap[fromBlock][toBlock].push_back({resultObj, compilerCtx->getDeciObjectType()});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_bool: {
                auto val = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
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
                valueStackMap[fromBlock][toBlock].push_back({resultObj, compilerCtx->getBoolObjectType()});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }

            // Arithmetic
            case IR::Opcode::add: handleBinaryOp(llvm::Instruction::Add, false, fromBlock, toBlock); break;
            case IR::Opcode::sub: handleBinaryOp(llvm::Instruction::Sub, false, fromBlock, toBlock); break;
            case IR::Opcode::mul: handleBinaryOp(llvm::Instruction::Mul, false, fromBlock, toBlock); break;
            case IR::Opcode::div: handleBinaryOp(llvm::Instruction::SDiv, false, fromBlock, toBlock); break;
            case IR::Opcode::mod: handleBinaryOp(llvm::Instruction::SRem, false, fromBlock, toBlock); break;

            // Unary
            case IR::Opcode::negate: {
                auto val = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                auto* rawVal = unboxValue(val.llvmValue, val.yoiType);
                auto* negatedRaw = Builder->CreateNeg(rawVal, "negtmp");
                auto* resultObj = createBasicObject(val.yoiType, negatedRaw);
                valueStackMap[fromBlock][toBlock].push_back({resultObj, val.yoiType});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::bitwise_not: {
                auto val = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                auto* rawVal = unboxValue(val.llvmValue, val.yoiType);
                auto* notRaw = Builder->CreateNot(rawVal, "nottmp");
                auto* resultObj = createBasicObject(val.yoiType, notRaw);
                valueStackMap[fromBlock][toBlock].push_back({resultObj, val.yoiType});
                callGcFunction(val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }

            // Comparison
            case IR::Opcode::equal: handleComparison(llvm::CmpInst::ICMP_EQ, false, fromBlock, toBlock); break;
            case IR::Opcode::not_equal: handleComparison(llvm::CmpInst::ICMP_NE, false, fromBlock, toBlock); break;
            case IR::Opcode::less_than: handleComparison(llvm::CmpInst::ICMP_SLT, false, fromBlock, toBlock); break;
            case IR::Opcode::less_equal: handleComparison(llvm::CmpInst::ICMP_SLE, false, fromBlock, toBlock); break;
            case IR::Opcode::greater_than: handleComparison(llvm::CmpInst::ICMP_SGT, false, fromBlock, toBlock); break;
            case IR::Opcode::greater_equal: handleComparison(llvm::CmpInst::ICMP_SGE, false, fromBlock, toBlock); break;

            // Memory
            case IR::Opcode::load_local: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* alloca = namedValues.at(varIndex);
                auto yoiType = currentFunctionDef->variableTable.get(varIndex);
                auto loadedPtr = Builder->CreateLoad(alloca->getAllocatedType(), alloca, "loadtmp");
                callGcFunction(loadedPtr, yoiType, true); // Loading creates a new reference
                valueStackMap[fromBlock][toBlock].push_back({loadedPtr, yoiType});
                break;
            }
            case IR::Opcode::store_local: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* alloca = namedValues.at(varIndex);
                auto yoiType = currentFunctionDef->variableTable.get(varIndex);
                auto valToStore = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                
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
                valueStackMap[fromBlock][toBlock].push_back({loadedPtr, yoiType});
                break;
            }
            case IR::Opcode::store_global: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* global = globalValues.at(varIndex);
                auto yoiType = yoiModule->globalVariables[varIndex];
                auto valToStore = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();

                callGcFunction(valToStore.llvmValue, valToStore.yoiType, true);
                auto* oldPtr = Builder->CreateLoad(global->getValueType(), global, "old_global_ptr");
                callGcFunction(oldPtr, yoiType, false);
                Builder->CreateStore(valToStore.llvmValue, global);
                callGcFunction(valToStore.llvmValue, valToStore.yoiType, false);
                break;
            }
            case IR::Opcode::load_member: {
                auto structVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
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
                valueStackMap[fromBlock][toBlock].push_back({loadedMember, memberYoiType});
                
                callGcFunction(structVal.llvmValue, structVal.yoiType, false); // Consume the struct reference from the stack
                break;
            }
            case IR::Opcode::store_member: {
                auto structVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                auto valueToStore = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                
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
                Builder->CreateBr(basicBlockMap[toBlock].at(instr.operands[0].value.codeBlockIndex));
                break;
            }
            case IR::Opcode::jump_if_true:
            case IR::Opcode::jump_if_false: {
                auto condObj = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                auto* condRaw = unboxValue(condObj.llvmValue, condObj.yoiType);
                callGcFunction(condObj.llvmValue, condObj.yoiType, false);

                auto* destBlock = basicBlockMap[toBlock].at(instr.operands[0].value.codeBlockIndex);
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
                auto retVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
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
                auto moduleIndex = instr.operands[0].value.symbolIndex;
                auto funcIndex = instr.operands[1].value.symbolIndex;
                auto argCount = instr.operands[2].value.symbolIndex;

                auto funcDef = yoiModule->functionTable[funcIndex];
                auto* function = functionMap.at(funcDef->name);

                std::vector<llvm::Value*> args;
                for(size_t i = 0; i < argCount; ++i) {
                    auto arg = valueStackMap[fromBlock][toBlock].back();
                    valueStackMap[fromBlock][toBlock].pop_back();
                    args.push_back(arg.llvmValue);
                    // Callee will retain, so we release the stack's reference
                    // callGcFunction(arg.llvmValue, arg.yoiType, false);
                }
                std::reverse(args.begin(), args.end());

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    auto* call = Builder->CreateCall(function, args, "calltmp");
                    // The returned value is the singleton, but we still put it on the stack.
                    // It doesn't need a ref count increase.
                    valueStackMap[fromBlock][toBlock].push_back({call, funcDef->returnType});
                } else {
                    auto* call = Builder->CreateCall(function, args, "calltmp");
                    // The returned value comes with a reference count for us to own.
                    valueStackMap[fromBlock][toBlock].push_back({call, funcDef->returnType});
                }
                break;
            }
            case IR::Opcode::invoke_imported: {
                auto libIndex = instr.operands[0].value.symbolIndex;
                auto funcIndex = instr.operands[1].value.symbolIndex;
                auto argCount = instr.operands[2].value.symbolIndex;

                auto funcDef = compilerCtx->getIRFFITable()->importedLibraries[libIndex].importedFunctionTable[funcIndex];

                auto rawFuncName = compilerCtx->getIRFFITable()->importedLibraries[libIndex].importedFunctionTable.getKey(funcIndex);
                auto mangledFuncName = L"imported#" + std::to_wstring(libIndex) + L"#" + rawFuncName + L"#wrapper";

                auto* function = functionMap.at(mangledFuncName);

                std::vector<llvm::Value*> args;
                for(size_t i = 0; i < argCount; ++i) {
                    auto arg = valueStackMap[fromBlock][toBlock].back();
                    valueStackMap[fromBlock][toBlock].pop_back();
                    args.push_back(arg.llvmValue);
                    // Callee will retain, so we release the stack's reference
                    // callGcFunction(arg.llvmValue, arg.yoiType, false);
                }
                std::reverse(args.begin(), args.end());

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    auto* call = Builder->CreateCall(function, args, "calltmp");
                    // The returned value is the singleton, but we still put it on the stack.
                    // It doesn't need a ref count increase.
                    valueStackMap[fromBlock][toBlock].push_back({call, funcDef->returnType});
                } else {
                    auto* call = Builder->CreateCall(function, args, "calltmp");
                    // The returned value comes with a reference count for us to own.
                    valueStackMap[fromBlock][toBlock].push_back({call, funcDef->returnType});
                }
                break;
            }
            case IR::Opcode::new_struct: {
                auto moduleIndex = instr.operands[0].value.symbolIndex;
                auto structIndex = instr.operands[1].value.symbolIndex;
                auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
                auto* structType = structTypeMap.at(key);

                auto size = TheModule->getDataLayout().getTypeAllocSize(structType);
                auto* sizeVal = llvm::ConstantInt::get(Builder->getInt64Ty(), size);

                auto* allocCall = Builder->CreateCall(runtimeObjectAllocFunc, sizeVal, "newtmp_alloc");
                auto* bitcast = Builder->CreateBitCast(allocCall, llvm::PointerType::get(structType, 0), "casttmp");
                
                auto* refCountPtr = Builder->CreateStructGEP(structType, bitcast, 0, "refcount_ptr");
                Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt64Ty(), 1), refCountPtr);
                
                auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
                valueStackMap[fromBlock][toBlock].push_back({bitcast, yoiType});
                break;
            }
            case IR::Opcode::new_interface: {
                auto moduleIndex = instr.operands[0].value.symbolIndex;
                auto interfaceIndex = instr.operands[1].value.symbolIndex;
                auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, interfaceIndex);
                auto* interfaceLLVMType = structTypeMap.at(key);
                
                auto size = TheModule->getDataLayout().getTypeAllocSize(interfaceLLVMType);
                auto* sizeVal = llvm::ConstantInt::get(Builder->getInt64Ty(), size);

                auto* allocCall = Builder->CreateCall(runtimeObjectAllocFunc, sizeVal, "newinterface_alloc");
                auto* bitcast = Builder->CreateBitCast(allocCall, llvm::PointerType::get(interfaceLLVMType, 0), "casttmp");
                
                auto* refCountPtr = Builder->CreateStructGEP(interfaceLLVMType, bitcast, 0, "refcount_ptr");
                Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt64Ty(), 1), refCountPtr);

                auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::interfaceObject, yoiModule->identifier, interfaceIndex);
                valueStackMap[fromBlock][toBlock].push_back({bitcast, yoiType});
                break;
            }
            case IR::Opcode::construct_interface_impl: {
                auto interfaceShellVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                auto structInstanceVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                
                auto interfaceImplIndex = instr.operands[1].value.symbolIndex;
                auto implDef = yoiModule->interfaceImplementationTable[interfaceImplIndex];

                auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject, interfaceShellVal.yoiType->typeAffiliateModule, interfaceShellVal.yoiType->typeIndex);
                auto* interfaceLLVMType = structTypeMap.at(interfaceKey);

                // Store `this` pointer at index 1
                auto* thisPtrField = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 1, "this_ptr_field");
                auto* castedStructPtr = Builder->CreateBitCast(structInstanceVal.llvmValue, llvm::PointerType::get(Builder->getInt8Ty(), 0), "casted_this");
                Builder->CreateStore(castedStructPtr, thisPtrField);
                // The interface now holds a reference to the struct.
                // callGcFunction(structInstanceVal.llvmValue, structInstanceVal.yoiType, true); // This reference is handled by the interface object's own GC logic

                // Populate GC function pointers at indices 2 and 3 with pointers to the interfaceImpl wrappers
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
                
                valueStackMap[fromBlock][toBlock].push_back(interfaceShellVal); // Put the constructed interface back
                break;
            }
            case IR::Opcode::invoke_virtual: {
                auto methodVTableIndex = instr.operands[1].value.symbolIndex;
                auto userArgCount = instr.operands[2].value.symbolIndex;

                std::vector<StackValue> userArgs;
                for (size_t i = 0; i < userArgCount - 1; ++i) { // userArgCount includes 'this'
                    userArgs.push_back(valueStackMap[fromBlock][toBlock].back());
                    valueStackMap[fromBlock][toBlock].pop_back();
                }
                std::reverse(userArgs.begin(), userArgs.end());
                
                auto interfaceShellVal = valueStackMap[fromBlock][toBlock].back();
                valueStackMap[fromBlock][toBlock].pop_back();

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
                    // callGcFunction(arg.llvmValue, arg.yoiType, false); // Arguments are consumed by the call
                }
                callGcFunction(interfaceShellVal.llvmValue, interfaceShellVal.yoiType, false); // Interface object is consumed by the call

                llvm::CallInst* call = Builder->CreateCall(virtualFuncType, funcPtrToCall, finalArgs, "virtcall");

                valueStackMap[fromBlock][toBlock].push_back({call, methodDef->returnType});
                break;
            }
            case IR::Opcode::new_array_int: 
            case IR::Opcode::new_array_bool:
            case IR::Opcode::new_array_char:
            case IR::Opcode::new_array_deci:
            case IR::Opcode::new_array_str: {
                yoi::indexT size = 1;
                yoi::vec<llvm::Value *> dimensionsVal;
                yoi::vec<yoi::indexT> dimensions;
                
                std::shared_ptr<yoi::IRValueType> elementType;
                for (auto &i : instr.operands) {
                    size *= i.value.symbolIndex;
                    dimensions.push_back(i.value.symbolIndex);
                }
                for (yoi::indexT i = 0; i < size; ++i) {
                    dimensionsVal.push_back(valueStackMap[fromBlock][toBlock][valueStackMap[fromBlock][toBlock].size() - size + i].llvmValue);
                }
                
                switch (instr.opcode) {
                    case IR::Opcode::new_array_int:
                        elementType = compilerCtx->getIntObjectType();
                        break;
                    case IR::Opcode::new_array_bool:
                        elementType = compilerCtx->getBoolObjectType();
                        break;
                    case IR::Opcode::new_array_char:
                        elementType = compilerCtx->getCharObjectType();
                        break;
                    case IR::Opcode::new_array_deci:
                        elementType = compilerCtx->getDeciObjectType();
                        break;
                    case IR::Opcode::new_array_str:
                        elementType = compilerCtx->getStrObjectType();
                        break;
                    default:
                        break;
                }

                // Create the array object
                auto arrayType = managedPtr(elementType->getArrayType(dimensions));
                auto val = createArrayObject(arrayType, dimensionsVal);

                for (yoi::indexT i = 0; i < size; ++i) {
                    callGcFunction(valueStackMap[fromBlock][toBlock].back().llvmValue, valueStackMap[fromBlock][toBlock].back().yoiType, false);
                    valueStackMap[fromBlock][toBlock].pop_back();
                }

                valueStackMap[fromBlock][toBlock].push_back({val, arrayType});
                break;
            }
            case IR::Opcode::new_array_struct: 
            case IR::Opcode::new_array_interface: {
                yoi::indexT size = 1;
                yoi::vec<llvm::Value *> dimensionsVal;
                yoi::vec<yoi::indexT> dimensions;
                
                std::shared_ptr<yoi::IRValueType> elementType;
                for (yoi::indexT i = 2; i < instr.operands.size(); ++i) {
                    size *= instr.operands[i].value.symbolIndex;
                    dimensions.push_back(instr.operands[i].value.symbolIndex);
                }
                for (yoi::indexT i = 0; i < size; ++i) {
                    dimensionsVal.push_back(valueStackMap[fromBlock][toBlock][valueStackMap[fromBlock][toBlock].size() - size + i].llvmValue);
                }
                
                elementType = managedPtr(IRValueType{instr.opcode == IR::Opcode::new_array_struct ? IRValueType::valueType::structObject : IRValueType::valueType::interfaceObject, yoiModule->identifier, instr.operands[1].value.symbolIndex});

                // Create the array object
                auto arrayType = managedPtr(elementType->getArrayType(dimensions));
                auto val = createArrayObject(arrayType, dimensionsVal);

                valueStackMap[fromBlock][toBlock].push_back({val, arrayType});

                break;
            }
            case IR::Opcode::load_element: {
                auto indexVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                auto arrayVal = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();

                auto arrayType = arrayVal.yoiType;
                auto unboxedIndexVal = unboxValue(indexVal.llvmValue, indexVal.yoiType);
                auto result = loadArrayElement(arrayType, arrayVal.llvmValue, unboxedIndexVal);
                valueStackMap[fromBlock][toBlock].push_back({result, managedPtr(arrayType->getElementType())});
                // resource releasing
                callGcFunction(indexVal.llvmValue, indexVal.yoiType, false);
                callGcFunction(arrayVal.llvmValue, arrayVal.yoiType, false);
                break;
            }
            case IR::Opcode::pop: {
                auto val = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
                if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                    auto globalStrConst = llvm::ConstantDataArray::getString(*TheContext, "Popped value: " + val.llvmValue->getName().str(), true);
                    auto globalStr = new llvm::GlobalVariable(*TheModule, globalStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, globalStrConst, "popped_value_str");
                    Builder->CreateCall(runtimeDebugPrintFunc, {globalStr});
                }
                callGcFunction(val.llvmValue, val.yoiType, false);
                break;
            }
            case IR::Opcode::nop:
                break;
            default:
                panic(0, 0, "LLVM Codegen: Unhandled yoi::IR opcode: " + std::string(magic_enum::enum_name(instr.opcode)));
        }
    }

    llvm::Type* LLVMCodegen::yoiTypeToLLVMType(const std::shared_ptr<IRValueType>& type, bool enforceForeignType) {
        if (enforceForeignType) {
            if (type->isArrayType()) {
                // TODO
            }
            auto key = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);
            if (foreignTypeMap.count(key)) {
                return foreignTypeMap.at(key);
            }

            yoi_assert(type->isForeignBasicType(), 0, 0, "LLVM Codegen: Enforcing foreign type, but type is not a exported type or basic type.");
        } else {
            auto key = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);
            if (structTypeMap.count(key)) {
                if (type->isArrayType()) {
                    return llvm::PointerType::get(getArrayLLVMType(type), 0);
                } else {
                    return llvm::PointerType::get(structTypeMap.at(key), 0);
                }
            }
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
            case yoi::IRValueType::valueType::foreignFloatType:
                return Builder->getFloatTy();
            case IRValueType::valueType::foreignInt32Type:
                return Builder->getInt32Ty();
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

    void LLVMCodegen::handleBinaryOp(llvm::Instruction::BinaryOps op, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock) {
        auto R = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
        auto L = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();

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
        valueStackMap[fromBlock][toBlock].push_back({resultObj, resultYoiType});

        // Consume operands
        callGcFunction(L.llvmValue, L.yoiType, false);
        callGcFunction(R.llvmValue, R.yoiType, false);
    }

    void LLVMCodegen::handleComparison(llvm::CmpInst::Predicate pred, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock) {
        auto R = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();
        auto L = valueStackMap[fromBlock][toBlock].back(); valueStackMap[fromBlock][toBlock].pop_back();

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
        valueStackMap[fromBlock][toBlock].push_back({resultObj, compilerCtx->getBoolObjectType()});

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
        // No GC for the none object singleton
        if (yoiType->type == IRValueType::valueType::none) {
            return;
        }

        std::string funcNameBase;
        if (yoiType->isArrayType()) {
            yoi::indexT size = 1;
            for (auto &i : yoiType->dimensions) {
                size *= i;
            }

            funcNameBase = "array_" + yoi::wstring2string(yoiType->to_string()) + "_" + std::to_string(size);
        } else {
            switch(yoiType->type) {
                case IRValueType::valueType::foreignInt32Type:
                case IRValueType::valueType::integerObject: funcNameBase = "basic_int"; break;
                case IRValueType::valueType::foreignFloatType:
                case IRValueType::valueType::decimalObject: funcNameBase = "basic_decimal"; break;
                case IRValueType::valueType::booleanObject: funcNameBase = "basic_bool"; break;
                case IRValueType::valueType::stringObject: funcNameBase = "basic_string"; break;
                case IRValueType::valueType::characterObject: funcNameBase = "basic_char"; break;
                case IRValueType::valueType::structObject:
                    funcNameBase = "struct_" + std::to_string(yoiType->typeAffiliateModule) + "_" + std::to_string(yoiType->typeIndex);
                    break;
                case IRValueType::valueType::interfaceObject:
                    funcNameBase = "interface_" + std::to_string(yoiType->typeAffiliateModule) + "_" + std::to_string(yoiType->typeIndex);
                    break;
                default: return; // No GC needed for raw types or unhandled types
            }
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
        llvm::OptimizationLevel OptLevelPB = compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::release ? llvm::OptimizationLevel::O3 : llvm::OptimizationLevel::O0;

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

        llvm::PassBuilder PB;
        llvm::LoopAnalysisManager LAM;
        llvm::FunctionAnalysisManager FAM;
        llvm::CGSCCAnalysisManager CGAM;
        llvm::ModuleAnalysisManager MAM;

        // Register all the analysis passes with the managers.
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        // Create the optimization pipeline for the module
        llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptLevelPB);

        // Optional: Add a verifier pass to check IR correctness after optimizations
        // This is good for debugging but can be removed for release builds.
        MPM.addPass(llvm::VerifierPass());

        // Run the optimization pipeline on the module
        MPM.run(*TheModule, MAM);

        llvm::legacy::PassManager CodeGenPasses;
        llvm::CodeGenFileType FileType = llvm::CodeGenFileType::ObjectFile; // To emit a .o file

        if (TM->addPassesToEmitFile(CodeGenPasses, Dest, nullptr, FileType)) {
            panic(0, 0, "TargetMachine can't emit a file of this type");
        }

        CodeGenPasses.run(*TheModule);
        Dest.flush();
    }

    void LLVMCodegen::generateForeignStructTypes() {
        for (auto &foreignTypePair : compilerCtx->getIRFFITable()->foreignTypeTable) {
            auto &typeName = foreignTypePair.first;
            auto typeId = std::make_tuple(IRValueType::valueType::structObject, foreignTypePair.second->typeAffiliateModule, foreignTypePair.second->typeIndex);
            auto structType = yoiModule->structTable[foreignTypePair.second->typeIndex];
            yoi::vec<std::string> fieldNames;
            yoi::vec<llvm::Type*> fieldTypes;
            for (auto &name : structType->nameIndexMap) {
                if (name.second.type != IRStructDefinition::nameInfo::nameType::field) continue;

                auto fieldType = yoiTypeToLLVMType(structType->fieldTypes[name.second.index], true);
                fieldNames.push_back(yoi::wstring2string(name.first));
            }
            auto llvmStructType = llvm::StructType::create(*TheContext, fieldTypes);
            // add to foreign type map
            foreignTypeMap[typeId] = llvmStructType;
        }
    }

    void LLVMCodegen::generateExportFunctionDecls() {
        for (auto &exportedFunction : compilerCtx->getIRFFITable()->exportedFunctionTable) {
            auto &funcName = exportedFunction.first;
            auto &mangledName = yoiModule->functionTable.getKey(exportedFunction.second.second);
            auto &funcDecl = yoiModule->functionTable[exportedFunction.second.second];
            llvm::Type *returnType = yoiTypeToLLVMType(funcDecl->returnType, true);
            yoi::vec<llvm::Type*> argTypes;
            for (auto &argType : funcDecl->argumentTypes) {
                argTypes.push_back(yoiTypeToLLVMType(argType, true)); // make sure all types converted
            }
            llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
            llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, yoi::wstring2string(funcName), TheModule.get());
            functionMap[funcName] = func;

            // add basic block
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", func);
            Builder->SetInsertPoint(BB);

            // load arguments
            yoi::vec<llvm::Value*> args;
            auto it = func->arg_begin();
            for (auto &arg : funcDecl->argumentTypes) {
                if (arg->isBasicType()) {
                    auto *argVal = createBasicObject(arg, it);
                    args.push_back(argVal);
                } else {
                    auto handledLLVMType = handleForeignTypeConv(it, arg->typeIndex, false); // convert to yoi type
                    args.push_back(handledLLVMType);
                }
                it ++;
            }
            // call function
            auto *mangledFunction = functionMap.at(mangledName);
            auto *result = Builder->CreateCall(mangledFunction, args, "result");
            llvm::Value *actualResultVal = nullptr;
            // convert result to foreign type
            if (funcDecl->returnType->isBasicType()) {
                actualResultVal = unboxValue(result, funcDecl->returnType);
            } else {
                actualResultVal = handleForeignTypeConv(result, funcDecl->returnType->typeIndex, true); // convert back to foreign type
            }
            // resource releasing
            callGcFunction(result, funcDecl->returnType, false);
            for (auto &arg : funcDecl->argumentTypes) {
                callGcFunction(args.back(), arg, false);
                args.pop_back();
            }

            // return with actual result
            Builder->CreateRet(actualResultVal);
        }
    }

    llvm::Value *LLVMCodegen::handleForeignTypeConv(llvm::Value *val, yoi::indexT foreignTypeIndex, bool convertToForeign) {
        // get the foreign type
        auto &foreignType = compilerCtx->getIRFFITable()->foreignTypeTable[foreignTypeIndex];
        auto &originalType = yoiModule->structTable[foreignType->typeIndex];
        // get the llvm type
        auto llvmType = foreignTypeMap.at(std::make_tuple(IRValueType::valueType::structObject, foreignType->typeAffiliateModule, foreignType->typeIndex));
        auto objectLLVMType = structTypeMap.at(std::make_tuple(IRValueType::valueType::structObject, foreignType->typeAffiliateModule, foreignType->typeIndex));

        if (convertToForeign) {
            llvm::Value *rawMemory = Builder->CreateAlloca(llvmType, nullptr, "yoi_to_foreign_alloca");
            // convert yoi type to foreign type
            for (yoi::indexT i = 0; i < originalType->fieldTypes.size(); i++) {
                // get the field value
                auto *fieldPtr = Builder->CreateStructGEP(llvmType, val, i + 1, "field_ptr");
                auto &fieldType = originalType->fieldTypes[i];
                llvm::Value *fieldVal = nullptr;
                if (fieldType->isBasicType()) {
                    fieldVal = unboxValue(fieldPtr, fieldType);
                } else if (fieldType->isForeignBasicType()) {
                    fieldVal = handleForeignTypeConv(fieldPtr, fieldType, true);
                } else {
                    fieldVal = handleForeignTypeConv(fieldPtr, fieldType->typeIndex, true);
                }
                // count field size
                auto size = TheModule->getDataLayout().getTypeAllocSize(yoiTypeToLLVMType(fieldType, true));
                // populate memory
                Builder->CreateMemCpy(fieldPtr, llvm::MaybeAlign(8), fieldVal, llvm::MaybeAlign(8), size);
            }
            return rawMemory;
        } else {
            llvm::Value *rawMemory = Builder->CreateAlloca(objectLLVMType, nullptr, "foreign_to_yoi_alloca");
            // convert foreign type to yoi type
            for (yoi::indexT i = 0; i < originalType->fieldTypes.size(); i++) {
                // get the field value
                auto *fieldPtr = Builder->CreateStructGEP(llvmType, rawMemory, i + 1, "field_ptr");
                auto &fieldType = originalType->fieldTypes[i];
                llvm::Value *fieldVal = nullptr;
                if (fieldType->isBasicType()) {
                    auto *loadedFieldValue = Builder->CreateLoad(yoiTypeToLLVMType(fieldType, true), fieldPtr, "loaded_field_val");
                    fieldVal = createBasicObject(fieldType, fieldPtr);
                } else if (fieldType->isForeignBasicType()) {
                    fieldVal = handleForeignTypeConv(fieldPtr, fieldType, false);
                } else {
                    fieldVal = handleForeignTypeConv(fieldPtr, fieldType->typeIndex, false);
                }
                // populate memory using store
                Builder->CreateStore(fieldVal, fieldPtr);
            }
            return rawMemory;
        }
    }

    void LLVMCodegen::generateMainFunction() {
        if (compilerCtx->getBuildConfig()->buildType == IRBuildConfig::BuildType::executable) {
            yoi::vec<llvm::Type*> argTypes = {
                llvm::Type::getInt32Ty(*TheContext),
                llvm::PointerType::get(llvm::Type::getInt8Ty(*TheContext), 0)
            };
            llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), argTypes, false);
            llvm::Function *elysiaMain = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "elysia_main", TheModule.get());
            llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", TheModule.get());
            functionMap[L"main"] = mainFunc;

            // add basic block
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", mainFunc);
            Builder->SetInsertPoint(BB);

            auto it = mainFunc->arg_begin();
            auto argc = it++;
            auto argv = it++;

            // invoke elysia_main
            auto res = Builder->CreateCall(elysiaMain, {argc, argv}, "result");

            // return with result
            Builder->CreateRet(res);
        }
    }

    void LLVMCodegen::generateImportFunctionImplementations() {
        yoi::indexT moduleIndex = 0;
        for (auto &libraryPair : compilerCtx->getIRFFITable()->importedLibraries) {
            auto &libraryName = libraryPair.first;
            compilerCtx->getBuildConfig()->additionalLinkingFiles.push_back(libraryName);
            for (auto &functionPair : libraryPair.second.importedFunctionTable) {
                auto &funcName = functionPair.first;
                auto wrapperMangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName + L"#wrapper";
                auto mangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName;
                auto &funcDef = functionPair.second;
                auto &wrapperFuncDecl = functionMap[wrapperMangledName];
                auto &externFuncDecl = functionMap[mangledName];

                // generate wrapper function
                // create basic block
                llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", wrapperFuncDecl);
                Builder->SetInsertPoint(BB);

                if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                    // print function name
                    std::string funcName = wstring2string(funcDef->name);
                    auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, funcName, true);
                    auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                    auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                    Builder->CreateCall(runtimeDebugReportCurrentFunctionFunc, llvm::ArrayRef<llvm::Value*>(debugArgs));
                }

                yoi::vec<llvm::Value*> args;
                auto it = wrapperFuncDecl->arg_begin();
                for (auto &arg : funcDef->argumentTypes) {
                    if (arg->isBasicType()) {
                        auto *argVal = unboxValue(it, arg);
                        callGcFunction(it, arg, false);
                        args.push_back(argVal);
                    } else if (arg->isForeignBasicType()) {
                        auto *argVal = handleForeignTypeConv(it, arg, true);
                        callGcFunction(it, arg, false);
                        args.push_back(argVal);
                    } else {
                        auto handledLLVMType = handleForeignTypeConv(it, arg->typeIndex, true); 
                        callGcFunction(it, arg, false);
                        args.push_back(handledLLVMType);
                    }
                    it++;
                }

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    Builder->CreateCall(externFuncDecl, args);
                    Builder->CreateRet(noneObjectSingleton);
                } else {
                    auto result = Builder->CreateCall(externFuncDecl, args, "result");
                    llvm::Value *actualResultVal = nullptr;
                    if (funcDef->returnType->isBasicType()) {
                        actualResultVal = createBasicObject(funcDef->returnType, result);
                    } else if (funcDef->returnType->isForeignBasicType()) {
                        actualResultVal = handleForeignTypeConv(result, funcDef->returnType, false);
                    } else {
                        actualResultVal = handleForeignTypeConv(result, funcDef->returnType->typeIndex, false); //convert back to yoi type
                    }
                    
                    // return with actual result
                    Builder->CreateRet(actualResultVal);
                }
            }
            moduleIndex ++;
        }
    }
    
    void LLVMCodegen::generateImportFunctionDeclarations() {
        yoi::indexT moduleIndex = 0;
        for (auto &libraryPair : compilerCtx->getIRFFITable()->importedLibraries) {
            auto &libraryName = libraryPair.first;
            compilerCtx->getBuildConfig()->additionalLinkingFiles.push_back(libraryName);
            for (auto &functionPair : libraryPair.second.importedFunctionTable) {
                auto &funcName = functionPair.first;
                // generate extern function first
                llvm::Type *returnType = yoiTypeToLLVMType(functionPair.second->returnType, true);
                yoi::vec<llvm::Type*> argTypes;
                for (auto &argType : functionPair.second->argumentTypes) {
                    argTypes.push_back(yoiTypeToLLVMType(argType, true)); // make sure all types converted
                }
                llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
                llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, yoi::wstring2string(funcName), TheModule.get());

                // add to function map
                auto mangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName;
                functionMap[mangledName] = func;

                // then generate wrapper function decl
                llvm::Type *wrapperReturnType = yoiTypeToLLVMType(normalizeForeignType(functionPair.second->returnType), false);
                yoi::vec<llvm::Type*> wrapperArgTypes;
                for (auto &argType : functionPair.second->argumentTypes) {
                    wrapperArgTypes.push_back(yoiTypeToLLVMType(normalizeForeignType(argType), false)); // normalize foreign int32 type to integerObject
                }
                llvm::FunctionType *wrapperFuncType = llvm::FunctionType::get(wrapperReturnType, wrapperArgTypes, false);
                llvm::Function *wrapperFunc = llvm::Function::Create(wrapperFuncType, llvm::Function::ExternalLinkage, yoi::wstring2string(mangledName), TheModule.get());

                // add to function map
                auto wrapperMangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName + L"#wrapper";
                functionMap[wrapperMangledName] = wrapperFunc;
            }
            moduleIndex ++;
        }
    }

    const std::shared_ptr<IRValueType> &
    LLVMCodegen::normalizeForeignType(const std::shared_ptr<IRValueType> &type) {
        switch (type->type) {
            case IRValueType::valueType::foreignFloatType: {
                return compilerCtx->getDeciObjectType();
            }
            case IRValueType::valueType::foreignInt32Type: {
                return compilerCtx->getIntObjectType();
            }
            default: {
                return type;
            }
        }
    }

    llvm::Value *LLVMCodegen::handleForeignTypeConv(llvm::Value *val,
                                                    const std::shared_ptr<IRValueType> &foreignType,
                                                    bool convertToForeign) {
        yoi_assert(foreignType->isForeignBasicType(), 0, 0, "foreign type must be a basic type");
        switch (foreignType->type) {
            case IRValueType::valueType::foreignFloatType: {
                if (convertToForeign) {
                    // unbox double type and convert to float type
                    auto *doubleVal = unboxValue(val, compilerCtx->getDeciObjectType());
                    auto *floatVal = Builder->CreateFPTrunc(doubleVal, llvm::Type::getFloatTy(*TheContext), "float_val");
                    return floatVal;
                } else {
                    // convert float type to double type
                    auto *floatVal = Builder->CreateFPExt(val, llvm::Type::getDoubleTy(*TheContext), "float_val");
                    // create new object
                    auto *newObj = createBasicObject(compilerCtx->getDeciObjectType(), floatVal);
                    return newObj;
                }
            }
            case IRValueType::valueType::foreignInt32Type: {
                if (convertToForeign) {
                    // unbox integer type and convert to int32 type
                    auto *intVal = unboxValue(val, compilerCtx->getIntObjectType());
                    auto *int32Val = Builder->CreateTrunc(intVal, llvm::Type::getInt32Ty(*TheContext), "int32_val");
                    return int32Val;
                } else {
                    // convert int32 type to integer type
                    auto *int32Val = Builder->CreateSExt(val, llvm::Type::getInt64Ty(*TheContext), "int32_val");
                    // create new object
                    auto *newObj = createBasicObject(compilerCtx->getIntObjectType(), int32Val);
                    return newObj;
                }
            }
            default: {
                yoi_assert(false, 0, 0, "unsupported foreign type");
                return nullptr;
            }
        }
    }

    llvm::Type *LLVMCodegen::getArrayLLVMType(const std::shared_ptr<IRValueType> &type, bool enforceForeignType) {
        if (enforceForeignType) {
            // TODO
        } else {
            yoi::indexT size = 1;
            for (auto &i : type->dimensions) {
                size *= i;
            }
            
            std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT> arrayKey = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex, size);
            if (auto it = arrayTypeMap.find(arrayKey); it!= arrayTypeMap.end()) {
                return it->second;
            }

            std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> structKey = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);

            llvm::Type *baseType = nullptr;
            switch (type->type) {
                case IRValueType::valueType::integerObject: 
                    baseType = llvm::Type::getInt64Ty(*TheContext);
                    break;
                case IRValueType::valueType::decimalObject:
                    baseType = llvm::Type::getDoubleTy(*TheContext);
                    break;
                case IRValueType::valueType::booleanObject:
                    baseType = llvm::Type::getInt1Ty(*TheContext);
                    break;
                case IRValueType::valueType::characterObject:
                    baseType = llvm::Type::getInt8Ty(*TheContext);
                    break;
                case IRValueType::valueType::stringObject:
                    baseType = llvm::PointerType::get(llvm::Type::getInt8Ty(*TheContext), 0);
                    break;
                case IRValueType::valueType::structObject:
                case IRValueType::valueType::interfaceObject:
                    baseType = llvm::PointerType::get(structTypeMap.at(structKey), 0); // only this is a object
                    break;
                default:
                    panic(0, 0, "LLVM Codegen: Unhandled or unmapped array type: " + std::string(magic_enum::enum_name(type->type)));
                    return nullptr;
            }
            auto arrayType = llvm::ArrayType::get(baseType, size);
            // build struct with ref counter
            auto structType = llvm::StructType::create(*TheContext, yoi::vec<llvm::Type*>{
                llvm::Type::getInt64Ty(*TheContext), // ref counter
                arrayType // array
            });
            // create gc function
            auto fullStructName = "array_" + yoi::wstring2string(type->to_string()) + "_" + std::to_string(size);
            auto incFuncName = fullStructName + "_gc_refcount_increase";
            auto decFuncName = fullStructName + "_gc_refcount_decrease";

            auto gcIncFuncType = llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), {llvm::PointerType::get(structType, 0)}, false);
            auto gcIncFunc = llvm::Function::Create(gcIncFuncType, llvm::Function::ExternalLinkage, incFuncName, TheModule.get());
            gcIncFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
            auto currentInsertPoint = Builder->GetInsertBlock();
            // add basic block
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", gcIncFunc);
            Builder->SetInsertPoint(BB);
            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                // print function name
                auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, incFuncName, true);
                auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                Builder->CreateCall(runtimeDebugReportCurrentFunctionFunc, llvm::ArrayRef<llvm::Value*>(debugArgs));
            }
            auto *objPtr = gcIncFunc->arg_begin();
            auto *refCounter = Builder->CreateStructGEP(structType, objPtr, 0, "ref_counter");
            auto *newRefCounter = Builder->CreateLoad(llvm::Type::getInt64Ty(*TheContext), refCounter, "new_ref_counter");
            auto *newRefCounterVal = Builder->CreateAdd(newRefCounter, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 1, true), "new_ref_counter_val");
            Builder->CreateStore(newRefCounterVal, refCounter);
            Builder->CreateRetVoid();

            auto gcDecFuncType = llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), {llvm::PointerType::get(structType, 0)}, false);
            auto gcDecFunc = llvm::Function::Create(gcDecFuncType, llvm::Function::ExternalLinkage, decFuncName, TheModule.get());
            gcDecFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
            // add basic block
            BB = llvm::BasicBlock::Create(*TheContext, "entry", gcDecFunc);
            auto nullFailedBlock = llvm::BasicBlock::Create(*TheContext, "null_failed", gcDecFunc);
            auto continueBlock = llvm::BasicBlock::Create(*TheContext, "continue", gcDecFunc);
            auto finalizeBlock = llvm::BasicBlock::Create(*TheContext, "finalize", gcDecFunc);
            auto retBlock = llvm::BasicBlock::Create(*TheContext, "ret", gcDecFunc);

            Builder->SetInsertPoint(BB);
            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                // print function name
                auto* debugStrConst = llvm::ConstantDataArray::getString(*TheContext, decFuncName, true);
                auto* debugStrGlobal = new llvm::GlobalVariable(*TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                Builder->CreateCall(runtimeDebugReportCurrentFunctionFunc, llvm::ArrayRef<llvm::Value*>(debugArgs));
            }

            objPtr = gcDecFunc->arg_begin();
            refCounter = Builder->CreateStructGEP(structType, objPtr, 0, "ref_counter");
            // check whether object is null
            auto *isObjNull = Builder->CreateIsNull(objPtr, "is_obj_null");
            Builder->CreateCondBr(isObjNull, nullFailedBlock, continueBlock);

            Builder->SetInsertPoint(continueBlock);
            newRefCounter = Builder->CreateLoad(llvm::Type::getInt64Ty(*TheContext), refCounter, "new_ref_counter");
            newRefCounterVal = Builder->CreateSub(newRefCounter, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 1, true), "new_ref_counter_val");
            Builder->CreateStore(newRefCounterVal, refCounter);
            
            auto icmpRes = Builder->CreateICmpEQ(newRefCounterVal, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 0, true), "ref_counter_zero");
            Builder->CreateCondBr(icmpRes, finalizeBlock, retBlock);
            // ret block
            Builder->SetInsertPoint(retBlock);
            Builder->CreateRetVoid();
            // null failed block
            Builder->SetInsertPoint(nullFailedBlock);
            Builder->CreateRetVoid();
            // finalize block
            Builder->SetInsertPoint(finalizeBlock);
            // free memory
            if (type->type == IRValueType::valueType::structObject || type->type == IRValueType::valueType::interfaceObject) {
                // decrease the ref count of array elements inside
                auto arrayPointer = Builder->CreateStructGEP(structType, objPtr, 1, "array_ptr");
                for (yoi::indexT i = 0; i < size; i++) {
                    auto elementPointer = Builder->CreateGEP(arrayType, arrayPointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 0), llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), i)}, "element_ptr");
                    auto elementPointerVal = Builder->CreateLoad(llvm::PointerType::get(llvm::Type::getInt64Ty(*TheContext), 0), elementPointer, "element_ptr_val"); // just too lazy, so I use int64*
                    callGcFunction(elementPointerVal, managedPtr(type->getElementType()), false);
                }
            }
            Builder->CreateCall(runtimeFinalizeObjectFunc, objPtr);
            Builder->CreateRetVoid();
            // add gc function to function map
            functionMap[yoi::string2wstring(incFuncName)] = gcIncFunc;
            functionMap[yoi::string2wstring(decFuncName)] = gcDecFunc;
            // add struct to struct map
            arrayTypeMap[arrayKey] = structType;
            // clean up the mess, reset the insert point
            Builder->SetInsertPoint(currentInsertPoint);
            return structType;
        }
    }

    llvm::Value *LLVMCodegen::createArrayObject(const std::shared_ptr<IRValueType> &type,
                                                const yoi::vec<llvm::Value *> &elements) {
        yoi_assert(type->isArrayType(), 0, 0, "type must be an array type");
        llvm::Type *llvmType = getArrayLLVMType(type, false);
        // initialize the llvm struct, allocate memory and store the array
        auto memSize = TheModule->getDataLayout().getTypeAllocSize(llvmType);
        auto *memoryPointer = Builder->CreateCall(runtimeObjectAllocFunc, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), memSize, true)});

        // increase the refcount to 1
        auto *refCounter = Builder->CreateStructGEP(llvmType, memoryPointer, 0, "ref_counter");
        auto *refCounterVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 1, true);
        Builder->CreateStore(refCounterVal, refCounter);

        // store the array
        yoi::indexT index = 0;
        auto arrayBasePointer = Builder->CreateStructGEP(llvmType, memoryPointer, 1, "array_ptr");
        for (auto &i : elements) {
            // if basic type, unbox it first
            if (type->isBasicType()) {
                auto elementLLVMType = yoiTypeToLLVMType(managedPtr(type->getElementType()), true);
                auto arrayPointer = Builder->CreateGEP(elementLLVMType, arrayBasePointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), index)}, "array_element_ptr");
                auto val = unboxValue(i, managedPtr(type->getElementType()));
                Builder->CreateStore(val, arrayPointer);
            } else {
                // otherwise, store the pointer directly
                auto arrayPointer = Builder->CreateGEP(llvm::PointerType::get(yoiTypeToLLVMType(managedPtr(type->getElementType())), 0), arrayBasePointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), index)}, "array_element_ptr");
                Builder->CreateStore(i, arrayPointer);
            }
            index ++;
        }
        return memoryPointer;
    }

    llvm::Value *
    LLVMCodegen::loadArrayElement(const std::shared_ptr<IRValueType> &type, llvm::Value *arrayPtr, llvm::Value *index) {
        yoi_assert(type->isArrayType(), 0, 0, "type must be an array type");
        llvm::Type *llvmType = getArrayLLVMType(type, false);

        yoi::indexT arraySize = 1;
        for (auto &i : type->dimensions) {
            arraySize *= i;
        }

        auto *arrayPointer = Builder->CreateStructGEP(getArrayLLVMType(type), arrayPtr, 1, "array_ptr");
        switch (type->type) {
            case IRValueType::valueType::integerObject:
            case IRValueType::valueType::decimalObject:
            case IRValueType::valueType::booleanObject:
            case IRValueType::valueType::stringObject:
            case IRValueType::valueType::characterObject: {
                auto elementType = managedPtr(type->getElementType());
                auto elementLLVMType = yoiTypeToLLVMType(elementType, true);
                auto pointerToElement = Builder->CreateGEP(llvm::ArrayType::get(elementLLVMType, arraySize), arrayPointer, {
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 0),
                    index
                }, "element_ptr");
                auto loadedVal = Builder->CreateLoad(elementLLVMType, pointerToElement, "loaded_val"); // get unboxed value, so ffi type
                auto *val = createBasicObject(elementType, loadedVal);
                return val;
            }
            case IRValueType::valueType::structObject:
            case IRValueType::valueType::interfaceObject: {
                auto elementType = managedPtr(type->getElementType());
                auto elementLLVMType = yoiTypeToLLVMType(elementType);
                auto pointerToElement = Builder->CreateGEP(elementLLVMType, arrayPointer, index, "element_ptr");
                auto *loadedVal = Builder->CreateLoad(yoiTypeToLLVMType(elementType), pointerToElement, "loaded_val");
                callGcFunction(loadedVal, elementType, true); // increase ref count
                return loadedVal;
            }
            default: {
                panic(0, 0, "LLVM Codegen: Unhandled or unmapped array type: " + std::string(magic_enum::enum_name(type->type)));
                return nullptr;
            }
        }

    }
    LLVMCodegen::ControlFlowAnalysis::ControlFlowAnalysis(const std::vector<std::shared_ptr<IRCodeBlock>> &blocks) {
        for (yoi::indexT i = 0; i < blocks.size(); i++) {
            for (auto &ins : blocks[i]->getIRArray()) {
                switch (ins.opcode) {
                    case IR::Opcode::jump:
                    case IR::Opcode::jump_if_true:
                    case IR::Opcode::jump_if_false: {
                        G[i].push_back(ins.operands[0].value.symbolIndex);
                        reverseG[ins.operands[0].value.symbolIndex].push_back(i);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }
    }
} // namespace yoi