//
// Created by XIaokang00010 on 2024/10/9.
//

#include "llvmCodegenContext.hpp"
#include "compiler/builtinModule.hpp"
#include "compiler/compilerContext.h"
#include "compiler/ir/IR.h"
#include "compiler/ir/IRLinker.hpp"
#include "compiler/llvmCodegen/codegenObjectCache.hpp"
#include "share/def.hpp"
#include <algorithm>
#include <iostream>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/SubtargetFeature.h>
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
#include <memory>
#include <string>
#include <tuple>
#include <filesystem>

namespace yoi {

    LLVMCodegen::LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, const std::shared_ptr<IRModule> &yoiModule)
        : compilerCtx(std::move(compilerCtx)),
        yoiModule(yoiModule) {

        codegenObjectCache.setBuildConfig(this->compilerCtx->getBuildConfig());

        auto cache_path = std::filesystem::path(this->compilerCtx->getBuildConfig()->buildCachePath);
        if (std::filesystem::exists(cache_path / "hoshi.cache.tsuki")) {
            FILE* cache_file = fopen((cache_path / "hoshi.cache.tsuki").string().c_str(), "rb");
            yoi_assert(cache_file != nullptr, 0, 0, "failed to open cache file");
            
            serialization::read(cache_file, codegenObjectCache);
            fclose(cache_file);

            yoi::vec<yoi::wstr> source_files;
            for (auto &module : this->compilerCtx->getCompiledModules()) {
                if (module.second->modulePath == L"builtin")
                    continue;
                source_files.push_back(module.second->modulePath);
            }
            codegenObjectCache.purge_and_update(source_files);
        }
    }

    void LLVMCodegen::declareRuntimeFunctions(LLVMModuleContext &llvmModCtx) {
        // void* runtime_object_alloc(unsigned long long sizeOfObject) -> i8* (i64)
        llvm::Type* i8PtrTy = llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
        llvm::Type* sizeTy = llvmModCtx.Builder->getInt64Ty();

        llvm::FunctionType *mallocFuncType = llvm::FunctionType::get(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), {sizeTy, sizeTy}, false);
        llvmModCtx.runtimeFunctions[L"mi_calloc"] = llvm::Function::Create(mallocFuncType, llvm::Function::ExternalLinkage, "mi_calloc", llvmModCtx.TheModule.get());
        llvmModCtx.runtimeFunctions[L"mi_calloc"]->setCallingConv(llvm::CallingConv::C);

        llvm::FunctionType *freeFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {i8PtrTy}, false);
        llvmModCtx.runtimeFunctions[L"mi_free"] = llvm::Function::Create(freeFuncType, llvm::Function::ExternalLinkage, "mi_free", llvmModCtx.TheModule.get());
        llvmModCtx.runtimeFunctions[L"mi_free"]->setCallingConv(llvm::CallingConv::C);

        llvm::FunctionType* allocType = llvm::FunctionType::get(i8PtrTy, {sizeTy, i8PtrTy}, false);
        llvm::FunctionType* funcType = llvm::FunctionType::get(i8PtrTy, {sizeTy}, false);
        llvmModCtx.runtimeFunctions[L"runtime_object_alloc_report"] = llvm::Function::Create(allocType, llvm::Function::ExternalLinkage, "runtime_object_alloc_report", llvmModCtx.TheModule.get());
        llvmModCtx.runtimeFunctions[L"object_alloc"] = llvm::Function::Create(funcType, llvm::Function::LinkOnceODRLinkage, "object_alloc", llvmModCtx.TheModule.get());
        llvmModCtx.runtimeFunctions[L"object_alloc"]->addFnAttr(llvm::Attribute::AlwaysInline);

        // void runtime_finalize_object(void* objectPtr) -> void (i8*)
        llvm::FunctionType* finalizeType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {i8PtrTy}, false);
        llvmModCtx.runtimeFunctions[L"runtime_finalize_object_report"] = llvm::Function::Create(finalizeType, llvm::Function::ExternalLinkage, "runtime_finalize_object_report", llvmModCtx.TheModule.get());
        llvmModCtx.runtimeFunctions[L"finalize_object"] = llvm::Function::Create(finalizeType, llvm::Function::LinkOnceODRLinkage, "finalize_object", llvmModCtx.TheModule.get());
        llvmModCtx.runtimeFunctions[L"finalize_object"]->addFnAttr(llvm::Attribute::AlwaysInline);

        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            // void runtime_debug_report_current_function(const char *function_name);
            llvm::Type* constCharPtrTy = llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
            llvm::FunctionType* debugReportType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {constCharPtrTy}, false);
            llvmModCtx.runtimeFunctions[L"runtime_debug_report_current_function"] = llvm::Function::Create(debugReportType, llvm::Function::ExternalLinkage, "runtime_debug_report_current_function", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_report_current_function"]->setCallingConv(llvm::CallingConv::C);

            // void runtime_debug_report_leave_function(const char *function_name);
            llvmModCtx.runtimeFunctions[L"runtime_debug_report_leave_function"] = llvm::Function::Create(debugReportType, llvm::Function::ExternalLinkage, "runtime_debug_report_leave_function", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_report_leave_function"]->setCallingConv(llvm::CallingConv::C);

            // void runtime_debug_print(const char *message);
            llvm::FunctionType* debugPrintType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {constCharPtrTy}, false);
            llvmModCtx.runtimeFunctions[L"runtime_debug_print"] = llvm::Function::Create(debugPrintType, llvm::Function::ExternalLinkage, "runtime_debug_print", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_print"]->setCallingConv(llvm::CallingConv::C);

            // void runtime_debug_print_address(void *address);
            llvm::FunctionType* debugPrintAddressType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {i8PtrTy}, false);
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_address"] = llvm::Function::Create(debugPrintAddressType, llvm::Function::ExternalLinkage, "runtime_debug_print_address", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_address"]->setCallingConv(llvm::CallingConv::C);

            // void runtime_debug_print_int(int value);
            llvm::FunctionType* debugPrintIntType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmModCtx.Builder->getInt64Ty()}, false);
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_int"] = llvm::Function::Create(debugPrintIntType, llvm::Function::ExternalLinkage, "runtime_debug_print_int", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_int"]->setCallingConv(llvm::CallingConv::C);

            // void runtime_debug_print_deci(double value);
            llvm::FunctionType* debugPrintDeciType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmModCtx.Builder->getDoubleTy()}, false);
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_deci"] = llvm::Function::Create(debugPrintDeciType, llvm::Function::ExternalLinkage, "runtime_debug_print_deci", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_deci"]->setCallingConv(llvm::CallingConv::C);

            llvm::FunctionType *debugPrintCurrentAllocatedMemoryType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {}, false);
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_current_allocated_memory"] = llvm::Function::Create(debugPrintCurrentAllocatedMemoryType, llvm::Function::ExternalLinkage, "runtime_debug_print_current_allocated_memory", llvmModCtx.TheModule.get());
            llvmModCtx.runtimeFunctions[L"runtime_debug_print_current_allocated_memory"]->setCallingConv(llvm::CallingConv::C);

            if (llvmModCtx.compileUnits.find(L"builtin") == llvmModCtx.compileUnits.end()) {
                llvmModCtx.compileUnits[L"builtin"] = llvmModCtx.DBuilder->createCompileUnit(
                    llvm::dwarf::DW_LANG_C,
                    llvmModCtx.DBuilder->createFile("builtin", "."),
                    "hoshi-lang",
                    false,
                    "",
                    0
                );
            }
        }
    }

    void LLVMCodegen::generateRuntimeFunctionImplementations(LLVMModuleContext &llvmModCtx) {
        llvm::Type* i8PtrTy = llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
        llvm::Type* sizeTy = llvmModCtx.Builder->getInt64Ty();

        // object_alloc
        auto* objAllocFunc = llvmModCtx.runtimeFunctions.at(L"object_alloc");
        llvm::BasicBlock *entryOA = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", objAllocFunc);
        llvmModCtx.Builder->SetInsertPoint(entryOA);
        llvm::Value* sizeOfObject = objAllocFunc->arg_begin();
        llvm::Value *mem = llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"mi_calloc"), {llvm::ConstantInt::get(sizeTy, 1), sizeOfObject});
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_object_alloc_report"), {sizeOfObject, mem});
        }
        llvmModCtx.Builder->CreateRet(mem);

        // finalize_object
        auto* finalizeObjFunc = llvmModCtx.runtimeFunctions.at(L"finalize_object");
        llvm::BasicBlock *entryFO = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", finalizeObjFunc);
        llvmModCtx.Builder->SetInsertPoint(entryFO);
        llvm::Value* objectPtr = finalizeObjFunc->arg_begin();
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_finalize_object_report"), {objectPtr});
        }
        llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"mi_free"), {objectPtr});
        llvmModCtx.Builder->CreateRetVoid();
    }

    void LLVMCodegen::generate(LLVMModuleContext &llvmModCtx) {
        TIMER("generateDeclarations", generateDeclarations(llvmModCtx));
        TIMER("generateForeignStructTypes", generateForeignStructTypes(llvmModCtx));
        TIMER("generateImportFunctionImplementations", generateImportFunctionImplementations(llvmModCtx));
        TIMER("generateImplementations", generateImplementations(llvmModCtx));
        TIMER("generateDescription", generateDescription(llvmModCtx));
        TIMER("generateExportFunctionDecls", generateExportFunctionDecls(llvmModCtx));
        TIMER("generateMainFunction", generateMainFunction(llvmModCtx));
        TIMER("generateRTTIImplmentation", generateRTTIImplmentation(llvmModCtx));
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            TIMER("llvmModCtx.DBuilder->finalize()", llvmModCtx.DBuilder->finalize());
        }
    }

    llvm::Module* LLVMCodegen::getModule(LLVMModuleContext &llvmModCtx) {
        return llvmModCtx.TheModule.get();
    }

    void LLVMCodegen::generateBasicTypeDeclarations(LLVMModuleContext &llvmModCtx) {
        // --- Declare Basic Object Struct Types ---
        std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Type*>> basicTypes = {
            {compilerCtx->getIntObjectType(), llvmModCtx.Builder->getInt64Ty()},
            {compilerCtx->getDeciObjectType(), llvmModCtx.Builder->getDoubleTy()},
            {compilerCtx->getBoolObjectType(), llvmModCtx.Builder->getInt1Ty()},
            {compilerCtx->getCharObjectType(), llvmModCtx.Builder->getInt8Ty()},
            {compilerCtx->getStrObjectType(), llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0)},
            {compilerCtx->getUnsignedObjectType(), llvmModCtx.Builder->getInt64Ty()},
            {compilerCtx->getShortObjectType(), llvmModCtx.Builder->getInt16Ty()}
        };

        for (const auto& pair : basicTypes) {
            auto yoiType = pair.first;
            auto rawType = pair.second;
            auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
            auto name = "yoi.basic." + wstring2string(yoiType->to_string());
            auto typeName = wstring2string(yoiType->to_string());
            auto* structType = llvm::StructType::create(*llvmModCtx.TheContext, {llvmModCtx.Builder->getInt64Ty(), llvmModCtx.Builder->getInt64Ty(), rawType}, name);
            auto* llvmStructPtrType = llvm::PointerType::get(structType, 0);
            llvmModCtx.structTypeMap[key] = structType;
            llvmModCtx.foreignTypeMap[key] = rawType;
            auto typeIdKey = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex, 0);
            llvmModCtx.typeIDMap[typeIdKey] = llvmModCtx.nextTypeId++;

            auto incFuncName = "basic_" + typeName + "_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::LinkOnceODRLinkage, incFuncName, llvmModCtx.TheModule.get());
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
#ifdef _WIN32
            llvm::Comdat *incC = llvmModCtx.TheModule->getOrInsertComdat(incFuncName);
            incC->setSelectionKind(llvm::Comdat::Any);
            incFunction->setComdat(incC);
#endif
            llvmModCtx.functionMap[string2wstring(incFuncName)] = incFunction;

            auto decFuncName = "basic_" + typeName + "_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::LinkOnceODRLinkage, decFuncName, llvmModCtx.TheModule.get());
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
#ifdef _WIN32
            llvm::Comdat *decC = llvmModCtx.TheModule->getOrInsertComdat(decFuncName);
            decC->setSelectionKind(llvm::Comdat::Any);
            decFunction->setComdat(decC);
#endif
            llvmModCtx.functionMap[string2wstring(decFuncName)] = decFunction;

            // generate basic type dyn array function
            getArrayLLVMType(llvmModCtx, managedPtr(pair.first->getDynamicArrayType()));
        }

        // --- Handle 'none' type as a special singleton object ---
        auto noneYoiType = compilerCtx->getNoneObjectType();
        auto noneKey = std::make_tuple(noneYoiType->type, noneYoiType->typeAffiliateModule, noneYoiType->typeIndex);
        llvmModCtx.foreignTypeMap[noneKey] = llvm::Type::getVoidTy(*llvmModCtx.TheContext);
    }

    void LLVMCodegen::generateBasicTypeImplementations(LLVMModuleContext &llvmModCtx) {
        std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Type*>> basicTypes = {
            {compilerCtx->getIntObjectType(), llvmModCtx.Builder->getInt64Ty()},
            {compilerCtx->getDeciObjectType(), llvmModCtx.Builder->getDoubleTy()},
            {compilerCtx->getBoolObjectType(), llvmModCtx.Builder->getInt1Ty()},
            {compilerCtx->getCharObjectType(), llvmModCtx.Builder->getInt8Ty()},
            {compilerCtx->getStrObjectType(), llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0)},
            {compilerCtx->getUnsignedObjectType(), llvmModCtx.Builder->getInt64Ty()},
            {compilerCtx->getShortObjectType(), llvmModCtx.Builder->getInt16Ty()}
        };

        // --- Generate GC Functions for Other Basic Types ---
        for (const auto& pair : basicTypes) {
            auto yoiType = pair.first;
            auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
            auto* llvmStructType = llvmModCtx.structTypeMap.at(key);
            auto* llvmStructPtrType = llvm::PointerType::get(llvmStructType, 0);
            auto typeName = wstring2string(yoiType->to_string());

            // --- Generate gc_refcount_increase ---
            auto incFuncName = "basic_" + typeName + "_gc_refcount_increase";
            auto* incFunction = llvmModCtx.functionMap.at(string2wstring(incFuncName));

            auto* incBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", incFunction);
            llvmModCtx.Builder->SetInsertPoint(incBlock);
            llvm::Value* thisPtr = incFunction->arg_begin();

            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                std::string debugStr = "Increasing refcount of " + typeName + " object";
                auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, debugStr, true);
                auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalValue::PrivateLinkage, debugStrConst, "debug_str");
                auto* debugStrPtr = llvmModCtx.Builder->CreateBitCast(debugStrGlobal, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print"), debugStrPtr);
                // address
                auto* castedPtr = llvmModCtx.Builder->CreateBitCast(thisPtr, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print_address"), castedPtr);
            }

            llvm::Value* incRefCountPtr = llvmModCtx.Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            auto beforeInc = llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::Add, incRefCountPtr, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);
            llvmModCtx.Builder->CreateRetVoid();

            // --- Generate gc_refcount_decrease ---
            auto decFuncName = "basic_" + typeName + "_gc_refcount_decrease";
            auto* decFunction = llvmModCtx.functionMap.at(string2wstring(decFuncName));

            auto* entryBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", decFunction);
            auto* finalizeBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "finalize", decFunction);
            auto* continueBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "continue", decFunction);

            llvmModCtx.Builder->SetInsertPoint(entryBlock);
            thisPtr = decFunction->arg_begin();
            
            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                std::string debugStr = "Decreasing refcount of " + typeName + " object";
                auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, debugStr, true);
                auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalValue::PrivateLinkage, debugStrConst, "debug_str");
                auto* debugStrPtr = llvmModCtx.Builder->CreateBitCast(debugStrGlobal, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print"), debugStrPtr);
                // address
                auto* castedPtr = llvmModCtx.Builder->CreateBitCast(thisPtr, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print_address"), castedPtr);
            }
            llvm::Value* decRefCountPtr = llvmModCtx.Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            llvm::Value* decOldRefCount = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), decRefCountPtr, "old_refcount");
            auto beforeDec = llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::Sub, decRefCountPtr, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);

            llvm::Value* shouldFinalize = llvmModCtx.Builder->CreateICmpSLE(beforeDec, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), "should_finalize");
            llvmModCtx.Builder->CreateCondBr(shouldFinalize, finalizeBlock, continueBlock);

            llvm::Value* castedPtr = llvmModCtx.Builder->CreateBitCast(thisPtr, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
            llvmModCtx.Builder->SetInsertPoint(finalizeBlock);
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"finalize_object"), castedPtr);
            llvmModCtx.Builder->CreateBr(continueBlock);

            llvmModCtx.Builder->SetInsertPoint(continueBlock);
            llvmModCtx.Builder->CreateRetVoid();
        }
    }


    // --- DECLARATION PHASE ---

    void LLVMCodegen::generateDeclarations(LLVMModuleContext &llvmModCtx) {
        declareRuntimeFunctions(llvmModCtx);
        generateBasicTypeDeclarations(llvmModCtx);
        generateStructShallowDeclarations(llvmModCtx);
        generateGlobalDeclarations(llvmModCtx);
        generateFunctionDeclarations(llvmModCtx);
        generateStructDeclarations(llvmModCtx);
        generateStructGCFunctionDeclarations(llvmModCtx);
        generateInterfaceObjectGCFunctionDeclarations(llvmModCtx);
        generateImportFunctionDeclarations(llvmModCtx);
        generateRTTIDeclaration(llvmModCtx);
    }

    void LLVMCodegen::generateStructShallowDeclarations(LLVMModuleContext &llvmModCtx) {
        for (auto& structDefPair : yoiModule->structTable) {
            auto structDef = structDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, yoiModule->structTable.getIndex(structDef->name));
            auto structName = "struct." + std::to_string(yoiModule->identifier) + "." + wstring2string(structDef->name);
            llvmModCtx.structTypeMap[key] = llvm::StructType::create(*llvmModCtx.TheContext, structName);
            auto typeIdKey = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, yoiModule->structTable.getIndex(structDef->name), 0);
            llvmModCtx.typeIDMap[typeIdKey] = llvmModCtx.nextTypeId++;
        }
        for (auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, yoiModule->interfaceTable.getIndex(interfaceDef->name));
            auto interfaceName = "interface." + std::to_string(yoiModule->identifier) + "." + wstring2string(interfaceDef->name);
            llvmModCtx.structTypeMap[key] = llvm::StructType::create(*llvmModCtx.TheContext, interfaceName);
            auto typeIdKey = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, yoiModule->interfaceTable.getIndex(interfaceDef->name), 0);
            llvmModCtx.typeIDMap[typeIdKey] = llvmModCtx.nextTypeId++;
        }
    }

    void LLVMCodegen::generateGlobalDeclarations(LLVMModuleContext &llvmModCtx) {
        for (auto& globalPair : yoiModule->globalVariables) {
            auto globalName = wstring2string(globalPair.first);
            // All globals are pointers to objects.
            auto globalType = yoiTypeToLLVMType(llvmModCtx, globalPair.second);
            auto initializer = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(globalType));
            auto* globalVar = new llvm::GlobalVariable(*llvmModCtx.TheModule, globalType, false, llvm::GlobalValue::CommonLinkage, initializer, globalName);
            llvmModCtx.globalValues[yoiModule->globalVariables.getIndex(globalPair.first)] = globalVar;
        }
    }

    void LLVMCodegen::generateFunctionDeclarations(LLVMModuleContext &llvmModCtx) {
        for (auto& funcPair : yoiModule->functionTable) {
            if (funcPair.second->hasAttribute(IRFunctionDefinition::FunctionAttrs::Unreachable))
                continue;

            auto funcDef = funcPair.second;
            auto funcName = wstring2string(funcDef->name);
            auto* funcType = getFunctionType(llvmModCtx, funcDef);
            auto* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, llvmModCtx.TheModule.get());

            if (funcDef->hasAttribute(IRFunctionDefinition::FunctionAttrs::AlwaysInline)) {
                function->addFnAttr(llvm::Attribute::AlwaysInline);
            }
            
            llvmModCtx.functionMap[funcDef->name] = function;
        }
    }

    // --- IMPLEMENTATION PHASE ---

    void LLVMCodegen::generateImplementations(LLVMModuleContext &llvmModCtx) {
        generateRuntimeFunctionImplementations(llvmModCtx);
        generateBasicTypeImplementations(llvmModCtx);
        generateStructGCFunctionImplementations(llvmModCtx);
        generateInterfaceObjectGCFunctionImplementations(llvmModCtx);
        generateFunctionImplementations(llvmModCtx);
    }

    void LLVMCodegen::generateStructDeclarations(LLVMModuleContext &llvmModCtx) {
        for (auto& structDefPair : yoiModule->structTable) {
            auto structDef = structDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, yoiModule->structTable.getIndex(structDef->name));
            auto* llvmStructType = llvmModCtx.structTypeMap.at(key);

            std::vector<llvm::Type*> fieldTypes;
            fieldTypes.push_back(llvmModCtx.Builder->getInt64Ty()); // gc_refcount
            fieldTypes.push_back(llvmModCtx.Builder->getInt64Ty()); // typeid
            for (const auto& fieldType : structDef->fieldTypes) {
                fieldTypes.push_back(yoiTypeToLLVMType(llvmModCtx, fieldType));
            }
            if (llvmStructType->isOpaque()) {
                llvmStructType->setBody(fieldTypes);
            }
        }
        for (auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, yoiModule->interfaceTable.getIndex(interfaceDef->name));
            auto* llvmInterfaceType = llvmModCtx.structTypeMap.at(key);

            std::vector<llvm::Type*> memberTypes;
            memberTypes.push_back(llvmModCtx.Builder->getInt64Ty()); // [0] refcount
            memberTypes.push_back(llvmModCtx.Builder->getInt64Ty()); // [1] typeid
            memberTypes.push_back(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0)); // [2] this ptr
            auto* gcFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), { llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0) }, false);
            auto* gcFuncPtrType = llvm::PointerType::get(gcFuncType, 0);
            memberTypes.push_back(gcFuncPtrType); // [3] gc_refcount_increase vptr
            memberTypes.push_back(gcFuncPtrType); // [4] gc_refcount_decrease vptr

            for (const auto& methodPair : interfaceDef->methodMap) {
                auto funcType = getFunctionType(llvmModCtx, methodPair.second);
                std::vector<llvm::Type*> virtualArgTypes;
                virtualArgTypes.push_back(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0)); // 'this' is always i8*
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

    void LLVMCodegen::generateStructGCFunctionDeclarations(LLVMModuleContext &llvmModCtx) {
        for (auto& structDefPair : yoiModule->structTable) {
            auto structDef = structDefPair.second;
            auto structIdx = yoiModule->structTable.getIndex(structDef->name);
            auto moduleID = yoiModule->identifier;
            auto key = std::make_tuple(IRValueType::valueType::structObject, moduleID, structIdx);
            auto* llvmStructType = llvmModCtx.structTypeMap.at(key);
            auto* llvmStructPtrType = llvm::PointerType::get(llvmStructType, 0);

            // --- Generate gc_refcount_increase ---
            auto incFuncName = "struct_" + std::to_string(moduleID) + "_" + std::to_string(structIdx) + "_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::LinkOnceODRLinkage, incFuncName, llvmModCtx.TheModule.get());
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
#ifdef _WIN32
            llvm::Comdat *incC = llvmModCtx.TheModule->getOrInsertComdat(incFuncName);
            incC->setSelectionKind(llvm::Comdat::Any);
            incFunction->setComdat(incC);
#endif
            llvmModCtx.functionMap[string2wstring(incFuncName)] = incFunction;

            // --- Generate gc_refcount_decrease ---
            auto decFuncName = "struct_" + std::to_string(moduleID) + "_" + std::to_string(structIdx) + "_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmStructPtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::LinkOnceODRLinkage, decFuncName, llvmModCtx.TheModule.get());
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
#ifdef _WIN32
            llvm::Comdat *decC = llvmModCtx.TheModule->getOrInsertComdat(decFuncName);
            decC->setSelectionKind(llvm::Comdat::Any);
            decFunction->setComdat(decC);
#endif
            llvmModCtx.functionMap[string2wstring(decFuncName)] = decFunction;
        }
    }

    void LLVMCodegen::generateStructGCFunctionImplementations(LLVMModuleContext &llvmModCtx) {
        for (auto& structDefPair : yoiModule->structTable) {
            auto moduleID = yoiModule->identifier;
            auto structDef = structDefPair.second;
            // if (compilerCtx->getImportedModule(structDef->linkedModuleId)->modulePath != llvmModCtx.absolute_path)
            //    continue;
            auto structIdx = yoiModule->structTable.getIndex(structDef->name);
            auto key = std::make_tuple(IRValueType::valueType::structObject, moduleID, structIdx);
            auto* llvmStructType = llvmModCtx.structTypeMap.at(key);
            auto* llvmStructPtrType = llvm::PointerType::get(llvmStructType, 0);

            // --- Generate gc_refcount_increase ---
            auto incFuncName = "struct_" + std::to_string(moduleID) + "_" + std::to_string(structIdx) + "_gc_refcount_increase";
            auto incFunction = llvmModCtx.functionMap[string2wstring(incFuncName)];

            auto* incBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", incFunction);
            llvmModCtx.Builder->SetInsertPoint(incBlock);
            llvm::Value* thisPtr = incFunction->arg_begin();
            llvm::Value* refCountPtr = llvmModCtx.Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            auto beforeInc = llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::Add, refCountPtr, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);
            llvmModCtx.Builder->CreateRetVoid();

            // --- Generate gc_refcount_decrease ---
            auto decFuncName = "struct_" + std::to_string(moduleID) + "_" + std::to_string(structIdx) + "_gc_refcount_decrease";
            auto decFunction = llvmModCtx.functionMap[string2wstring(decFuncName)];

            auto* entryBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", decFunction);
            auto* finalizeBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "finalize", decFunction);
            auto* continueBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "continue", decFunction);

            llvmModCtx.Builder->SetInsertPoint(entryBlock);
            thisPtr = decFunction->arg_begin();

            refCountPtr = llvmModCtx.Builder->CreateStructGEP(llvmStructType, thisPtr, 0, "refcount_ptr");
            auto beforeDec = llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::Sub, refCountPtr, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);

            llvm::Value* shouldFinalize = llvmModCtx.Builder->CreateICmpSLE(beforeDec, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), "should_finalize");
            llvmModCtx.Builder->CreateCondBr(shouldFinalize, finalizeBlock, continueBlock);

            llvmModCtx.Builder->SetInsertPoint(finalizeBlock);
            llvm::Value* castedPtr = llvmModCtx.Builder->CreateBitCast(thisPtr, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
            // if any finalizer presents, call it
            if (auto funcName = structDef->name + L"::finalizer"; llvmModCtx.functionMap[funcName] != nullptr && yoiModule->functionTable[funcName]->hasAttribute(IRFunctionDefinition::FunctionAttrs::Finalizer)) {
                llvmModCtx.Builder->CreateCall(llvmModCtx.functionMap[funcName], {castedPtr});
            }
            // call dec for inner object (if any)
            for (yoi::indexT innerIdx = 0; innerIdx < structDef->fieldTypes.size(); ++innerIdx) {
                // create gep
                auto fieldPtr = llvmModCtx.Builder->CreateStructGEP(llvmStructType, thisPtr, innerIdx + 2, "field_ptr"); // skip refcount at index 0, and typeid at index 1
                auto fieldType = structDef->fieldTypes[innerIdx];
                // Load the field value before calling its GC function
                llvm::Value* loadedField = llvmModCtx.Builder->CreateLoad(yoiTypeToLLVMType(llvmModCtx, fieldType), fieldPtr, "loaded_field_for_gc");
                callGcFunction(llvmModCtx, loadedField, fieldType, false); // Decrease refcount of member
            }
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"finalize_object"), castedPtr);
            llvmModCtx.Builder->CreateBr(continueBlock);

            llvmModCtx.Builder->SetInsertPoint(continueBlock);
            llvmModCtx.Builder->CreateRetVoid();
        }
    }

    void LLVMCodegen::generateInterfaceObjectGCFunctionDeclarations(LLVMModuleContext &llvmModCtx) {
        // These are the top-level GC wrappers for the interface objects themselves.
        // They manage the interface object's own refcount and dispatch to the interfaceImpl wrappers.
        for (const auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            auto interfaceIdx = yoiModule->interfaceTable.getIndex(interfaceDef->name);
            auto moduleID = yoiModule->identifier;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, moduleID, interfaceIdx);
            auto* llvmInterfaceType = llvmModCtx.structTypeMap.at(key);
            auto* llvmInterfacePtrType = llvm::PointerType::get(llvmInterfaceType, 0);
            auto* i8PtrTy = llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
            auto* gcFuncTypeForDispatch = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), { i8PtrTy }, false);
            auto* gcFuncPtrTypeForDispatch = llvm::PointerType::get(gcFuncTypeForDispatch, 0);

            auto incFuncName = "interface_" + std::to_string(moduleID) + "_" + std::to_string(interfaceIdx) + "_gc_refcount_increase";
            auto* incFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmInterfacePtrType}, false);
            auto* incFunction = llvm::Function::Create(incFuncType, llvm::Function::LinkOnceODRLinkage, incFuncName, llvmModCtx.TheModule.get());
            incFunction->addFnAttr(llvm::Attribute::AlwaysInline);
#ifdef _WIN32
            llvm::Comdat *incC = llvmModCtx.TheModule->getOrInsertComdat(incFuncName);
            incC->setSelectionKind(llvm::Comdat::Any);
            incFunction->setComdat(incC);
#endif
            llvmModCtx.functionMap[string2wstring(incFuncName)] = incFunction;

            auto decFuncName = "interface_" + std::to_string(moduleID) + "_" + std::to_string(interfaceIdx) + "_gc_refcount_decrease";
            auto* decFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvmInterfacePtrType}, false);
            auto* decFunction = llvm::Function::Create(decFuncType, llvm::Function::LinkOnceODRLinkage, decFuncName, llvmModCtx.TheModule.get());
            decFunction->addFnAttr(llvm::Attribute::AlwaysInline);
#ifdef _WIN32
            llvm::Comdat *decC = llvmModCtx.TheModule->getOrInsertComdat(decFuncName);
            decC->setSelectionKind(llvm::Comdat::Any);
            decFunction->setComdat(decC);
#endif
            llvmModCtx.functionMap[string2wstring(decFuncName)] = decFunction;
        }
    }

    void LLVMCodegen::generateInterfaceObjectGCFunctionImplementations(LLVMModuleContext &llvmModCtx) {
        for (const auto& interfaceDefPair : yoiModule->interfaceTable) {
            auto interfaceDef = interfaceDefPair.second;
            // if (compilerCtx->getImportedModule(interfaceDef->linkedModuleId)->modulePath != llvmModCtx.absolute_path)
            //    continue;
            auto interfaceIdx = yoiModule->interfaceTable.getIndex(interfaceDef->name);
            auto moduleID = yoiModule->identifier;
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, moduleID, interfaceIdx);
            auto* llvmInterfaceType = llvmModCtx.structTypeMap.at(key);
            auto* llvmInterfacePtrType = llvm::PointerType::get(llvmInterfaceType, 0);
            auto* i8PtrTy = llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
            auto* gcFuncTypeForDispatch = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), { i8PtrTy }, false);
            auto* gcFuncPtrTypeForDispatch = llvm::PointerType::get(gcFuncTypeForDispatch, 0);

            auto incFuncName = "interface_" + std::to_string(moduleID) + "_" + std::to_string(interfaceIdx) + "_gc_refcount_increase";
            auto incFunction = llvmModCtx.functionMap[string2wstring(incFuncName)];

            auto* incEntryBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", incFunction);

            llvmModCtx.Builder->SetInsertPoint(incEntryBlock);
            llvm::Value* thisPtr = incFunction->arg_begin();

            llvm::Value* incRefCountPtr = llvmModCtx.Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 0, "refcount_ptr");
            // llvm::Value* incOldRefCount = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), incRefCountPtr, "old_refcount");
            auto beforeInc = llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::Add, incRefCountPtr, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);
            llvmModCtx.Builder->CreateRetVoid();

            auto decFuncName = "interface_" + std::to_string(moduleID) + "_" + std::to_string(interfaceIdx) + "_gc_refcount_decrease";
            auto decFunction = llvmModCtx.functionMap[string2wstring(decFuncName)];
            llvmModCtx.functionMap[string2wstring(decFuncName)] = decFunction;

            auto* decEntryBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", decFunction);
            auto* decFinalizeBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "finalize", decFunction);
            auto* decContinueBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "continue", decFunction);

            llvmModCtx.Builder->SetInsertPoint(decEntryBlock);
            thisPtr = decFunction->arg_begin();

            llvm::Value* decRefCountPtr = llvmModCtx.Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 0, "refcount_ptr");
            // llvm::Value* decOldRefCount = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), decRefCountPtr, "old_refcount");
            // llvm::Value* decNewRefCount = llvmModCtx.Builder->CreateSub(decOldRefCount, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), "new_refcount");
            // llvmModCtx.Builder->CreateStore(decNewRefCount, decRefCountPtr);
            auto beforeDec = llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::Sub, decRefCountPtr, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);

            llvm::Value* shouldFinalize = llvmModCtx.Builder->CreateICmpSLE(beforeDec, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), "should_finalize");
            llvmModCtx.Builder->CreateCondBr(shouldFinalize, decFinalizeBlock, decContinueBlock);

            llvmModCtx.Builder->SetInsertPoint(decFinalizeBlock);

            auto* concreteThisPtr = llvmModCtx.Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 2, "this_ptr_field");
            auto* loadedConcreteThis = llvmModCtx.Builder->CreateLoad(i8PtrTy, concreteThisPtr, "concrete_this");

            llvm::Value* gcDecSlotPtr = llvmModCtx.Builder->CreateStructGEP(llvmInterfaceType, thisPtr, 4, "gc_dec_slot");
            llvm::Value* gcDecFuncPtr = llvmModCtx.Builder->CreateLoad(gcFuncPtrTypeForDispatch, gcDecSlotPtr, "gc_func_ptr");
            llvmModCtx.Builder->CreateCall(gcFuncTypeForDispatch, gcDecFuncPtr, {loadedConcreteThis});

            llvm::Value* castedInterfacePtr = llvmModCtx.Builder->CreateBitCast(thisPtr, i8PtrTy);
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"finalize_object"), castedInterfacePtr);
            llvmModCtx.Builder->CreateBr(decContinueBlock);
            llvmModCtx.Builder->SetInsertPoint(decContinueBlock);
            llvmModCtx.Builder->CreateRetVoid();
        }
    }


    void LLVMCodegen::generateFunctionImplementations(LLVMModuleContext &llvmModCtx) {
        for (auto& funcPair : yoiModule->functionTable) {
            if (funcPair.second->hasAttribute(IRFunctionDefinition::FunctionAttrs::Unreachable))
                continue;

            if (compilerCtx->getImportedModule(funcPair.second->linkedModuleId)->modulePath != llvmModCtx.absolute_path) {
                continue;
            }
            
            if (!funcPair.second->codeBlock.empty()) {
                llvmModCtx.currentFunctionDef = funcPair.second;
                generateFunction(llvmModCtx, *funcPair.second);
            }
        }
    }

    void LLVMCodegen::generateFunction(LLVMModuleContext &llvmModCtx, IRFunctionDefinition& funcDef) {
        llvmModCtx.currentFunction = llvmModCtx.functionMap.at(funcDef.name);
        if (funcDef.codeBlock.empty()) return;

        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            auto sourceFileKey = funcDef.debugInfo.sourceFile == L"<entry>" ? L"builtin" : funcDef.debugInfo.sourceFile;
            if (auto it = llvmModCtx.compileUnits.find(sourceFileKey); it == llvmModCtx.compileUnits.end()) {
                std::filesystem::path sourceFile = std::filesystem::path(sourceFileKey);

                llvmModCtx.compileUnits[sourceFileKey] = llvmModCtx.DBuilder->createCompileUnit(
                    llvm::dwarf::DW_LANG_C,
                    llvmModCtx.DBuilder->createFile(sourceFile.filename().string(), sourceFile.parent_path().string()),
                    "hoshi-lang",
                    false,
                    "",
                    0
                );
            }
            auto diFile = llvmModCtx.compileUnits[sourceFileKey];

            llvm::SmallVector<llvm::Metadata *, 8> argsDIInfo;
            if (funcDef.returnType->type == IRValueType::valueType::none) {
                argsDIInfo.push_back(nullptr);
            } else {
                argsDIInfo.push_back(getDIType(llvmModCtx, funcDef.returnType));
            }

            for (const auto& argType : funcDef.argumentTypes) {
                argsDIInfo.push_back(getDIType(llvmModCtx, argType));
            }

            auto *subroutineType = llvmModCtx.DBuilder->createSubroutineType(llvmModCtx.DBuilder->getOrCreateTypeArray(argsDIInfo));
            
            auto *sp = llvmModCtx.DBuilder->createFunction(
                (llvm::DIScope*) diFile->getFile(),
                yoi::wstring2string(funcDef.name),
                "",
                diFile->getFile(),
                funcDef.debugInfo.line + 1,
                subroutineType,
                funcDef.debugInfo.line + 1,
                llvm::DINode::FlagPrototyped,
                llvm::DISubprogram::SPFlagDefinition
            );
            llvmModCtx.currentFunction->setSubprogram(sp);
        }

        llvmModCtx.controlFlowAnalysis = ControlFlowAnalysis{funcDef.codeBlock};
        llvmModCtx.valueStackPhi.clear();
        llvmModCtx.basicBlockMap.clear();
        llvmModCtx.basicBlockVisited.clear();

        llvmModCtx.basicBlockMap[0] = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", llvmModCtx.currentFunction);
        auto* entryBlock = llvmModCtx.basicBlockMap[0];
        llvmModCtx.Builder->SetInsertPoint(entryBlock);

        // invoke runtime_debug_report_current_function
        set_current_file_path(funcDef.debugInfo.sourceFile);
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug){
            llvmModCtx.Builder->SetCurrentDebugLocation({llvm::DILocation::get(*llvmModCtx.TheContext, funcDef.debugInfo.line + 1, funcDef.debugInfo.column + 1, llvmModCtx.currentFunction->getSubprogram())});
            std::string funcName = wstring2string(funcDef.name);
            auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, funcName, true);
            auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
            auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_report_current_function"), llvm::ArrayRef<llvm::Value*>(debugArgs));
        }


        llvmModCtx.namedValues.clear();
        auto& varTableRef = funcDef.getVariableTable();

        // Allocate space for all local variables (args + locals) and init to null
        auto& vars = varTableRef.getVariables();
        auto& names = varTableRef.getReversedVariableNameMap();
        for (yoi::indexT i = 0; i < vars.size(); ++i) {
            vars[i] = managedPtr(IRValueType{*(vars[i])}.addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope));
            auto* llvmType = yoiTypeToLLVMType(llvmModCtx, vars[i]);
            auto* alloca = llvmModCtx.Builder->CreateAlloca(llvmType, nullptr, wstring2string(names.at(i)));
            llvmModCtx.Builder->CreateStore(llvm::Constant::getNullValue(llvmType), alloca);
            llvmModCtx.namedValues[i] = alloca;

            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                std::filesystem::path sourcePath(funcDef.debugInfo.sourceFile);
                auto* DILocalVar = llvmModCtx.DBuilder->createAutoVariable(
                    llvmModCtx.currentFunction->getSubprogram(),
                    wstring2string(names.at(i)),
                    llvmModCtx.DBuilder->createFile(sourcePath.filename().string(), sourcePath.parent_path().string()),
                    funcDef.debugInfo.line + 1,
                    getDIType(llvmModCtx, vars[i])
                );

                llvmModCtx.DBuilder->insertDeclare(
                    alloca,      // The memory location of the variable
                    DILocalVar,  // The debug info for the variable
                    llvmModCtx.DBuilder->createExpression(), // An empty expression
                    llvm::DILocation::get(*llvmModCtx.TheContext, funcDef.debugInfo.line, 1, llvmModCtx.currentFunction->getSubprogram()),
                    llvmModCtx.Builder->GetInsertBlock()
                );
            }
        }

        // Store incoming arguments into their allocas, handling reference counts
        auto arg_it = llvmModCtx.currentFunction->arg_begin();
        for (yoi::indexT i = 0; i < funcDef.argumentTypes.size(); ++i, ++arg_it) {
            auto* alloca = llvmModCtx.namedValues.at(i);
            // Arguments are considered "retained" by the callee
            llvmModCtx.Builder->CreateStore(arg_it, alloca);
        }

        generateCodeBlock(llvmModCtx, *funcDef.codeBlock[0], 0, 0);
        llvmModCtx.DBuilder->finalize();
        if (llvm::verifyFunction(*llvmModCtx.currentFunction, &llvm::errs())) {
            llvmModCtx.TheModule->print(llvm::errs(), nullptr);
            panic(funcDef.debugInfo.line, funcDef.debugInfo.column, "LLVM function verification failed for: " + wstring2string(funcDef.name));
        }
    }

    void LLVMCodegen::generateFunctionExitCleanup(LLVMModuleContext &llvmModCtx) {
        for (const auto& pair : llvmModCtx.namedValues) {
            auto varIndex = pair.first;
            auto* alloca = pair.second;
            auto varYoiType = llvmModCtx.currentFunctionDef->variableTable.get(varIndex);

            // Load the final pointer value from the local variable
            auto* objPtr = llvmModCtx.Builder->CreateLoad(alloca->getAllocatedType(), alloca, "cleanup_load");

            // Decrease its reference count
            if (varYoiType->hasAttribute(IRValueType::ValueAttr::Nullable))
                callGcFunction(llvmModCtx, objPtr, varYoiType, false, true);
            else
                generateIfTargetNotNull(llvmModCtx, objPtr, varYoiType, [&] () {
                    callGcFunction(llvmModCtx, objPtr, varYoiType, false, true);
                }, true);
        }
    }

    void LLVMCodegen::generateCodeBlock(LLVMModuleContext &llvmModCtx, IRCodeBlock& block, yoi::indexT fromBlock, yoi::indexT toBlock, llvm::BasicBlock *actualFromBlock) {
        // check whether generated
        if (llvmModCtx.basicBlockVisited.contains(toBlock) && toBlock != 0) {
            // merge stack values
            llvmModCtx.valueStackPhi.enterNode(toBlock, fromBlock, llvmModCtx.basicBlockMap.at(toBlock), actualFromBlock ? actualFromBlock : llvmModCtx.basicBlockMap.at(fromBlock));
            llvmModCtx.valueStackPhi.finalizeNode();
            return;
        }
        llvmModCtx.basicBlockVisited[toBlock] = true;

        llvmModCtx.Builder->SetInsertPoint(llvmModCtx.basicBlockMap.at(toBlock));
        if (llvmModCtx.Builder->GetInsertBlock()->getTerminator()) return;

        for (const auto& succ : llvmModCtx.controlFlowAnalysis.G[toBlock]) {
            if (!llvmModCtx.basicBlockMap.contains(succ)) {
                llvmModCtx.basicBlockMap[succ] = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "block_" + std::to_string(succ), llvmModCtx.currentFunction);
            }
        }

        llvmModCtx.valueStackPhi.enterNode(toBlock, fromBlock, llvmModCtx.basicBlockMap.at(toBlock), actualFromBlock ? actualFromBlock : llvmModCtx.basicBlockMap.at(fromBlock));

        llvm::BasicBlock *actual_from_block_for_next = llvmModCtx.basicBlockMap.at(toBlock);

        for (const auto& instr : block.getIRArray()) {
            generateInstruction(llvmModCtx, instr, fromBlock, toBlock);
            if (llvmModCtx.Builder->GetInsertBlock()->getTerminator()) {
                actual_from_block_for_next = llvmModCtx.Builder->GetInsertBlock();
                break;
            }
        }

        llvmModCtx.valueStackPhi.finalizeNode();

        for (const auto& succ : llvmModCtx.controlFlowAnalysis.G[toBlock]) {
            // prepare the value stack for the next block
            generateCodeBlock(llvmModCtx, *llvmModCtx.currentFunctionDef->codeBlock[succ], toBlock, succ, actual_from_block_for_next);
        }
    }

    void LLVMCodegen::generateInstruction(LLVMModuleContext &llvmModCtx, const IR& instr, yoi::indexT fromBlock, yoi::indexT toBlock) {
        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            auto scope = llvmModCtx.currentFunction->getSubprogram();
            set_current_file_path(instr.debugInfo.sourceFile);
            llvmModCtx.Builder->SetCurrentDebugLocation(llvm::DILocation::get(*llvmModCtx.TheContext, instr.debugInfo.line + 1, instr.debugInfo.column + 1, scope));
            // insert call to runtime_debug_print extern func
            std::string debugStr = "Performing: " + yoi::wstring2string(instr.to_string());
            auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, debugStr, true);
            auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
            auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print"), llvm::ArrayRef<llvm::Value*>(debugArgs));
        }
        switch(instr.opcode) {
            case IR::Opcode::push_integer: {
                auto val = llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), instr.operands[0].value.integer, true);
                llvmModCtx.valueStackPhi.push_back({val, managedPtr(compilerCtx->getIntObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::push_decimal: {
                auto val = llvm::ConstantFP::get(llvmModCtx.Builder->getDoubleTy(), instr.operands[0].value.decimal);
                llvmModCtx.valueStackPhi.push_back({val, managedPtr(compilerCtx->getDeciObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::push_boolean: {
                auto val = llvm::ConstantInt::get(llvmModCtx.Builder->getInt1Ty(), instr.operands[0].value.boolean);
                llvmModCtx.valueStackPhi.push_back({val, managedPtr(compilerCtx->getBoolObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::push_short: {
                auto val = llvm::ConstantInt::get(llvmModCtx.Builder->getInt16Ty(), instr.operands[0].value.shortV);
                llvmModCtx.valueStackPhi.push_back({val, managedPtr(compilerCtx->getShortObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::push_unsigned: {
                auto val = llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), instr.operands[0].value.unsignedV, false);
                llvmModCtx.valueStackPhi.push_back({val, managedPtr(compilerCtx->getUnsignedObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::push_string: {
                auto& str = yoiModule->stringLiteralPool.getStringLiteral(instr.operands[1].value.stringLiteralIndex);
                // Create a global string literal for this string
                auto *literal = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, yoi::wstring2string(str), true);
                auto *globalStr = llvmModCtx.Builder->CreateGlobalString(wstring2string(str), "global_string_literal");
                llvmModCtx.valueStackPhi.push_back({globalStr, managedPtr(compilerCtx->getStrObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::push_character: {
                auto val = llvm::ConstantInt::get(llvmModCtx.Builder->getInt8Ty(), instr.operands[0].value.character);
                llvmModCtx.valueStackPhi.push_back({val, managedPtr(compilerCtx->getCharObjectType()->getBasicRawType())});
                break;
            }
            // Basic Type Casting
            case IR::Opcode::basic_cast_char: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                llvm::Value* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isDoubleTy()) {
                    castedVal = llvmModCtx.Builder->CreateFPToSI(rawVal, llvmModCtx.Builder->getInt8Ty(), "deci_to_char_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = llvmModCtx.Builder->CreateTrunc(rawVal, llvmModCtx.Builder->getInt8Ty(), "bool_to_char_cast");
                } else if (rawVal->getType()->isIntegerTy(64)) { // int (no-op)
                    castedVal = llvmModCtx.Builder->CreateTrunc(rawVal, llvmModCtx.Builder->getInt8Ty(), "int_to_char_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char (no-op)
                    castedVal = rawVal;
                } else {
                    panic(instr.debugInfo.line, instr.debugInfo.column, "LLVM Codegen: Unsupported type for basic_cast_char");
                }

                llvmModCtx.valueStackPhi.push_back({castedVal, managedPtr(compilerCtx->getCharObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_int: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                llvm::Value* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isDoubleTy()) {
                    castedVal = llvmModCtx.Builder->CreateFPToSI(rawVal, llvmModCtx.Builder->getInt64Ty(), "deci_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "bool_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "char_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(16)) { // short
                    castedVal = llvmModCtx.Builder->CreateSExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "short_to_int_cast");
                } else if (rawVal->getType()->isIntegerTy(64)) { // int (no-op)
                    castedVal = rawVal;
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_int");
                }

                llvmModCtx.valueStackPhi.push_back({castedVal, managedPtr(compilerCtx->getIntObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_deci: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                llvm::Value* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isIntegerTy(64)) { // int
                    castedVal = llvmModCtx.Builder->CreateSIToFP(rawVal, llvmModCtx.Builder->getDoubleTy(), "int_to_deci_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = llvmModCtx.Builder->CreateUIToFP(rawVal, llvmModCtx.Builder->getDoubleTy(), "bool_to_deci_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = llvmModCtx.Builder->CreateUIToFP(rawVal, llvmModCtx.Builder->getDoubleTy(), "char_to_deci_cast");
                } else if (rawVal->getType()->isIntegerTy(16)) { // short
                    castedVal = llvmModCtx.Builder->CreateSIToFP(rawVal, llvmModCtx.Builder->getDoubleTy(), "short_to_deci_cast");
                } else if (rawVal->getType()->isDoubleTy()) { // deci (no-op)
                    castedVal = rawVal;
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_deci");
                }

                llvmModCtx.valueStackPhi.push_back({castedVal, managedPtr(compilerCtx->getDeciObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_unsigned: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                llvm::Value* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isIntegerTy(64)) { // int
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "int_to_unsigned_cast");
                } else if (rawVal->getType()->isDoubleTy()) { // deci
                    castedVal = llvmModCtx.Builder->CreateFPToUI(rawVal, llvmModCtx.Builder->getInt64Ty(), "deci_to_unsigned_cast");
                } else if (rawVal->getType()->isIntegerTy(16)) { // short
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "short_to_unsigned_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "char_to_unsigned_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt64Ty(), "bool_to_unsigned_cast");
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_unsigned");
                }

                llvmModCtx.valueStackPhi.push_back({castedVal, managedPtr(compilerCtx->getUnsignedObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_short: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                llvm::Value* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isIntegerTy(64)) { // int
                    castedVal = llvmModCtx.Builder->CreateTrunc(rawVal, llvmModCtx.Builder->getInt16Ty(), "int_to_short_cast");
                } else if (rawVal->getType()->isDoubleTy()) { // deci
                    castedVal = llvmModCtx.Builder->CreateFPToSI(rawVal, llvmModCtx.Builder->getInt16Ty(), "deci_to_short_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt16Ty(), "char_to_short_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool
                    castedVal = llvmModCtx.Builder->CreateZExt(rawVal, llvmModCtx.Builder->getInt16Ty(), "bool_to_short_cast");
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_short");
                }

                llvmModCtx.valueStackPhi.push_back({castedVal, managedPtr(compilerCtx->getShortObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::basic_cast_bool: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                llvm::Value* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                llvm::Value* castedVal = nullptr;

                if (rawVal->getType()->isIntegerTy(64)) { // int
                    castedVal = llvmModCtx.Builder->CreateICmpNE(rawVal, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 0), "int_to_bool_cast");
                } else if (rawVal->getType()->isDoubleTy()) { // deci
                    castedVal = llvmModCtx.Builder->CreateFCmpONE(rawVal, llvm::ConstantFP::get(llvmModCtx.Builder->getDoubleTy(), 0.0), "deci_to_bool_cast");
                } else if (rawVal->getType()->isIntegerTy(16)) { // short
                    castedVal = llvmModCtx.Builder->CreateICmpNE(rawVal, llvm::ConstantInt::get(llvmModCtx.Builder->getInt8Ty(), 0), "short_to_bool_cast");
                } else if (rawVal->getType()->isIntegerTy(8)) { // char
                    castedVal = llvmModCtx.Builder->CreateICmpNE(rawVal, llvm::ConstantInt::get(llvmModCtx.Builder->getInt8Ty(), 0), "char_to_bool_cast");
                } else if (rawVal->getType()->isIntegerTy(1)) { // bool (no-op)
                    castedVal = rawVal;
                } else {
                    panic(0, 0, "LLVM Codegen: Unsupported type for basic_cast_bool");
                }

                llvmModCtx.valueStackPhi.push_back({castedVal, managedPtr(compilerCtx->getBoolObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            // Arithmetic
            case IR::Opcode::add: handleBinaryOp(llvmModCtx, llvm::Instruction::Add, false, fromBlock, toBlock); break;
            case IR::Opcode::sub: handleBinaryOp(llvmModCtx, llvm::Instruction::Sub, false, fromBlock, toBlock); break;
            case IR::Opcode::mul: handleBinaryOp(llvmModCtx, llvm::Instruction::Mul, false, fromBlock, toBlock); break;
            case IR::Opcode::div: handleBinaryOp(llvmModCtx, llvm::Instruction::SDiv, false, fromBlock, toBlock); break;
            case IR::Opcode::mod: handleBinaryOp(llvmModCtx, llvm::Instruction::SRem, false, fromBlock, toBlock); break;
            case IR::Opcode::bitwise_and: handleBinaryOp(llvmModCtx, llvm::Instruction::And, false, fromBlock, toBlock); break;
            case IR::Opcode::bitwise_or: handleBinaryOp(llvmModCtx, llvm::Instruction::Or, false, fromBlock, toBlock); break;
            case IR::Opcode::bitwise_xor: handleBinaryOp(llvmModCtx, llvm::Instruction::Xor, false, fromBlock, toBlock); break;
            case IR::Opcode::left_shift: handleBinaryOp(llvmModCtx, llvm::Instruction::Shl, false, fromBlock, toBlock); break;
            case IR::Opcode::right_shift: handleBinaryOp(llvmModCtx, llvm::Instruction::LShr, false, fromBlock, toBlock); break;
            // Unary
            case IR::Opcode::negate: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                auto* negatedRaw = rawVal->getType()->isDoubleTy() ? llvmModCtx.Builder->CreateFNeg(rawVal, "negtmp") : llvmModCtx.Builder->CreateNeg(rawVal, "negtmp");
                // llvmModCtx.valueStackPhi.push_back({resultObj, val.yoiType});
                llvmModCtx.valueStackPhi.push_back({negatedRaw, managedPtr(val.yoiType->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }
            case IR::Opcode::bitwise_not: {
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto* rawVal = unboxValue(llvmModCtx, val.llvmValue, val.yoiType);
                auto* notRaw = llvmModCtx.Builder->CreateNot(rawVal, "nottmp");
                // auto* resultObj = createBasicObject(llvmModCtx, val.yoiType, notRaw);
                llvmModCtx.valueStackPhi.push_back({notRaw, managedPtr(val.yoiType->getBasicRawType())});
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false); // Consume operand
                break;
            }

            // Comparison
            case IR::Opcode::equal: handleComparison(llvmModCtx, llvm::CmpInst::ICMP_EQ, false, fromBlock, toBlock); break;
            case IR::Opcode::not_equal: handleComparison(llvmModCtx, llvm::CmpInst::ICMP_NE, false, fromBlock, toBlock); break;
            case IR::Opcode::less_than: handleComparison(llvmModCtx, llvm::CmpInst::ICMP_SLT, false, fromBlock, toBlock); break;
            case IR::Opcode::less_equal: handleComparison(llvmModCtx, llvm::CmpInst::ICMP_SLE, false, fromBlock, toBlock); break;
            case IR::Opcode::greater_than: handleComparison(llvmModCtx, llvm::CmpInst::ICMP_SGT, false, fromBlock, toBlock); break;
            case IR::Opcode::greater_equal: handleComparison(llvmModCtx, llvm::CmpInst::ICMP_SGE, false, fromBlock, toBlock); break;

            // Memory
            case IR::Opcode::load_local: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* alloca = llvmModCtx.namedValues.at(varIndex);
                auto yoiType = llvmModCtx.currentFunctionDef->variableTable.get(varIndex);
                auto loadedPtr = llvmModCtx.Builder->CreateLoad(yoiTypeToLLVMType(llvmModCtx, yoiType, yoiType->isBasicRawType() || yoiType->hasAttribute(IRValueType::ValueAttr::Raw)), alloca, "loadtmp");
                callGcFunction(llvmModCtx, loadedPtr, yoiType, true);
                llvmModCtx.valueStackPhi.push_back({loadedPtr, yoiType});
                break;
            }
            case IR::Opcode::store_local: {
                auto varIndex = instr.operands[0].value.symbolIndex;
                auto* alloca = llvmModCtx.namedValues.at(varIndex);
                auto yoiType = llvmModCtx.currentFunctionDef->variableTable.get(varIndex);
                auto valToStore = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                valToStore = yoiType->metadata.hasMetadata(L"regressed_interface_impl") ? valToStore : promiseInterfaceObjectIfInterface(llvmModCtx, valToStore);

                // Release old value
                auto* oldPtr = llvmModCtx.Builder->CreateLoad(alloca->getAllocatedType(), alloca, "old_ptr_for_store");
                if (yoiType->hasAttribute(IRValueType::ValueAttr::Nullable))
                    callGcFunction(llvmModCtx, oldPtr, yoiType, false, true);
                else
                    generateIfTargetNotNull(llvmModCtx, oldPtr, yoiType, [&] () {
                        callGcFunction(llvmModCtx, oldPtr, yoiType, false, true);
                    }, !yoiType->hasAttribute(IRValueType::ValueAttr::Raw));
                // Store new value
                if (llvmModCtx.currentFunctionDef->variableTable.get(varIndex)->hasAttribute(IRValueType::ValueAttr::Raw)) {
                    auto unboxedVal = unboxValue(llvmModCtx, valToStore.llvmValue, valToStore.yoiType);
                    llvmModCtx.Builder->CreateStore(unboxedVal, alloca);
                } else {
                    auto object = ensureObject(llvmModCtx, valToStore.yoiType, valToStore.llvmValue);
                    if (object.first->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                        callGcFunction(llvmModCtx, object.second, valToStore.yoiType, true, true, true);
                    llvmModCtx.Builder->CreateStore(object.second, alloca);
                }
                
                break;
            }
            case IR::Opcode::load_global: {
                auto varIndex = instr.operands[1].value.symbolIndex;
                auto* global = llvmModCtx.globalValues.at(varIndex);
                auto yoiType = yoiModule ->globalVariables[varIndex];
                yoiType->addAttribute(IRValueType::ValueAttr::Nullable).addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope);
                auto loadedPtr = llvmModCtx.Builder->CreateLoad(global->getValueType(), global, "loadglobaltmp");
                llvmModCtx.valueStackPhi.push_back({loadedPtr, yoiType});
                break;
            }
            case IR::Opcode::store_global: {
                auto varIndex = instr.operands[1].value.symbolIndex;
                auto* global = llvmModCtx.globalValues.at(varIndex);
                auto yoiType = yoiModule->globalVariables[varIndex];
                auto valToStore = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                valToStore = promiseInterfaceObjectIfInterface(llvmModCtx, valToStore);

                yoiType->addAttribute(IRValueType::ValueAttr::Nullable);

                auto* oldPtr = llvmModCtx.Builder->CreateLoad(global->getValueType(), global, "old_global_ptr");
                callGcFunction(llvmModCtx, oldPtr, yoiType, false, true);

                auto object = ensureObject(llvmModCtx, valToStore.yoiType, valToStore.llvmValue);
                if (object.first->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                    callGcFunction(llvmModCtx, object.second, valToStore.yoiType, true, true, true);

                llvmModCtx.Builder->CreateStore(object.second, global);
                break;
            }
            case IR::Opcode::load_member: {
                auto structVal = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto memberIndex = instr.operands[0].value.symbolIndex;
                auto llvmMemberIndex = memberIndex + 2; // +2 to skip gc_refcount header and type index

                auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
                auto* llvmStructType = llvmModCtx.structTypeMap.at(key);
                auto* gep = llvmModCtx.Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, llvmMemberIndex, "memberptr");

                auto yoiStructDef = compilerCtx->getIRObjectFile()->compiledModule->structTable[std::get<2>(key)];
                auto memberYoiType = yoiStructDef->fieldTypes[memberIndex];
                if (structVal.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                    memberYoiType = managedPtr(IRValueType{*memberYoiType}.addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope));
                memberYoiType->addAttribute(IRValueType::ValueAttr::Nullable);
                memberYoiType->removeAttribute(IRValueType::ValueAttr::Raw); // workaround for incorrect optimization labelling

                llvm::Type* loadedType = yoiTypeToLLVMType(llvmModCtx, memberYoiType);
                auto* loadedMember = llvmModCtx.Builder->CreateLoad(loadedType, gep, "loadmember");
                callGcFunction(llvmModCtx, loadedMember, memberYoiType, true); // Create new reference for the loaded member
                llvmModCtx.valueStackPhi.push_back({loadedMember, memberYoiType});

                callGcFunction(llvmModCtx, structVal.llvmValue, structVal.yoiType, false); // Consume the struct reference from the stack
                break;
            }
            case IR::Opcode::store_member: {
                auto structVal = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto valueToStore = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                valueToStore = promiseInterfaceObjectIfInterface(llvmModCtx, valueToStore);

                auto memberIndex = instr.operands[0].value.symbolIndex;
                auto llvmMemberIndex = memberIndex + 2; // +2 to skip gc_refcount header and type index

                auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
                auto* llvmStructType = llvmModCtx.structTypeMap.at(key);
                auto* gep = llvmModCtx.Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, llvmMemberIndex, "memberptr");

                auto yoiStructDef = compilerCtx->getIRObjectFile()->compiledModule->structTable[std::get<2>(key)];
                auto memberYoiType = yoiStructDef->fieldTypes[memberIndex];
                memberYoiType->addAttribute(IRValueType::ValueAttr::Nullable);

                auto* oldMemberPtr = llvmModCtx.Builder->CreateLoad(yoiTypeToLLVMType(llvmModCtx, memberYoiType), gep, "old_member_ptr");
                callGcFunction(llvmModCtx, oldMemberPtr, memberYoiType, false, true);

                auto object = ensureObject(llvmModCtx, valueToStore.yoiType, valueToStore.llvmValue);
                if (object.first->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !valueToStore.yoiType->hasAttribute(IRValueType::ValueAttr::Raw))
                    callGcFunction(llvmModCtx, object.second, valueToStore.yoiType, true, true, true);

                llvmModCtx.Builder->CreateStore(object.second, gep);

                callGcFunction(llvmModCtx, structVal.llvmValue, structVal.yoiType, false);
                break;
            }

            // Control Flow
            case IR::Opcode::jump: {
                llvmModCtx.Builder->CreateBr(llvmModCtx.basicBlockMap.at(instr.operands[0].value.codeBlockIndex));
                break;
            }
            case IR::Opcode::jump_if_true:
            case IR::Opcode::jump_if_false: {
                auto condObj = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto* condRaw = unboxValue(llvmModCtx, condObj.llvmValue, condObj.yoiType);
                callGcFunction(llvmModCtx, condObj.llvmValue, condObj.yoiType, false);

                auto* destBlock = llvmModCtx.basicBlockMap.at(instr.operands[0].value.codeBlockIndex);
                auto* nextBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "fallthrough", llvmModCtx.currentFunction);

                if (instr.opcode == IR::Opcode::jump_if_true) {
                    llvmModCtx.Builder->CreateCondBr(condRaw, destBlock, nextBlock);
                } else { // jump_if_false
                    llvmModCtx.Builder->CreateCondBr(condRaw, nextBlock, destBlock);
                }
                llvmModCtx.Builder->SetInsertPoint(nextBlock);
                break;
            }

            case IR::Opcode::ret: {
                auto retVal = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                    std::string funcName = wstring2string(llvmModCtx.currentFunctionDef->name);
                    auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, funcName, true);
                    auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                    auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                    llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_report_leave_function"), llvm::ArrayRef<llvm::Value*>(debugArgs));
                }

                retVal = promiseInterfaceObjectIfInterface(llvmModCtx, retVal);

                // The caller receives ownership, so we don't decrease the ref count here.
                if (llvmModCtx.currentFunctionDef->returnType->hasAttribute(IRValueType::ValueAttr::Raw)) {
                    auto res = unboxValue(llvmModCtx, retVal.llvmValue, retVal.yoiType);
                    // call gc function for the return value
                    callGcFunction(llvmModCtx, retVal.llvmValue, retVal.yoiType, false);
                    generateFunctionExitCleanup(llvmModCtx);
                    llvmModCtx.Builder->CreateRet(res);
                } else {
                    auto object = ensureObject(llvmModCtx, retVal.yoiType, retVal.llvmValue);
                    if (object.first->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                        callGcFunction(llvmModCtx, object.second, retVal.yoiType, true, true, true);
                    generateFunctionExitCleanup(llvmModCtx);
                    llvmModCtx.Builder->CreateRet(object.second);
                }
                break;
            }
            case IR::Opcode::ret_none: {
                generateFunctionExitCleanup(llvmModCtx);

                if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                    std::string funcName = wstring2string(llvmModCtx.currentFunctionDef->name);
                    auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, funcName, true);
                    auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                    auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                    llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_report_leave_function"), llvm::ArrayRef<llvm::Value*>(debugArgs));
                }

                llvmModCtx.Builder->CreateRetVoid();
                break;
            }
            // Functions
            case IR::Opcode::invoke: {
                auto moduleIndex = instr.operands[0].value.symbolIndex;
                auto funcIndex = instr.operands[1].value.symbolIndex;
                auto argCount = instr.operands[2].value.symbolIndex;

                auto funcDef = yoiModule->functionTable[funcIndex];
                auto* function = llvmModCtx.functionMap.at(funcDef->name);

                std::vector<llvm::Value*> args;
                std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Value*>> postCleanup;

                for(size_t i = 0; i < argCount; ++i) {
                    auto arg = llvmModCtx.valueStackPhi.back();
                    llvmModCtx.valueStackPhi.pop_back();

                    arg = promiseInterfaceObjectIfInterface(llvmModCtx, arg);

                    if (funcDef->argumentTypes[argCount - i - 1]->hasAttribute(IRValueType::ValueAttr::Raw)) {
                        args.push_back(unboxValue(llvmModCtx, arg.llvmValue, arg.yoiType));
                        callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, false);
                    } else if (funcDef->argumentTypes[argCount - i - 1]->hasAttribute(IRValueType::ValueAttr::Borrow)) {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        args.push_back(object.second);
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !arg.yoiType->hasAttribute(IRValueType::ValueAttr::Raw));
                        else postCleanup.push_back(object);
                    } else {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        args.push_back(object.second);
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !arg.yoiType->hasAttribute(IRValueType::ValueAttr::Raw))
                            callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, true, true);
                        else;
                    }
                }
                std::reverse(args.begin(), args.end());

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    llvmModCtx.Builder->CreateCall(function, args);
                } else {
                    auto* call = llvmModCtx.Builder->CreateCall(function, args, "calltmp");
                    // The returned value comes with a reference count for us to own.
                    llvmModCtx.valueStackPhi.push_back({call, funcDef->returnType});
                }

                for (auto &i : postCleanup) {
                    callGcFunction(llvmModCtx, i.second, i.first, false);
                }
                break;
            }
            case IR::Opcode::invoke_dangling: {
                auto moduleIndex = instr.operands[0].value.symbolIndex;
                auto funcIndex = instr.operands[1].value.symbolIndex;
                auto argCount = instr.operands[2].value.symbolIndex;

                yoi_assert(argCount, instr.debugInfo.line, instr.debugInfo.column, "invoke_dangling with no arguments");

                auto funcDef = yoiModule->functionTable[funcIndex];
                auto* function = llvmModCtx.functionMap.at(funcDef->name);

                std::vector<llvm::Value*> args;
                std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Value*>> postCleanup;

                llvm::Value *postponed = nullptr;
                {
                    auto arg = llvmModCtx.valueStackPhi.back();
                    llvmModCtx.valueStackPhi.pop_back();

                    arg = promiseInterfaceObjectIfInterface(llvmModCtx, arg);

                    if (funcDef->argumentTypes[0]->hasAttribute(IRValueType::ValueAttr::Raw)) {
                        postponed = unboxValue(llvmModCtx, arg.llvmValue, arg.yoiType);
                        callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, false);
                    } else if (funcDef->argumentTypes[0]->hasAttribute(IRValueType::ValueAttr::Borrow)) {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        postponed = object.second;
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !arg.yoiType->hasAttribute(IRValueType::ValueAttr::Raw));
                        else postCleanup.push_back(object);
                    } else {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        postponed = object.second;
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !arg.yoiType->hasAttribute(IRValueType::ValueAttr::Raw))
                            callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, true, true);
                        else;
                    }
                }

                for(size_t i = 1; i < argCount; ++i) {
                    auto arg = llvmModCtx.valueStackPhi.back();
                    llvmModCtx.valueStackPhi.pop_back();

                    arg = promiseInterfaceObjectIfInterface(llvmModCtx, arg);

                    if (funcDef->argumentTypes[argCount - i]->hasAttribute(IRValueType::ValueAttr::Raw)) {
                        args.push_back(unboxValue(llvmModCtx, arg.llvmValue, arg.yoiType));
                        callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, false);
                    } else if (funcDef->argumentTypes[argCount - i]->hasAttribute(IRValueType::ValueAttr::Borrow)) {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        args.push_back(object.second);
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !arg.yoiType->hasAttribute(IRValueType::ValueAttr::Raw));
                        else postCleanup.push_back(object);
                    } else {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        args.push_back(object.second);
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !arg.yoiType->hasAttribute(IRValueType::ValueAttr::Raw))
                            callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, true, true);
                        else;
                    }
                }

                args.push_back(postponed);

                std::reverse(args.begin(), args.end());

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    llvmModCtx.Builder->CreateCall(function, args);
                } else {
                    auto* call = llvmModCtx.Builder->CreateCall(function, args, "calltmp");
                    // The returned value comes with a reference count for us to own.
                    llvmModCtx.valueStackPhi.push_back({call, funcDef->returnType});
                }

                for (auto &i : postCleanup) {
                    callGcFunction(llvmModCtx, i.second, i.first, false);
                }
                break;
            }
            case IR::Opcode::invoke_imported: {
                auto libIndex = instr.operands[0].value.symbolIndex;
                auto funcIndex = instr.operands[1].value.symbolIndex;
                auto argCount = instr.operands[2].value.symbolIndex;

                auto funcDef = compilerCtx->getIRFFITable()->importedLibraries[libIndex].importedFunctionTable[funcIndex];

                if (funcDef->hasAttribute(IRFunctionDefinition::FunctionAttrs::Intrinsic)) {
                    handleIntrinsicCall(llvmModCtx, instr);
                    break;
                }

                bool noffi = funcDef->hasAttribute(IRFunctionDefinition::FunctionAttrs::NoFFI);

                auto rawFuncName = compilerCtx->getIRFFITable()->importedLibraries[libIndex].importedFunctionTable.getKey(funcIndex);
                auto mangledFuncName = L"imported#" + std::to_wstring(libIndex) + L"#" + rawFuncName;
                if (!noffi) mangledFuncName += L"#wrapper";

                auto* function = llvmModCtx.functionMap.at(mangledFuncName);

                std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Value*>> postCleanup;
                std::vector<llvm::Value*> args;

                for(size_t i = 0; i < argCount; ++i) {
                    auto arg = llvmModCtx.valueStackPhi.back();
                    llvmModCtx.valueStackPhi.pop_back();
                    arg = promiseInterfaceObjectIfInterface(llvmModCtx, arg);
                    
                    if ((arg.yoiType->isBasicType() || arg.yoiType->isBasicRawType()) && !noffi) {
                        auto *param = unboxValue(llvmModCtx, arg.llvmValue, arg.yoiType);
                        if (arg.yoiType->type == IRValueType::valueType::stringObject || arg.yoiType->type == IRValueType::valueType::stringLiteral) {
                            // the only fucking pointer that needs special handling here
                            // we convert it to a int64 while passing it to the imported function
                            param = llvmModCtx.Builder->CreatePtrToInt(param, llvm::Type::getInt64Ty(*llvmModCtx.TheContext), "string_to_int");
                        }
                        // clean up the mess immediately
                        callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, false);
                        args.push_back(param);
                    } else {
                        auto object = ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue);
                        postCleanup.push_back(object);
                        if (arg.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && noffi) // retain the value for no ffi calls to prevent being destoryed
                            callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, true, true, true);
                        args.push_back(postCleanup.back().second);
                    }
                    // Callee will retain, so we release the stack's reference
                    // callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, false);
                }
                std::reverse(args.begin(), args.end());

                if (funcDef->returnType->type == IRValueType::valueType::none) {
                    llvmModCtx.Builder->CreateCall(function, args);
                } else {
                    auto* call = llvmModCtx.Builder->CreateCall(function, args, "calltmp");
                    // The returned value comes with a reference count for us to own.
                    // llvmModCtx.valueStackPhi.push_back({call, funcDef->returnType});
                    auto onstackType = noffi ? *funcDef->returnType : compilerCtx->normalizeForeignBasicType(funcDef->returnType);

                    if (noffi && (funcDef->returnType->type == IRValueType::valueType::structObject ||
                                  funcDef->returnType->type == IRValueType::valueType::interfaceObject)) {
                        // audit the type id before pushing to stack
                        auto notNullBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "audit_typeid_not_null", llvmModCtx.currentFunction);
                        auto continueBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "audit_typeid_continue", llvmModCtx.currentFunction);
                        llvmModCtx.Builder->CreateCondBr(llvmModCtx.Builder->CreateIsNotNull(call, "audit_typeid_isnotnull"), notNullBlock, continueBlock);
                        llvmModCtx.Builder->SetInsertPoint(notNullBlock);
                        auto typeIdKey = std::make_tuple(IRValueType::valueType::structObject,
                                                         ENTRY_MODULE_ID_CONST,
                                                         funcDef->returnType->typeIndex,
                                                         0);
                        auto structKey = std::make_tuple(IRValueType::valueType::structObject,
                                                         ENTRY_MODULE_ID_CONST,
                                                         funcDef->returnType->typeIndex);
                        auto typeId = llvmModCtx.typeIDMap[typeIdKey];
                        auto llvmStruct = llvmModCtx.structTypeMap.at(structKey);
                        auto *typeIdPtr = llvmModCtx.Builder->CreateStructGEP(llvmStruct, call, 1, "typeid_ptr");
                        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), typeId), typeIdPtr);
                        llvmModCtx.Builder->CreateBr(continueBlock);
                        llvmModCtx.Builder->SetInsertPoint(continueBlock);
                    }

                    llvmModCtx.valueStackPhi.push_back({call, managedPtr(onstackType.isBasicType() && !noffi ? onstackType.getBasicRawType() : onstackType)});
                }

                if (!noffi) {
                    for (auto &i : postCleanup) {
                        callGcFunction(llvmModCtx, i.second, i.first, false);
                    }
                }
                break;
            }
            case IR::Opcode::new_struct: {
                auto moduleIndex = instr.operands[0].value.symbolIndex;
                auto structIndex = instr.operands[1].value.symbolIndex;
                auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
                auto* structType = llvmModCtx.structTypeMap.at(key);

                auto size = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(structType);
                auto* sizeVal = llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), size);

                auto* allocCall = llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"object_alloc"), sizeVal, "newtmp_alloc");
                auto* bitcast = llvmModCtx.Builder->CreateBitCast(allocCall, llvm::PointerType::get(structType, 0), "casttmp");

                auto* refCountPtr = llvmModCtx.Builder->CreateStructGEP(structType, bitcast, 0, "refcount_ptr");
                llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), refCountPtr);

                auto* typeIdPtr = llvmModCtx.Builder->CreateStructGEP(structType, bitcast, 1, "typeid_ptr");
                auto typeIdKey = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structIndex, 0);
                llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), llvmModCtx.typeIDMap[typeIdKey]), typeIdPtr);


                auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
                llvmModCtx.valueStackPhi.push_back({bitcast, yoiType});
                break;
            }
            case IR::Opcode::construct_interface_impl: {
                auto interfaceImplIndex = instr.operands[1].value.symbolIndex;
                auto interfaceImplDef = yoiModule->interfaceImplementationTable[interfaceImplIndex];
                auto &top = llvmModCtx.valueStackPhi.back();
                top = promiseInterfaceObjectIfInterface(llvmModCtx, top);
                auto object = ensureObject(llvmModCtx, top.yoiType, top.llvmValue);
                top.yoiType = object.first;
                top.llvmValue = object.second;

                // prevent bugs for reusing the IRValueType
                top.yoiType = managedPtr(*top.yoiType);
                top.yoiType->type = IRValueType::valueType::interfaceObject;
                top.yoiType->typeAffiliateModule = ENTRY_MODULE_ID_CONST;
                top.yoiType->typeIndex = interfaceImplDef->implInterfaceIndex.second;
                top.yoiType->metadata.setMetadata(L"regressed_interface_impl", std::pair<yoi::indexT, yoi::indexT>{ENTRY_MODULE_ID_CONST, interfaceImplIndex});
                break;
            }
            case IR::Opcode::bind_elements_post:
            case IR::Opcode::bind_elements_pred: {
                auto array = llvmModCtx.valueStackPhi.back();
                llvmModCtx.valueStackPhi.pop_back();
                
                yoi_assert(array.yoiType->isArrayType() || array.yoiType->isDynamicArrayType(), instr.debugInfo.line, instr.debugInfo.column, "Expected array type for bind_elements");

                auto arrayLLVMType = getArrayLLVMType(llvmModCtx, array.yoiType);
                // gep index 2
                auto *arrayLen = llvmModCtx.Builder->CreateStructGEP(arrayLLVMType, array.llvmValue, 2, "array_len");
                auto *loadedArrayLen = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), arrayLen, "loaded_array_len");

                auto startPos = instr.opcode == IR::Opcode::bind_elements_post ? llvmModCtx.Builder->getInt64(0) : llvmModCtx.Builder->CreateSub(loadedArrayLen, llvmModCtx.Builder->getInt64(instr.operands[0].value.symbolIndex), "start_pos");

                for (yoi::indexT i = 0;i < instr.operands[0].value.symbolIndex;i++) {
                    auto currentPos = llvmModCtx.Builder->CreateAdd(startPos, llvmModCtx.Builder->getInt64(i), "current_pos");
                    auto currentValue = loadArrayElement(llvmModCtx, array.yoiType, array.llvmValue, currentPos);
                    std::shared_ptr<IRValueType> elementType;
                    if (array.yoiType->isBasicType()) {
                        elementType = managedPtr(array.yoiType->getElementType().getBasicRawType());
                    } else if (array.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope)) {
                        elementType = managedPtr(array.yoiType->getElementType().addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope).addAttribute(IRValueType::ValueAttr::Nullable));
                    } else {
                        elementType = managedPtr(array.yoiType->getElementType().addAttribute(IRValueType::ValueAttr::Nullable));
                    }
                    llvmModCtx.valueStackPhi.push_back({currentValue, elementType});
                }

                callGcFunction(llvmModCtx, array.llvmValue, array.yoiType, false); // Release the reference to the array
                break;
            }
            case IR::Opcode::bind_fields_post:
            case IR::Opcode::bind_fields_pred: {
                auto structVal = llvmModCtx.valueStackPhi.back();
                llvmModCtx.valueStackPhi.pop_back();
                
                yoi_assert(structVal.yoiType->type == IRValueType::valueType::structObject, instr.debugInfo.line, instr.debugInfo.column, "Expected struct type for bind_values");
                auto structDef = yoiModule->structTable[structVal.yoiType->typeIndex];

                auto startPos = instr.opcode == IR::Opcode::bind_fields_post ? 0 : structDef->fieldTypes.size() - instr.operands[0].value.symbolIndex;
                for (yoi::indexT memberIndex = startPos; memberIndex < startPos + instr.operands[0].value.symbolIndex; memberIndex++) {
                    auto llvmMemberIndex = memberIndex + 2; // +2 to skip gc_refcount header and type index

                    auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
                    auto* llvmStructType = llvmModCtx.structTypeMap.at(key);
                    auto* gep = llvmModCtx.Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, llvmMemberIndex, "memberptr");

                    auto yoiStructDef = compilerCtx->getIRObjectFile()->compiledModule->structTable[std::get<2>(key)];
                    auto memberYoiType = yoiStructDef->fieldTypes[memberIndex];
                    if (structVal.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                        memberYoiType = managedPtr(IRValueType{*memberYoiType}.addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope));
                    memberYoiType->addAttribute(IRValueType::ValueAttr::Nullable);
                    memberYoiType->removeAttribute(IRValueType::ValueAttr::Raw); // workaround for incorrect optimization labelling

                    llvm::Type* loadedType = yoiTypeToLLVMType(llvmModCtx, memberYoiType);
                    auto* loadedMember = llvmModCtx.Builder->CreateLoad(loadedType, gep, "loadmember");
                    callGcFunction(llvmModCtx, loadedMember, memberYoiType, true); // Create new reference for the loaded member
                    llvmModCtx.valueStackPhi.push_back({loadedMember, memberYoiType});
                }

                callGcFunction(llvmModCtx, structVal.llvmValue, structVal.yoiType, false); // Release the reference to the struct
                break;
            }
            case IR::Opcode::invoke_virtual: {
                auto methodVTableIndex = instr.operands[2].value.symbolIndex;
                auto userArgCount = instr.operands[3].value.symbolIndex;

                std::vector<StackValue> userArgs;
                std::vector<std::pair<std::shared_ptr<IRValueType>, llvm::Value*>> postCleanup;

                for (size_t i = 0; i < userArgCount - 1; ++i) { // userArgCount includes 'this'
                    userArgs.push_back(promiseInterfaceObjectIfInterface(llvmModCtx, llvmModCtx.valueStackPhi.back()));
                    llvmModCtx.valueStackPhi.pop_back();
                }
                std::reverse(userArgs.begin(), userArgs.end());

                auto interfaceShellVal = llvmModCtx.valueStackPhi.back();
                llvmModCtx.valueStackPhi.pop_back();

                auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject, interfaceShellVal.yoiType->typeAffiliateModule, interfaceShellVal.yoiType->typeIndex);
                auto* interfaceLLVMType = llvmModCtx.structTypeMap.at(interfaceKey);
                auto interfaceDef = yoiModule->interfaceTable[std::get<2>(interfaceKey)];

                // Load the concrete `this` pointer from index 2
                auto concreteThisPtrRaw = unwrapInterfaceObject(llvmModCtx, interfaceShellVal);
                auto* bitcastedPointer = llvmModCtx.Builder->CreateBitCast(concreteThisPtrRaw, llvm::PointerType::get(llvmModCtx.Builder->getInt64Ty(), 0), "casted_this");
                // increase the reference count of this pointer, so that when leaving the function, it won't be collected
                // llvmModCtx.Builder->CreateStore(llvmModCtx.Builder->CreateAdd(oldRefcount, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1)), bitcastedPointer);
                // llvmModCtx.Builder->CreateAtomicRMW(llvm::AtomicRMWInst::BinOp::Add, bitcastedPointer, llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), llvm::MaybeAlign(8), llvm::AtomicOrdering::Monotonic);
                // for now, we pass the value by borrow, this stmt is no longer needed.

                // btw, we have increased the refcount of the interface as well before, so when we finish the invoking, we need to decrease it.

                // Load the function pointer to call from the v-table. User methods start at index 5.

                std::vector<llvm::Value*> finalArgs;
                finalArgs.push_back(concreteThisPtrRaw);
                for(yoi::indexT paramIndex = 0; paramIndex < userArgs.size(); ++paramIndex) {
                    const auto& arg = userArgs[paramIndex];
                    auto paramDef = interfaceDef->methodMap[methodVTableIndex]->argumentTypes[paramIndex];
                    auto object = (paramDef->hasAttribute(IRValueType::ValueAttr::Nullable) || (!paramDef->isBasicType() && !paramDef->isBasicRawType()) || !paramDef->dimensions.empty()) 
                        ? ensureObject(llvmModCtx, arg.yoiType, arg.llvmValue) 
                        : std::pair{managedPtr(arg.yoiType->getBasicRawType()), unboxValue(llvmModCtx, arg.llvmValue, arg.yoiType)};
                    finalArgs.push_back(object.second);
                    // default to borrow
                    if (object.first->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !object.first->hasAttribute(IRValueType::ValueAttr::Raw));
                        // callGcFunction(llvmModCtx, arg.llvmValue, arg.yoiType, true, true);
                    else postCleanup.emplace_back(object);
                }
                
                std::shared_ptr<IRFunctionDefinition> methodDef;
                llvm::Value *funcPtrToCall = nullptr;
                llvm::FunctionType *virtualFuncType = nullptr;
                if (interfaceShellVal.yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
                    auto interfaceImplIndex = interfaceShellVal.yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                    auto interfaceImplDef = yoiModule->interfaceImplementationTable[interfaceImplIndex.second];
                    methodDef = yoiModule->functionTable[interfaceImplDef->virtualMethods[methodVTableIndex]->typeIndex];
                    funcPtrToCall = llvmModCtx.functionMap[methodDef->name];
                    virtualFuncType = llvmModCtx.functionMap[methodDef->name]->getFunctionType();
                } else {
                    auto vtableSlotIndex = methodVTableIndex + 5;
                    auto* vtableSlotPtr = llvmModCtx.Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, vtableSlotIndex, "vtable_slot_ptr");

                    auto interfaceDef = compilerCtx->getIRObjectFile()->compiledModule->interfaceTable[std::get<2>(interfaceKey)];
                    methodDef = interfaceDef->methodMap[methodVTableIndex];
                    auto* funcType = getFunctionType(llvmModCtx, methodDef);

                    std::vector<llvm::Type*> virtualArgTypes;
                    virtualArgTypes.push_back(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
                    for (size_t i = 0; i < funcType->getNumParams(); ++i) {
                        virtualArgTypes.push_back(funcType->getParamType(i));
                    }
                    virtualFuncType = llvm::FunctionType::get(funcType->getReturnType(), virtualArgTypes, false);
                    auto* virtualFuncPtrType = llvm::PointerType::get(virtualFuncType, 0);
                    funcPtrToCall = llvmModCtx.Builder->CreateLoad(virtualFuncPtrType, vtableSlotPtr, "func_ptr");
                }

                if (methodDef->returnType->type == IRValueType::valueType::none) {
                    llvmModCtx.Builder->CreateCall(virtualFuncType, funcPtrToCall, finalArgs);
                } else {
                    llvm::CallInst* call = llvmModCtx.Builder->CreateCall(virtualFuncType, funcPtrToCall, finalArgs, "virtcall");
                    llvmModCtx.valueStackPhi.push_back({call, methodDef->returnType});
                }
                
                for (auto &i : postCleanup) {
                    callGcFunction(llvmModCtx, i.second, i.first, false);
                }

                callGcFunction(llvmModCtx, interfaceShellVal.llvmValue, interfaceShellVal.yoiType, false);
                break;
            }
            case IR::Opcode::new_array_int:
            case IR::Opcode::new_array_bool:
            case IR::Opcode::new_array_char:
            case IR::Opcode::new_array_deci:
            case IR::Opcode::new_array_unsigned:
            case IR::Opcode::new_array_short:
            case IR::Opcode::new_array_str: {
                yoi::indexT size = 1;
                yoi::vec<StackValue> dimensionsVal;
                yoi::vec<yoi::indexT> dimensions;

                std::shared_ptr<yoi::IRValueType> elementType;
                for (yoi::indexT i = 1; i < instr.operands.size(); ++i) {
                    size *= instr.operands[i].value.symbolIndex;
                    dimensions.push_back(instr.operands[i].value.symbolIndex);
                }
                for (yoi::indexT i = 0; i < instr.operands[0].value.symbolIndex; ++i) {
                    // for basic types, receiving value is not owning the value, so we don't need to increase the refcount.
                    dimensionsVal.push_back(llvmModCtx.valueStackPhi[llvmModCtx.valueStackPhi.size() - instr.operands[0].value.symbolIndex + i]);
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
                    case IR::Opcode::new_array_unsigned:
                        elementType = compilerCtx->getUnsignedObjectType();
                        break;
                    case IR::Opcode::new_array_short:
                        elementType = compilerCtx->getShortObjectType();
                        break;
                    default:
                        break;
                }

                // Create the array object
                auto arrayType = managedPtr(elementType->getArrayType(dimensions));
                auto val = createArrayObject(llvmModCtx, arrayType, dimensionsVal);

                for (yoi::indexT i = 0; i < instr.operands[0].value.symbolIndex; ++i) {
                    callGcFunction(llvmModCtx, llvmModCtx.valueStackPhi.back().llvmValue, llvmModCtx.valueStackPhi.back().yoiType, false);
                    llvmModCtx.valueStackPhi.pop_back();
                }

                llvmModCtx.valueStackPhi.push_back({val, arrayType});
                break;
            }
            case IR::Opcode::new_array_struct:
            case IR::Opcode::new_array_interface: {
                yoi::indexT size = 1;
                yoi::vec<StackValue> dimensionsVal;
                yoi::vec<yoi::indexT> dimensions;

                std::shared_ptr<yoi::IRValueType> elementType;
                for (yoi::indexT i = 3; i < instr.operands.size(); ++i) {
                    size *= instr.operands[i].value.symbolIndex;
                    dimensions.push_back(instr.operands[i].value.symbolIndex);
                }
                for (yoi::indexT i = 0; i < instr.operands[2].value.symbolIndex; ++i) {
                    auto value = promiseInterfaceObjectIfInterface(llvmModCtx, llvmModCtx.valueStackPhi[llvmModCtx.valueStackPhi.size() - size + i]);
                    if (value.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                        callGcFunction(llvmModCtx, value.llvmValue, value.yoiType, true, true, true);
                    dimensionsVal.push_back(value);
                }

                elementType = managedPtr(IRValueType{instr.opcode == IR::Opcode::new_array_struct ? IRValueType::valueType::structObject : IRValueType::valueType::interfaceObject, yoiModule->identifier, instr.operands[1].value.symbolIndex});

                // Create the array object
                auto arrayType = managedPtr(elementType->getArrayType(dimensions));
                auto val = createArrayObject(llvmModCtx, arrayType, dimensionsVal);
                for (yoi::indexT i = 0; i < size; ++i) {
                    // pop the values from the stack
                    llvmModCtx.valueStackPhi.pop_back();
                }

                llvmModCtx.valueStackPhi.push_back({val, arrayType});
                break;
            }
            case IR::Opcode::new_dynamic_array_int:
            case IR::Opcode::new_dynamic_array_bool:
            case IR::Opcode::new_dynamic_array_char:
            case IR::Opcode::new_dynamic_array_deci:
            case IR::Opcode::new_dynamic_array_unsigned:
            case IR::Opcode::new_dynamic_array_short:
            case IR::Opcode::new_dynamic_array_str: {
                yoi::indexT size = instr.operands.back().value.symbolIndex;

                auto llvmSize = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto unboxedSize = unboxValue(llvmModCtx, llvmSize.llvmValue, llvmSize.yoiType);

                yoi::vec<StackValue> valuesToStore;
                std::shared_ptr<yoi::IRValueType> elementType;
                for (yoi::indexT i = 0; i < size; ++i) {
                    auto value = llvmModCtx.valueStackPhi[llvmModCtx.valueStackPhi.size() - size + i];
                    valuesToStore.push_back(value);
                }

                switch (instr.opcode) {
                    case IR::Opcode::new_dynamic_array_int:
                        elementType = compilerCtx->getIntObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_bool:
                        elementType = compilerCtx->getBoolObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_char:
                        elementType = compilerCtx->getCharObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_deci:
                        elementType = compilerCtx->getDeciObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_str:
                        elementType = compilerCtx->getStrObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_short:
                        elementType = compilerCtx->getShortObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_unsigned:
                        elementType = compilerCtx->getUnsignedObjectType();
                        break;
                    default:
                        break;
                }

                // Create the array object
                auto arrayType = managedPtr(elementType->getDynamicArrayType());
                auto val = createDynamicArrayObject(llvmModCtx, arrayType, valuesToStore, unboxedSize);

                for (yoi::indexT i = 0; i < size; ++i) {
                    callGcFunction(llvmModCtx, llvmModCtx.valueStackPhi.back().llvmValue, llvmModCtx.valueStackPhi.back().yoiType, false);
                    llvmModCtx.valueStackPhi.pop_back();
                }

                llvmModCtx.valueStackPhi.push_back({val, arrayType});

                // release index
                callGcFunction(llvmModCtx, llvmSize.llvmValue, llvmSize.yoiType, false);
                break;
            }
            case IR::Opcode::new_dynamic_array_struct:
            case IR::Opcode::new_dynamic_array_interface: {
                yoi::indexT size = instr.operands.back().value.symbolIndex;

                auto llvmSize = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto unboxedSize = unboxValue(llvmModCtx, llvmSize.llvmValue, llvmSize.yoiType);

                yoi::vec<StackValue> valuesToStore;
                std::shared_ptr<yoi::IRValueType> elementType;
                for (yoi::indexT i = 0; i < size; ++i) {
                    auto value = promiseInterfaceObjectIfInterface(llvmModCtx, llvmModCtx.valueStackPhi[llvmModCtx.valueStackPhi.size() - size + i]);
                    if (value.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                        callGcFunction(llvmModCtx, value.llvmValue, value.yoiType, true, true, true);
                    valuesToStore.push_back(value);
                }

                elementType = managedPtr(IRValueType{instr.opcode == IR::Opcode::new_dynamic_array_struct ? IRValueType::valueType::structObject : IRValueType::valueType::interfaceObject, yoiModule->identifier, instr.operands[1].value.symbolIndex});

                // Create the array object
                auto arrayType = managedPtr(elementType->getDynamicArrayType());
                auto val = createDynamicArrayObject(llvmModCtx, arrayType, valuesToStore, unboxedSize);

                for (yoi::indexT i = 0; i < size; ++i) {
                    llvmModCtx.valueStackPhi.pop_back();
                }

                llvmModCtx.valueStackPhi.push_back({val, arrayType});
                callGcFunction(llvmModCtx, llvmSize.llvmValue, llvmSize.yoiType, false);
                break;
            }
            case IR::Opcode::load_element: {
                auto indexVal = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto arrayVal = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                auto arrayType = arrayVal.yoiType;
                std::shared_ptr<yoi::IRValueType> elementType;
                if (arrayType->isBasicType()) {
                    elementType = managedPtr(arrayType->getElementType().getBasicRawType());
                } else if (arrayType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope)) {
                    elementType = managedPtr(arrayType->getElementType().addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope).addAttribute(IRValueType::ValueAttr::Nullable));
                } else {
                    elementType = managedPtr(arrayType->getElementType().addAttribute(IRValueType::ValueAttr::Nullable));
                }
                auto unboxedIndexVal = unboxValue(llvmModCtx, indexVal.llvmValue, indexVal.yoiType);
                auto result = loadArrayElement(llvmModCtx, arrayType, arrayVal.llvmValue, unboxedIndexVal);

                llvmModCtx.valueStackPhi.push_back({result, elementType});
                // resource releasing
                callGcFunction(llvmModCtx, indexVal.llvmValue, indexVal.yoiType, false);
                callGcFunction(llvmModCtx, arrayVal.llvmValue, arrayVal.yoiType, false);
                break;
            }
            case IR::Opcode::pop: {
                if (llvmModCtx.valueStackPhi.empty())
                    break;
                auto val = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                callGcFunction(llvmModCtx, val.llvmValue, val.yoiType, false);
                break;
            }
            case IR::Opcode::direct_assign: {
                auto rhs = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto lhs = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                rhs = promiseInterfaceObjectIfInterface(llvmModCtx, rhs);

                auto object = ensureObject(llvmModCtx, rhs.yoiType, rhs.llvmValue);
                rhs = {object.second, object.first};

                auto lhsType = lhs.yoiType;
                auto rhsType = rhs.yoiType;
                auto lhsLLVMType = llvmModCtx.structTypeMap.at(std::make_tuple(lhsType->type, lhsType->typeAffiliateModule, lhsType->typeIndex));
                auto rhsLLVMType = llvmModCtx.structTypeMap.at(std::make_tuple(rhsType->type, rhsType->typeAffiliateModule, rhsType->typeIndex));

                if (lhsType->type == IRValueType::valueType::structObject) {
                    // reduce refcount of object inside the lhs
                    yoi::indexT fieldIndex = 2;
                    for (const auto& field : yoiModule->structTable[lhsType->typeIndex]->fieldTypes) {
                        auto fieldPtr = llvmModCtx.Builder->CreateStructGEP(lhsLLVMType, lhs.llvmValue, fieldIndex, "field_ptr");
                        auto loadedFieldPtr = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), fieldPtr, "loaded_field_ptr");
                        callGcFunction(llvmModCtx, loadedFieldPtr, field, false);
                        fieldIndex++;
                    }
                    fieldIndex = 2;
                    for (const auto& field : yoiModule->structTable[rhsType->typeIndex]->fieldTypes) {
                        auto fieldPtr = llvmModCtx.Builder->CreateStructGEP(rhsLLVMType, rhs.llvmValue, fieldIndex, "field_ptr");
                        auto loadedFieldPtr = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), fieldPtr, "loaded_field_ptr");
                        callGcFunction(llvmModCtx, loadedFieldPtr, field, true);
                        fieldIndex++;
                    }
                } else if (lhsType->type == IRValueType::valueType::interfaceObject) {
                    // reduce refcount of object inside the lhs
                    // this time, we use implementation-specific vtable slots to reduce refcount
                    auto thisPtr = llvmModCtx.Builder->CreateStructGEP(lhsLLVMType, lhs.llvmValue, 1, "this_ptr_field");
                    auto* concreteThisPtrRaw = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), thisPtr, "concrete_this_raw");
                    auto* implGcDecSlot = llvmModCtx.Builder->CreateStructGEP(lhsLLVMType, lhs.llvmValue, 3, "impl_gc_dec_slot");
                    auto* implGcDecFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0)}, false);
                    llvmModCtx.Builder->CreateCall(implGcDecFuncType, implGcDecSlot, {concreteThisPtrRaw});
                    auto rhsThisPtr = llvmModCtx.Builder->CreateStructGEP(rhsLLVMType, rhs.llvmValue, 1, "this_ptr_field");
                    auto* rhsConcreteThisPtrRaw = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), rhsThisPtr, "rhs_concrete_this_raw");
                    auto* implGcIncSlot = llvmModCtx.Builder->CreateStructGEP(rhsLLVMType, rhs.llvmValue, 2, "impl_gc_inc_slot");
                    auto* implGcIncFuncType = llvm::FunctionType::get(llvmModCtx.Builder->getVoidTy(), {llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0)}, false);
                    llvmModCtx.Builder->CreateCall(implGcIncFuncType, implGcIncSlot, {rhsConcreteThisPtrRaw});
                }

                auto structTypeSize = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(lhsLLVMType);
                // offset from 16 bytes to skip the refcount, and memcpy the rhs value to lhs
                auto* lhsPtr = llvmModCtx.Builder->CreateBitCast(lhs.llvmValue, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), "lhs_ptr");
                auto* rhsPtr = llvmModCtx.Builder->CreateBitCast(rhs.llvmValue, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), "rhs_ptr");
                auto* offsettedLhsPtr = llvmModCtx.Builder->CreateGEP(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), lhsPtr, {llvm::ConstantInt::get(llvmModCtx.Builder->getInt32Ty(), 16, true)});
                auto* offsettedRhsPtr = llvmModCtx.Builder->CreateGEP(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), rhsPtr, {llvm::ConstantInt::get(llvmModCtx.Builder->getInt32Ty(), 16, true)});
                llvmModCtx.Builder->CreateMemCpy(offsettedLhsPtr, llvm::MaybeAlign(8), offsettedRhsPtr, llvm::MaybeAlign(8), structTypeSize - 16);
                callGcFunction(llvmModCtx, rhs.llvmValue, rhs.yoiType, false);
                llvmModCtx.valueStackPhi.push_back(lhs);
                break;
            }
            case IR::Opcode::dyn_cast_int:
            case IR::Opcode::dyn_cast_bool:
            case IR::Opcode::dyn_cast_deci:
            case IR::Opcode::dyn_cast_str:
            case IR::Opcode::dyn_cast_char:
            case IR::Opcode::dyn_cast_struct:
            case IR::Opcode::dyn_cast_any: {
                std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> structTypeKey;
                std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT> structTypeIDKey;

                auto interfaceRhs = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                auto structTypeIndex = instr.operands[1].value.symbolIndex;
                std::shared_ptr<IRValueType> structYoiType;

                switch (instr.opcode) {
                    case IR::Opcode::dyn_cast_int:
                        structTypeKey = std::make_tuple(IRValueType::valueType::integerObject, instr.operands[0].value.symbolIndex, structTypeIndex);
                        structTypeIDKey = std::make_tuple(IRValueType::valueType::integerObject, instr.operands[0].value.symbolIndex, structTypeIndex, 0);
                        structYoiType = managedPtr(IRValueType{IRValueType::valueType::integerObject, instr.operands[0].value.symbolIndex, structTypeIndex});
                        break;
                    case IR::Opcode::dyn_cast_bool:
                        structTypeKey = std::make_tuple(IRValueType::valueType::booleanObject, instr.operands[0].value.symbolIndex, structTypeIndex);
                        structTypeIDKey = std::make_tuple(IRValueType::valueType::booleanObject, instr.operands[0].value.symbolIndex, structTypeIndex, 0);
                        structYoiType = managedPtr(IRValueType{IRValueType::valueType::booleanObject, instr.operands[0].value.symbolIndex, structTypeIndex});
                        break;
                    case IR::Opcode::dyn_cast_deci:
                        structTypeKey = std::make_tuple(IRValueType::valueType::decimalObject, instr.operands[0].value.symbolIndex, structTypeIndex);
                        structTypeIDKey = std::make_tuple(IRValueType::valueType::decimalObject, instr.operands[0].value.symbolIndex, structTypeIndex, 0);
                        structYoiType = managedPtr(IRValueType{IRValueType::valueType::decimalObject, instr.operands[0].value.symbolIndex, structTypeIndex});
                        break;
                    case IR::Opcode::dyn_cast_char:
                        structTypeKey = std::make_tuple(IRValueType::valueType::characterObject, instr.operands[0].value.symbolIndex, structTypeIndex);
                        structTypeIDKey = std::make_tuple(IRValueType::valueType::characterObject, instr.operands[0].value.symbolIndex, structTypeIndex, 0);
                        structYoiType = managedPtr(IRValueType{IRValueType::valueType::characterObject, instr.operands[0].value.symbolIndex, structTypeIndex});
                        break;
                    case IR::Opcode::dyn_cast_str:
                        structTypeKey = std::make_tuple(yoi::IRValueType::valueType::stringObject, instr.operands[0].value.symbolIndex, structTypeIndex);
                        structTypeIDKey = std::make_tuple(IRValueType::valueType::stringObject, instr.operands[0].value.symbolIndex, structTypeIndex, 0);
                        structYoiType = managedPtr(IRValueType{yoi::IRValueType::valueType::stringObject, instr.operands[0].value.symbolIndex, structTypeIndex});
                        break;
                    case IR::Opcode::dyn_cast_any:
                        structTypeIDKey = std::make_tuple(static_cast<IRValueType::valueType>(instr.operands[0].value.symbolIndex), instr.operands[1].value.symbolIndex, instr.operands[2].value.symbolIndex, instr.operands[3].value.symbolIndex);
                        structYoiType = managedPtr(IRValueType{static_cast<IRValueType::valueType>(instr.operands[0].value.symbolIndex), instr.operands[1].value.symbolIndex, instr.operands[2].value.symbolIndex, yoi::vec<yoi::indexT>{instr.operands[3].value.symbolIndex}});
                        break;
                    default:
                        structTypeKey = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structTypeIndex);
                        structTypeIDKey = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structTypeIndex, 0);
                        structYoiType = managedPtr(IRValueType{IRValueType::valueType::structObject, yoiModule->identifier, structTypeIndex});
                        break;
                }

                if (interfaceRhs.yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
                    auto regressedImpl = interfaceRhs.yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                    auto implDef = yoiModule->interfaceImplementationTable[regressedImpl.second];
                    if (implDef->implStructIndex == structTypeKey) {
                        llvmModCtx.valueStackPhi.push_back({unwrapInterfaceObject(llvmModCtx, interfaceRhs), managedPtr(IRValueType{std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex)})});
                    } else {
                        auto nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), 0));
                        llvmModCtx.valueStackPhi.push_back({nullValue, managedPtr(IRValueType{std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex)})});
                    }

                    if (interfaceRhs.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                        callGcFunction(llvmModCtx, interfaceRhs.llvmValue, interfaceRhs.yoiType, true, true);

                    // no decrement here
                    // for which it is a reuse

                    break;
                }

                auto structTypeId = llvmModCtx.typeIDMap.at(structTypeIDKey);
                auto structTypeLLVMType = structYoiType->isArrayType() || structYoiType->isDynamicArrayType() 
                    ? llvmModCtx.arrayTypeMap.at(structTypeIDKey)
                    : llvmModCtx.structTypeMap.at(structTypeKey);
                
                // offset by 16 bytes to skip the refcount and typeid
                auto* interfacePtr = llvmModCtx.Builder->CreateBitCast(interfaceRhs.llvmValue, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), "interface_ptr");
                auto* offsettedInterfacePtr = llvmModCtx.Builder->CreateGEP(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), interfacePtr, {llvm::ConstantInt::get(llvmModCtx.Builder->getInt32Ty(), 16, true)});
                auto* structPtrPtr = llvmModCtx.Builder->CreateBitCast(offsettedInterfacePtr, llvm::PointerType::get(structTypeLLVMType, 0), "struct_ptr");
                auto* loadedStructPtr = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(structTypeLLVMType, 0), structPtrPtr, "loaded_struct_ptr");
                // offset by 8 bytes and check typeid
                auto* typeIdPtr = llvmModCtx.Builder->CreateStructGEP(structTypeLLVMType, loadedStructPtr, 1, "typeid_ptr");
                auto* loadedTypeId = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), typeIdPtr, "loaded_typeid");
                auto* expectedTypeId = llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), structTypeId, true);
                auto* typeIdMatch = llvmModCtx.Builder->CreateICmpEQ(loadedTypeId, expectedTypeId, "typeid_match");
                
                auto* failedMatchBB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "failed_match_bb", llvmModCtx.currentFunction);
                auto* successBB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "success_bb", llvmModCtx.currentFunction);
                auto* continueBB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "continue_bb", llvmModCtx.currentFunction);

                llvmModCtx.Builder->CreateCondBr(typeIdMatch, successBB, failedMatchBB);
                // failed match
                llvmModCtx.Builder->SetInsertPoint(failedMatchBB);
                auto* nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(structTypeLLVMType, 0));
                llvmModCtx.Builder->CreateBr(continueBB);
                // success match
                llvmModCtx.Builder->SetInsertPoint(successBB);
                auto* resultObject = loadedStructPtr;
                callGcFunction(llvmModCtx, resultObject, structYoiType, true);
                llvmModCtx.Builder->CreateBr(continueBB);
                // in continue block, decrement the interface refcount
                llvmModCtx.Builder->SetInsertPoint(continueBB);
                auto *finalValue = llvmModCtx.Builder->CreatePHI(llvm::PointerType::get(structTypeLLVMType, 0), 2, "final_value");
                finalValue->addIncoming(resultObject, successBB);
                finalValue->addIncoming(nullValue, failedMatchBB);
                callGcFunction(llvmModCtx, interfaceRhs.llvmValue, interfaceRhs.yoiType, false);

                llvmModCtx.valueStackPhi.push_back({finalValue, structYoiType});
                break;
            }
            case IR::Opcode::push_null: {
                auto nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0));
                llvmModCtx.valueStackPhi.push_back({nullValue, managedPtr(IRValueType{IRValueType::valueType::pointerObject})});
                break;
            }
            case IR::Opcode::pointer_cast: {
                auto rhs = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto value = llvmModCtx.Builder->CreateBitCast(rhs.llvmValue, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), "pointer_cast");
                llvmModCtx.valueStackPhi.push_back({value, managedPtr(IRValueType{IRValueType::valueType::pointerObject})});
                callGcFunction(llvmModCtx, rhs.llvmValue, rhs.yoiType, false);
                break;
            }
            case IR::Opcode::store_element: {
                auto index = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto lhs = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                auto rhs = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

                yoi_assert(lhs.yoiType->isArrayType() || lhs.yoiType->isDynamicArrayType(), instr.debugInfo.line, instr.debugInfo.column, "LLVM Codegen: store element on non-array type.");
                yoi_assert(index.yoiType->type == IRValueType::valueType::unsignedObject || index.yoiType->type == IRValueType::valueType::unsignedRaw, instr.debugInfo.line, instr.debugInfo.column, "LLVM Codegen: store element with non-integer index.");

                rhs = promiseInterfaceObjectIfInterface(llvmModCtx, rhs);

                // unbox index
                auto* indexValue = unboxValue(llvmModCtx, index.llvmValue, index.yoiType);
                if (rhs.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !lhs.yoiType->isBasicType())
                    callGcFunction(llvmModCtx, rhs.llvmValue, rhs.yoiType, true, true, true);
                storeArrayElement(llvmModCtx, lhs.yoiType, rhs.yoiType, lhs.llvmValue, indexValue, rhs.llvmValue);

                // release resource
                callGcFunction(llvmModCtx, index.llvmValue, index.yoiType, false);
                callGcFunction(llvmModCtx, rhs.llvmValue, rhs.yoiType, false);
                callGcFunction(llvmModCtx, lhs.llvmValue, lhs.yoiType, false);
                break;
            }
            case IR::Opcode::array_length: {
                auto array = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                yoi_assert(array.yoiType->isArrayType() || array.yoiType->isDynamicArrayType(), instr.debugInfo.line, instr.debugInfo.column, "LLVM Codegen: array length on non-array type.");
                auto arrayLLVMType = getArrayLLVMType(llvmModCtx, array.yoiType);
                // gep index 2
                auto *arrayLen = llvmModCtx.Builder->CreateStructGEP(arrayLLVMType, array.llvmValue, 2, "array_len");
                auto *loadedArrayLen = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), arrayLen, "loaded_array_len");
                llvmModCtx.valueStackPhi.push_back({loadedArrayLen, managedPtr(compilerCtx->getIntObjectType()->getBasicRawType())});
                callGcFunction(llvmModCtx, array.llvmValue, array.yoiType, false);
                break;
            }
            case IR::Opcode::interfaceof: {
                // get the typeid off the stack
                auto typeidValue = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                yoi_assert(typeidValue.yoiType->type == IRValueType::valueType::integerObject || typeidValue.yoiType->type == IRValueType::valueType::integerRaw, instr.debugInfo.line, instr.debugInfo.column, "LLVM Codegen: interfaceof with non-integer typeid.");
                // get the interface object off the stack
                auto interfaceValue = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
                

                if (interfaceValue.yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
                    auto regressedImpl = interfaceValue.yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                    auto implDef = yoiModule->interfaceImplementationTable[regressedImpl.second];
                    if (llvmModCtx.typeIDMap.contains({std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex), 0})) {
                        auto *trueBoolean = llvm::ConstantInt::get(llvmModCtx.Builder->getInt1Ty(), 1, true);
                        llvmModCtx.valueStackPhi.push_back({trueBoolean, managedPtr(compilerCtx->getBoolObjectType()->getBasicRawType())});
                    } else {
                        auto *falseBoolean = llvm::ConstantInt::get(llvmModCtx.Builder->getInt1Ty(), 0, true);
                        llvmModCtx.valueStackPhi.push_back({falseBoolean, managedPtr(compilerCtx->getBoolObjectType()->getBasicRawType())});
                    }
                    callGcFunction(llvmModCtx, interfaceValue.llvmValue, interfaceValue.yoiType, false);
                    callGcFunction(llvmModCtx, typeidValue.llvmValue, typeidValue.yoiType, false);
                    break;
                }

                // evaluate the interface this
                auto interfaceKey = std::make_tuple(interfaceValue.yoiType->type, interfaceValue.yoiType->typeAffiliateModule, interfaceValue.yoiType->typeIndex);
                // auto thisPtrToStruct = llvmModCtx.Builder->CreateStructGEP(llvmModCtx.structTypeMap.at(interfaceKey), interfaceValue.llvmValue, 2, "this_ptr_to_struct");
                // auto loadedThisPtr = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvmModCtx.Builder->getInt64Ty(), 0), thisPtrToStruct, "loaded_this_ptr");
                auto loadedThisPtr = unwrapInterfaceObject(llvmModCtx, interfaceValue);
                // offset by 8 bytes and check typeid
                auto* typeIdPtr = llvmModCtx.Builder->CreateGEP(llvmModCtx.Builder->getInt64Ty(), loadedThisPtr, {llvm::ConstantInt::get(llvmModCtx.Builder->getInt32Ty(), 1, true)});
                auto* loadedTypeId = llvmModCtx.Builder->CreateLoad(llvmModCtx.Builder->getInt64Ty(), typeIdPtr, "loaded_typeid");
                auto* expectedTypeId = unboxValue(llvmModCtx, typeidValue.llvmValue, typeidValue.yoiType);
                auto* typeIdMatch = llvmModCtx.Builder->CreateICmpEQ(loadedTypeId, expectedTypeId, "typeid_match");
                
                auto* failedMatchBB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "failed_match_bb", llvmModCtx.currentFunction);
                auto* successBB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "success_bb", llvmModCtx.currentFunction);
                auto* continueBB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "continue_bb", llvmModCtx.currentFunction);

                llvmModCtx.Builder->CreateCondBr(typeIdMatch, successBB, failedMatchBB);
                // failed match
                llvmModCtx.Builder->SetInsertPoint(failedMatchBB);
                auto *falseBoolean = llvm::ConstantInt::get(llvmModCtx.Builder->getInt1Ty(), 0, true);
                llvmModCtx.Builder->CreateBr(continueBB);
                // success match
                llvmModCtx.Builder->SetInsertPoint(successBB);
                auto *trueBoolean = llvm::ConstantInt::get(llvmModCtx.Builder->getInt1Ty(), 1, true);
                llvmModCtx.Builder->CreateBr(continueBB);
                // in continue block, decrement the interface refcount
                // but phi first
                llvmModCtx.Builder->SetInsertPoint(continueBB);
                auto phiNode = llvmModCtx.Builder->CreatePHI(llvm::Type::getInt1Ty(*llvmModCtx.TheContext), 2, "phi_node");
                phiNode->addIncoming(trueBoolean, successBB);
                phiNode->addIncoming(falseBoolean, failedMatchBB);

                callGcFunction(llvmModCtx, interfaceValue.llvmValue, interfaceValue.yoiType, false);
                callGcFunction(llvmModCtx, typeidValue.llvmValue, typeidValue.yoiType, false);
                llvmModCtx.valueStackPhi.push_back({phiNode, managedPtr(compilerCtx->getBoolObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::typeid_object_non_stack: {
                auto key = std::make_tuple(static_cast<IRValueType::valueType>(instr.operands[0].value.symbolIndex), instr.operands[1].value.symbolIndex, instr.operands[2].value.symbolIndex, instr.operands[3].value.symbolIndex);
                auto typeId = llvmModCtx.typeIDMap.at(key);
                llvmModCtx.valueStackPhi.push_back({llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), typeId, true), managedPtr(compilerCtx->getIntObjectType()->getBasicRawType())});
                break;
            }
            case IR::Opcode::nop:
                break;
            default:
                panic(instr.debugInfo.line, instr.debugInfo.column, "LLVM Codegen: Unhandled yoi::IR opcode: " + std::string(magic_enum::enum_name(instr.opcode)));
        }
    }

    llvm::Type* LLVMCodegen::yoiTypeToLLVMType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType>& type, bool enforceForeignType) {
        if (enforceForeignType) {
            if (type->isArrayType()) {
                // TODO
            }
            auto [typeEnum, typeModule, typeIndex, dim, attr, _a, _b] = type->isBasicRawType() ? type->getBasicObjectType() : *type;
            auto key = std::make_tuple(typeEnum, typeModule, typeIndex);
            if (llvmModCtx.foreignTypeMap.count(key)) {
                return llvmModCtx.foreignTypeMap.at(key);
            }

            yoi_assert(type->isForeignBasicType(), 0, 0, "LLVM Codegen: Enforcing foreign type, but type is not a exported type or basic type.");
        } else {
            auto key = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);
            if (llvmModCtx.structTypeMap.count(key)) {
                if (type->isArrayType() || type->isDynamicArrayType()) {
                    return llvm::PointerType::get(getArrayLLVMType(llvmModCtx, type), 0);
                } else {
                    return llvm::PointerType::get(llvmModCtx.structTypeMap.at(key), 0);
                }
            }
        }

        // Fallback for non-object types or errors
        switch (type->type) {
            case IRValueType::valueType::integerRaw:
                return llvmModCtx.Builder->getInt64Ty();
            case IRValueType::valueType::decimalRaw:
                return llvmModCtx.Builder->getDoubleTy();
            case IRValueType::valueType::booleanRaw:
                return llvmModCtx.Builder->getInt1Ty();
            case IRValueType::valueType::shortRaw:
                return llvmModCtx.Builder->getInt16Ty();
            case IRValueType::valueType::unsignedRaw:
                return llvmModCtx.Builder->getInt64Ty();
            case IRValueType::valueType::charRaw:
                return llvmModCtx.Builder->getInt8Ty();
            case IRValueType::valueType::pointer:
            case IRValueType::valueType::pointerObject: // generic pointer
                return llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
            case yoi::IRValueType::valueType::foreignFloatType:
                return llvmModCtx.Builder->getFloatTy();
            case IRValueType::valueType::foreignInt32Type:
                return llvmModCtx.Builder->getInt32Ty();
            case IRValueType::valueType::none:
                return llvmModCtx.Builder->getVoidTy();
            case IRValueType::valueType::stringLiteral:
                return llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0);
            default:
                panic(0, 0, "LLVM Codegen: Unhandled or unmapped yoi::IRValueType: " + std::string(magic_enum::enum_name(type->type)));
                return nullptr;
        }
        panic(0, 0, "No LLVM type available for yoiTypeToLLVMType yet: " + yoi::wstring2string(type->to_string()));
    }

    llvm::FunctionType* LLVMCodegen::getFunctionType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRFunctionDefinition>& funcDef) {
        auto* returnType = yoiTypeToLLVMType(llvmModCtx, funcDef->returnType, funcDef->returnType->hasAttribute(IRValueType::ValueAttr::Raw));

        std::vector<llvm::Type*> argTypes;
        for (const auto& argType : funcDef->argumentTypes) {
            argTypes.push_back(yoiTypeToLLVMType(llvmModCtx, argType, argType->hasAttribute(IRValueType::ValueAttr::Raw)));
        }
        return llvm::FunctionType::get(returnType, argTypes, false);
    }

    llvm::Constant* LLVMCodegen::getGlobalInitializer(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType>& type) {
        auto* llvmType = yoiTypeToLLVMType(llvmModCtx, type);
        return llvm::Constant::getNullValue(llvmType);
    }

    void LLVMCodegen::handleBinaryOp(LLVMModuleContext &llvmModCtx, llvm::Instruction::BinaryOps op, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock) {
        auto R = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
        auto L = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
        bool isUnsigned = L.yoiType->type == IRValueType::valueType::unsignedObject || L.yoiType->type == IRValueType::valueType::unsignedRaw;

        llvm::Value* lValRaw = unboxValue(llvmModCtx, L.llvmValue, L.yoiType);
        llvm::Value* rValRaw = unboxValue(llvmModCtx, R.llvmValue, R.yoiType);

        bool typesAreFloats = lValRaw->getType()->isDoubleTy() || rValRaw->getType()->isDoubleTy();
        auto resultYoiType = L.yoiType->getBasicRawType();

        llvm::Value* resultRaw;

        if (typesAreFloats) {
            if (lValRaw->getType()->isIntegerTy()) lValRaw = llvmModCtx.Builder->CreateSIToFP(lValRaw, llvmModCtx.Builder->getDoubleTy(), "inttofp");
            if (rValRaw->getType()->isIntegerTy()) rValRaw = llvmModCtx.Builder->CreateSIToFP(rValRaw, llvmModCtx.Builder->getDoubleTy(), "inttofp");
            auto fop = op;
            switch(op) {
                case llvm::Instruction::Add: fop = llvm::Instruction::FAdd; break;
                case llvm::Instruction::Sub: fop = llvm::Instruction::FSub; break;
                case llvm::Instruction::Mul: fop = llvm::Instruction::FMul; break;
                case llvm::Instruction::SDiv: fop = llvm::Instruction::FDiv; break;
                case llvm::Instruction::SRem: fop = llvm::Instruction::FRem; break;
                default: panic(0,0, "Unsupported float binary op");
            }
            resultRaw = llvmModCtx.Builder->CreateBinOp(fop, lValRaw, rValRaw, "fbinop");
        } else if (isUnsigned) {
            auto newOp = op;
            switch (op) {
                case llvm::Instruction::SDiv: newOp = llvm::Instruction::UDiv; break;
                case llvm::Instruction::SRem: newOp = llvm::Instruction::URem; break;
                default: break;
            }
            resultRaw = llvmModCtx.Builder->CreateBinOp(newOp, lValRaw, rValRaw, "ubinop");
        } else {
            resultRaw = llvmModCtx.Builder->CreateBinOp(op, lValRaw, rValRaw, "ibinop");
        }

        llvmModCtx.valueStackPhi.push_back({resultRaw, managedPtr(resultYoiType)});

        // Consume operands
        callGcFunction(llvmModCtx, L.llvmValue, L.yoiType, false);
        callGcFunction(llvmModCtx, R.llvmValue, R.yoiType, false);
    }

    void LLVMCodegen::handleComparison(LLVMModuleContext &llvmModCtx, llvm::CmpInst::Predicate pred, bool isFloat, yoi::indexT fromBlock, yoi::indexT toBlock) {
        auto R = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();
        auto L = llvmModCtx.valueStackPhi.back(); llvmModCtx.valueStackPhi.pop_back();

        llvm::Value* lValRaw = L.yoiType->type == IRValueType::valueType::pointerObject ? L.llvmValue : unboxValue(llvmModCtx, L.llvmValue, L.yoiType);
        llvm::Value* rValRaw = R.yoiType->type == IRValueType::valueType::pointerObject ? R.llvmValue : unboxValue(llvmModCtx, R.llvmValue, R.yoiType);

        bool isUnsigned = L.yoiType->type == IRValueType::valueType::unsignedObject || L.yoiType->type == IRValueType::valueType::unsignedRaw;
        bool typesAreFloats = lValRaw->getType()->isDoubleTy() || rValRaw->getType()->isDoubleTy();

        llvm::Value* resultRaw;
        if (typesAreFloats) {
            if (lValRaw->getType()->isIntegerTy()) lValRaw = llvmModCtx.Builder->CreateSIToFP(lValRaw, llvmModCtx.Builder->getDoubleTy(), "inttofp");
            if (rValRaw->getType()->isIntegerTy()) rValRaw = llvmModCtx.Builder->CreateSIToFP(rValRaw, llvmModCtx.Builder->getDoubleTy(), "inttofp");

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
            resultRaw = llvmModCtx.Builder->CreateFCmp(fpred, lValRaw, rValRaw, "fcmp");
        } else if (isUnsigned) {
            auto newPred = pred;
            switch (pred) {
                case llvm::CmpInst::ICMP_SLT: newPred = llvm::CmpInst::ICMP_ULT; break;
                case llvm::CmpInst::ICMP_SLE: newPred = llvm::CmpInst::ICMP_ULE; break;
                case llvm::CmpInst::ICMP_SGT: newPred = llvm::CmpInst::ICMP_UGT; break;
                case llvm::CmpInst::ICMP_SGE: newPred = llvm::CmpInst::ICMP_UGE; break;
                default: break;
            }
            resultRaw = llvmModCtx.Builder->CreateICmp(newPred, lValRaw, rValRaw, "ucmp");
        } else {
            resultRaw = llvmModCtx.Builder->CreateICmp(pred, lValRaw, rValRaw, "icmp");
        }

        llvmModCtx.valueStackPhi.push_back({resultRaw, managedPtr(compilerCtx->getBoolObjectType()->getBasicRawType())});

        // Consume operands
        callGcFunction(llvmModCtx, L.llvmValue, L.yoiType, false);
        callGcFunction(llvmModCtx, R.llvmValue, R.yoiType, false);
    }

    llvm::Value* LLVMCodegen::createBasicObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType>& yoiType, llvm::Value* rawValue) {
        if (yoiType->isBasicRawType() || yoiType->hasAttribute(IRValueType::ValueAttr::Raw)) {
            auto bitCastedValue = llvmModCtx.Builder->CreateBitCast(rawValue, yoiTypeToLLVMType(llvmModCtx, yoiType, true), "bitcast_val");
            return bitCastedValue;
        }

        auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
        auto typeIdKey = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex, 0);
        auto* objType = llvmModCtx.structTypeMap.at(key);
        auto typeId = llvmModCtx.typeIDMap.at(typeIdKey);

        auto size = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(objType);
        auto* sizeVal = llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), size);

        auto* allocCall = llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"object_alloc"), sizeVal, "new_obj_alloc");
        auto* newObjPtr = llvmModCtx.Builder->CreateBitCast(allocCall, llvm::PointerType::get(objType, 0), "new_obj_ptr");

        auto* refCountPtr = llvmModCtx.Builder->CreateStructGEP(objType, newObjPtr, 0, "refcount_ptr");
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), refCountPtr);

        auto typeIdPtr = llvmModCtx.Builder->CreateStructGEP(objType, newObjPtr, 1, "typeid_ptr");
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), typeId), typeIdPtr);

        auto* valuePtr = llvmModCtx.Builder->CreateStructGEP(objType, newObjPtr, 2, "value_ptr");
        llvmModCtx.Builder->CreateStore(rawValue, valuePtr);

        return newObjPtr;
    }

    llvm::Value* LLVMCodegen::unboxValue(LLVMModuleContext &llvmModCtx, llvm::Value* objectPtr, const std::shared_ptr<IRValueType>& yoiType) {
        if (yoiType->isBasicRawType() || yoiType->hasAttribute(IRValueType::ValueAttr::Raw)) {
            auto bitCastedValue = llvmModCtx.Builder->CreateBitCast(objectPtr, yoiTypeToLLVMType(llvmModCtx, yoiType, true), "bitcast_val");
            return bitCastedValue;
        }

        auto key = std::make_tuple(yoiType->type, yoiType->typeAffiliateModule, yoiType->typeIndex);
        auto* objType = llvmModCtx.structTypeMap.at(key);
        auto* valuePtr = llvmModCtx.Builder->CreateStructGEP(objType, objectPtr, 2, "value_ptr");
        return llvmModCtx.Builder->CreateLoad(objType->getElementType(2), valuePtr, "unboxed_val");
    }

    llvm::Function *LLVMCodegen::getGcFunction(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &yoiType, bool isIncrease) {
        auto finalType = managedPtr(*yoiType);

        if (yoiType->type == IRValueType::valueType::interfaceObject && yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
            auto impl = yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
            if (impl.first != -1) {
                auto implDef = yoiModule->interfaceImplementationTable[impl.second];
                finalType->type = std::get<0>(implDef->implStructIndex);
                finalType->typeAffiliateModule = std::get<1>(implDef->implStructIndex);
                finalType->typeIndex = std::get<2>(implDef->implStructIndex);
            }
        }

        std::string funcNameBase;
        if (finalType->isArrayType() || finalType->isDynamicArrayType()) {
            funcNameBase = "array_" + yoi::wstring2string(finalType->to_string());
        } else {
            switch(finalType->type) {
                case IRValueType::valueType::foreignInt32Type:
                case IRValueType::valueType::integerObject: funcNameBase = "basic_int"; break;
                case IRValueType::valueType::foreignFloatType:
                case IRValueType::valueType::decimalObject: funcNameBase = "basic_decimal"; break;
                case IRValueType::valueType::booleanObject: funcNameBase = "basic_bool"; break;
                case IRValueType::valueType::stringObject: funcNameBase = "basic_string"; break;
                case IRValueType::valueType::characterObject: funcNameBase = "basic_char"; break;
                case IRValueType::valueType::shortObject: funcNameBase = "basic_short"; break;
                case IRValueType::valueType::unsignedObject: funcNameBase = "basic_unsigned"; break;
                case IRValueType::valueType::structObject:
                    funcNameBase = "struct_" + std::to_string(finalType->typeAffiliateModule) + "_" + std::to_string(finalType->typeIndex);
                    break;
                case IRValueType::valueType::interfaceObject:
                    funcNameBase = "interface_" + std::to_string(finalType->typeAffiliateModule) + "_" + std::to_string(finalType->typeIndex);
                    break;
                default: return nullptr; // No GC needed for raw types or unhandled types
            }
        }

        auto funcName = funcNameBase + (isIncrease ? "_gc_refcount_increase" : "_gc_refcount_decrease");
        auto* gcFunc = llvmModCtx.functionMap.at(string2wstring(funcName));
        return gcFunc;
    }

    void LLVMCodegen::generateDescription(LLVMModuleContext &llvmModCtx) {
        auto* descStr = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext,
            std::string("hoshi-lang-")
            + yoi::wstring2string(compilerCtx->getBuildConfig()->buildPlatform)
            + "-"
            + yoi::wstring2string(compilerCtx->getBuildConfig()->buildArch),
            true);
        auto* descGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, descStr->getType(), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, descStr, "yoi_desc");

        auto* buildTypeStr = llvm::ConstantDataArray::getIntegerValue(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), llvm::APInt(64, static_cast<uint64_t>(compilerCtx->getBuildConfig()->buildType)));
        auto* buildTypeGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, buildTypeStr->getType(), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, buildTypeStr, "yoi_build_type");
    }

    void LLVMCodegen::generateTargetObjectCode(LLVMModuleContext &llvmModCtx, const yoi::wstr &pathToOutput) {
        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        llvmModCtx.TheModule->setTargetTriple(TargetTriple);
        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
        if (!Target) {
            panic(0, 0, "Could not create target for " + TargetTriple + " (" + Error + ")");
        }

        auto CPU = llvm::sys::getHostCPUName();

        // Automatically detect the features of the host CPU
        llvm::SubtargetFeatures SubFeatures;
        llvm::StringMap<bool> HostFeatures = llvm::sys::getHostCPUFeatures();
        for (auto &F : HostFeatures) {
            SubFeatures.AddFeature(F.first(), F.second);
        }
        auto Features = SubFeatures.getString();
        // printf("Target triple %s, using CPU %s with features %s\n", TargetTriple.c_str(), CPU.str().c_str(), !Features.empty() ? Features.c_str() : "N/A");

        llvm::TargetOptions Opt;
        auto RM = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
        llvm::CodeGenOptLevel OptLevel = compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::release ? llvm::CodeGenOptLevel::Aggressive : llvm::CodeGenOptLevel::None;
        llvm::OptimizationLevel OptLevelPB = compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::release ? llvm::OptimizationLevel::O3 : llvm::OptimizationLevel::O0;

        std::unique_ptr<llvm::TargetMachine> TM(
        Target->createTargetMachine(TargetTriple, CPU, Features, Opt, RM, std::optional<llvm::CodeModel::Model>(), OptLevel));

        if (!TM) {
            panic(0, 0, "Could not create TargetMachine for " + TargetTriple);
        }

        llvmModCtx.TheModule->setDataLayout(TM->createDataLayout());

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
        MPM.run(*llvmModCtx.TheModule, MAM);

        llvm::legacy::PassManager CodeGenPasses;
        llvm::CodeGenFileType FileType = llvm::CodeGenFileType::ObjectFile; // To emit a .o file

        if (TM->addPassesToEmitFile(CodeGenPasses, Dest, nullptr, FileType)) {
            panic(0, 0, "TargetMachine can't emit a file of this type");
        }

        CodeGenPasses.run(*llvmModCtx.TheModule);
        Dest.flush();
    }

    void LLVMCodegen::generateForeignStructTypes(LLVMModuleContext &llvmModCtx) {
        for (auto &foreignTypePair : compilerCtx->getIRFFITable()->foreignTypeTable) {
            auto &typeName = foreignTypePair.first;
            auto typeId = std::make_tuple(IRValueType::valueType::structObject, foreignTypePair.second->typeAffiliateModule, foreignTypePair.second->typeIndex);
            auto structType = yoiModule->structTable[foreignTypePair.second->typeIndex];
            yoi::vec<std::string> fieldNames;
            yoi::vec<llvm::Type*> fieldTypes;
            for (auto &name : structType->nameIndexMap) {
                if (name.second.type != IRStructDefinition::nameInfo::nameType::field) continue;

                auto fieldType = yoiTypeToLLVMType(llvmModCtx, structType->fieldTypes[name.second.index], true);
                fieldNames.push_back(yoi::wstring2string(name.first));
            }
            auto llvmStructType = llvm::StructType::create(*llvmModCtx.TheContext, fieldTypes);
            // add to foreign type map
            llvmModCtx.foreignTypeMap[typeId] = llvmStructType;
        }
    }

    void LLVMCodegen::generateExportFunctionDecls(LLVMModuleContext &llvmModCtx) {
        for (auto &exportedFunction : compilerCtx->getIRFFITable()->exportedFunctionTable) {
            auto &funcName = exportedFunction.first;
            auto &mangledName = yoiModule->functionTable.getKey(std::get<1>(exportedFunction.second));
            auto &funcDecl = yoiModule->functionTable[std::get<1>(exportedFunction.second)];
            auto &attrs = std::get<2>(exportedFunction.second);
            bool noffi = std::find(attrs.begin(), attrs.end(), IRFunctionDefinition::FunctionAttrs::NoFFI) != attrs.end();

            if (noffi) {
                llvm::Type *returnType = yoiTypeToLLVMType(llvmModCtx, funcDecl->returnType, false);
                yoi::vec<llvm::Type*> argTypes;
                for (auto &argType : funcDecl->argumentTypes) {
                    argTypes.push_back(yoiTypeToLLVMType(llvmModCtx, argType, false)); // make sure all types converted
                }
                llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
                llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, yoi::wstring2string(funcName), llvmModCtx.TheModule.get());
                llvmModCtx.functionMap[funcName] = func;

                llvm::BasicBlock *BB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", func);
                llvmModCtx.Builder->SetInsertPoint(BB);
                // load arguments
                yoi::vec<llvm::Value*> args;
                auto it = func->arg_begin();
                for (auto &arg : funcDecl->argumentTypes) {
                    args.push_back(it++);
                }
                // call function
                auto *mangledFunction = llvmModCtx.functionMap.at(mangledName);
                auto *result = llvmModCtx.Builder->CreateCall(mangledFunction, args, "result");
                // return with result
                llvmModCtx.Builder->CreateRet(result);
            } else {
                llvm::Type *returnType = yoiTypeToLLVMType(llvmModCtx, funcDecl->returnType, true);
                yoi::vec<llvm::Type*> argTypes;
                for (auto &argType : funcDecl->argumentTypes) {
                    argTypes.push_back(yoiTypeToLLVMType(llvmModCtx, argType, true)); // make sure all types converted
                }
                llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
                llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, yoi::wstring2string(funcName), llvmModCtx.TheModule.get());
                llvmModCtx.functionMap[funcName] = func;

                // add basic block
                llvm::BasicBlock *BB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", func);
                llvmModCtx.Builder->SetInsertPoint(BB);

                // load arguments
                yoi::vec<llvm::Value*> args;
                auto it = func->arg_begin();
                for (auto &arg : funcDecl->argumentTypes) {
                    yoi_assert(!arg->isArrayType() && !arg->isDynamicArrayType(), funcDecl->debugInfo.line, funcDecl->debugInfo.column, "Array return type not supported for foreign functions");
                    if (arg->isBasicType()) {
                        auto *argVal = createBasicObject(llvmModCtx, arg, it);
                        args.push_back(argVal);
                    } else {
                        auto handledLLVMType = handleForeignTypeConv(llvmModCtx, it, arg->typeIndex, 0, false); // convert to yoi type
                        args.push_back(handledLLVMType);
                    }
                    it ++;
                }
                // call function
                auto *mangledFunction = llvmModCtx.functionMap.at(mangledName);
                auto *result = llvmModCtx.Builder->CreateCall(mangledFunction, args, "result");
                llvm::Value *actualResultVal = nullptr;
                // convert result to foreign type
                yoi_assert(!funcDecl->returnType->isArrayType() && !funcDecl->returnType->isDynamicArrayType(), funcDecl->debugInfo.line, funcDecl->debugInfo.column, "Array return type not supported for foreign functions");
                if (funcDecl->returnType->isBasicType()) {
                    actualResultVal = unboxValue(llvmModCtx, result, funcDecl->returnType);
                } else {
                    actualResultVal = handleForeignTypeConv(llvmModCtx, result, funcDecl->returnType->typeIndex, 0, true); // convert back to foreign type
                }
                // resource releasing
                callGcFunction(llvmModCtx, result, funcDecl->returnType, false);
                for (auto &arg : funcDecl->argumentTypes) {
                    callGcFunction(llvmModCtx, args.back(), arg, false);
                    args.pop_back();
                }

                // return with actual result
                llvmModCtx.Builder->CreateRet(actualResultVal);
            }
        }

            
    }

    llvm::Value *LLVMCodegen::handleForeignTypeConv(LLVMModuleContext &llvmModCtx, llvm::Value *val, yoi::indexT foreignTypeIndex, yoi::indexT isArray, bool convertToForeign) {
        // get the foreign type
        auto &foreignType = compilerCtx->getIRFFITable()->foreignTypeTable[foreignTypeIndex];
        auto &originalType = yoiModule->structTable[foreignType->typeIndex];
        // get the llvm type
        auto llvmType = llvmModCtx.foreignTypeMap.at(std::make_tuple(IRValueType::valueType::structObject, foreignType->typeAffiliateModule, foreignType->typeIndex));
        auto objectLLVMType = llvmModCtx.structTypeMap.at(std::make_tuple(IRValueType::valueType::structObject, foreignType->typeAffiliateModule, foreignType->typeIndex));

        auto copyToOne = [&](llvm::Value *src, llvm::Value *dest) {
            // convert yoi type to foreign type
            for (yoi::indexT i = 0; i < originalType->fieldTypes.size(); i++) {
                // get the field value
                auto *fieldPtr = llvmModCtx.Builder->CreateStructGEP(objectLLVMType, val, i + 2, "field_ptr");
                auto *destFieldPtr = llvmModCtx.Builder->CreateStructGEP(llvmType, dest, i, "dest_field_ptr");
                auto &fieldType = originalType->fieldTypes[i];
                llvm::Value *fieldVal = nullptr;
                if (fieldType->isBasicType()) {
                    fieldVal = unboxValue(llvmModCtx, fieldPtr, fieldType);
                } else if (fieldType->isForeignBasicType()) {
                    fieldVal = handleForeignTypeConv(llvmModCtx, fieldPtr, fieldType, true);
                } else {
                    fieldVal = handleForeignTypeConv(llvmModCtx, fieldPtr, fieldType->typeIndex, 0, true);
                }
                // count field size
                auto size = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(yoiTypeToLLVMType(llvmModCtx, fieldType, true));
                // populate memory
                llvmModCtx.Builder->CreateMemCpy(destFieldPtr, llvm::MaybeAlign(8), fieldVal, llvm::MaybeAlign(8), size);
            }
        };

        if (convertToForeign) {
            llvm::Value *srcObjectToCopy = nullptr;
            llvm::Value *rawMemory = nullptr;

            if (isArray != 0) {
                // load value
                auto arrayLLVMType = llvmModCtx.arrayTypeMap.at(std::make_tuple(IRValueType::valueType::structObject, foreignType->typeAffiliateModule, foreignType->typeIndex, isArray));
                auto *loadedVal = llvmModCtx.Builder->CreateLoad(arrayLLVMType, val, "loaded_val");
                // offset to 2
                auto *arrayLength = llvmModCtx.Builder->CreateLoad(
                    llvmModCtx.Builder->getInt64Ty(),
                    llvmModCtx.Builder->CreateStructGEP(arrayLLVMType, loadedVal, 2, "array_length"),
                    "array_length_val"
                );

                rawMemory = llvmModCtx.Builder->CreateAlloca(llvmType, arrayLength, "yoi_to_foreign_alloca");

                for (yoi::indexT i = 0; i < isArray; i++) {
                    // get the array element
                    auto *element = loadArrayElement(llvmModCtx, managedPtr(IRValueType{
                        IRValueType::valueType::structObject,
                        foreignType->typeAffiliateModule,
                        foreignType->typeIndex,
                        {isArray}
                    }), val, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), i));
                    // copy to foreign type
                    auto *dest = llvmModCtx.Builder->CreateGEP(llvmType, rawMemory, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), i)});
                    copyToOne(element, dest);
                }
            } else {
                srcObjectToCopy = val;
                rawMemory = llvmModCtx.Builder->CreateAlloca(llvmType, nullptr, "yoi_to_foreign_alloca");
                copyToOne(srcObjectToCopy, rawMemory);
            }
            return rawMemory;
        } else {
            llvm::Value *rawMemory = llvmModCtx.Builder->CreateAlloca(objectLLVMType, nullptr, "foreign_to_yoi_alloca");
            // convert foreign type to yoi type
            for (yoi::indexT i = 0; i < originalType->fieldTypes.size(); i++) {
                // get the field value
                auto *fieldPtr = llvmModCtx.Builder->CreateStructGEP(llvmType, rawMemory, i + 2, "field_ptr");
                auto &fieldType = originalType->fieldTypes[i];
                llvm::Value *fieldVal = nullptr;
                if (fieldType->isBasicType()) {
                    auto *loadedFieldValue = llvmModCtx.Builder->CreateLoad(yoiTypeToLLVMType(llvmModCtx, fieldType, true), fieldPtr, "loaded_field_val");
                    fieldVal = createBasicObject(llvmModCtx, fieldType, fieldPtr);
                } else if (fieldType->isForeignBasicType()) {
                    fieldVal = handleForeignTypeConv(llvmModCtx, fieldPtr, fieldType, false);
                } else {
                    fieldVal = handleForeignTypeConv(llvmModCtx, fieldPtr, fieldType->typeIndex, 0, false);
                }
                // populate memory using store
                llvmModCtx.Builder->CreateStore(fieldVal, fieldPtr);
            }
            return rawMemory;
        }
    }

    void LLVMCodegen::generateMainFunction(LLVMModuleContext &llvmModCtx) {
        if (compilerCtx->getBuildConfig()->buildType == IRBuildConfig::BuildType::executable) {
            yoi::vec<llvm::Type*> argTypes = {
                llvm::Type::getInt32Ty(*llvmModCtx.TheContext),
                llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), 0)
            };
            llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvmModCtx.TheContext), argTypes, false);
            llvm::Function *elysiaMain = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "elysia_main", llvmModCtx.TheModule.get());
            llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", llvmModCtx.TheModule.get());
            llvmModCtx.functionMap[L"main"] = mainFunc;

            // add basic block
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", mainFunc);
            llvmModCtx.Builder->SetInsertPoint(BB);

            auto it = mainFunc->arg_begin();
            auto argc = it++;
            auto argv = it++;

            if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                // print starting message
                std::string startMsg = "Starting hoshi-lang program...\n";
                auto* startStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, startMsg, true);
                auto* startStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, startStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, startStrConst, "start_str");
                auto startArgs = std::array<llvm::Value*, 1>{ startStrGlobal };
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print"), llvm::ArrayRef<llvm::Value*>(startArgs));
                // print argc and argv by runtime_print_int and runtime_print_address
                // i32 to i64
                auto argc_i64 = llvmModCtx.Builder->CreateSExt(argc, llvmModCtx.Builder->getInt64Ty(), "argc_i64");
                // print argc
                auto argcArgs = std::array<llvm::Value*, 1>{ argc_i64 };
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print_int"), llvm::ArrayRef<llvm::Value*>(argcArgs));
                auto argvArgs = std::array<llvm::Value*, 1>{ argv };
                llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_print_address"), llvm::ArrayRef<llvm::Value*>(argvArgs));
            }

            // invoke elysia_main
            auto res = llvmModCtx.Builder->CreateCall(elysiaMain, {argc, argv}, "result");

            // return with result
            llvmModCtx.Builder->CreateRet(res);
        }
    }

    void LLVMCodegen::generateImportFunctionImplementations(LLVMModuleContext &llvmModCtx) {
        yoi::indexT moduleIndex = 0;
        for (auto &libraryPair : compilerCtx->getIRFFITable()->importedLibraries) {
            auto &libraryName = libraryPair.first;
            for (auto &functionPair : libraryPair.second.importedFunctionTable) {
                auto &funcName = functionPair.first;
                auto wrapperMangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName + L"#wrapper";
                auto mangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName;
                auto &funcDef = functionPair.second;
                bool noffi = funcDef->hasAttribute(IRFunctionDefinition::FunctionAttrs::NoFFI);
                auto &wrapperFuncDecl = llvmModCtx.functionMap[wrapperMangledName];
                auto &externFuncDecl = llvmModCtx.functionMap[mangledName];

                // generate wrapper function
                // create basic block
                if (!noffi) {
                    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", wrapperFuncDecl);
                    llvmModCtx.Builder->SetInsertPoint(BB);

                    if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                        // print function name
                        std::string funcName = wstring2string(funcDef->name);
                        auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, funcName, true);
                        auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                        auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                        llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_report_current_function"), llvm::ArrayRef<llvm::Value*>(debugArgs));
                    }

                    yoi_assert(!funcDef->returnType->isArrayType() && !funcDef->returnType->isDynamicArrayType(), funcDef->debugInfo.line, funcDef->debugInfo.column, "Array return type not supported for foreign functions");

                    yoi::vec<llvm::Value*> args;
                    auto it = wrapperFuncDecl->arg_begin();
                    for (auto &arg : funcDef->argumentTypes) {
                        if (arg->isForeignBasicType()) {
                            auto *argVal = handleForeignTypeConv(llvmModCtx, it, arg, true);
                            // callGcFunction(llvmModCtx, it, arg, false);  // no gc now, cause all raw value
                            args.push_back(argVal);
                        } else if (arg->isBasicType()) {
                            if (arg->isArrayType() || arg->isDynamicArrayType()) {
                                auto arrayLLVMType = getArrayLLVMType(llvmModCtx, arg);
                                auto *object = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(arrayLLVMType, 0), it, "loaded_arg");
                                // struct gep to array data
                                auto *arrayData = llvmModCtx.Builder->CreateStructGEP(arrayLLVMType, object, 3, "array_data");
                                // bitcast to pointer type
                                auto *arrayDataPtr = llvmModCtx.Builder->CreateBitCast(arrayData, llvm::PointerType::get(yoiTypeToLLVMType(llvmModCtx, managedPtr(arg->getElementType())), 0));
                                args.push_back(arrayDataPtr);
                            } else {
                                // auto *argVal = unboxValue(llvmModCtx, it, managedPtr(arg->getBasicRawType()));
                                // auto *argVal = llvmModCtx.Builder->CreateLoad(yoiTypeToLLVMType(llvmModCtx, arg, true), it, "loaded_arg");
                                args.push_back(it);
                            }
                        } else {
                            auto handledLLVMType = handleForeignTypeConv(llvmModCtx, it, arg->typeIndex, 0, true);
                            callGcFunction(llvmModCtx, it, arg, false);
                            args.push_back(handledLLVMType);
                        }
                        it++;
                    }

                    if (funcDef->returnType->type == IRValueType::valueType::none) {
                        llvmModCtx.Builder->CreateCall(externFuncDecl, args);
                        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                            // print function name
                            std::string funcName = wstring2string(funcDef->name);
                            auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, funcName, true);
                            auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                            auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_report_leave_function"), llvm::ArrayRef<llvm::Value*>(debugArgs));
                        }
                        llvmModCtx.Builder->CreateRetVoid();
                    } else {
                        auto result = llvmModCtx.Builder->CreateCall(externFuncDecl, args, "result");

                        llvm::Value *actualResultVal = nullptr;
                        if (funcDef->returnType->isForeignBasicType()) {
                            actualResultVal = handleForeignTypeConv(llvmModCtx, result, funcDef->returnType, false);
                        } else if (funcDef->returnType->isBasicType()) {
                            actualResultVal = result;
                        } else {
                            actualResultVal = handleForeignTypeConv(llvmModCtx, result, funcDef->returnType->typeIndex, 0, false); //convert back to yoi type
                        }
                        
                        if (compilerCtx->getBuildConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
                            // print function name
                            std::string funcName = wstring2string(funcDef->name);
                            auto* debugStrConst = llvm::ConstantDataArray::getString(*llvmModCtx.TheContext, funcName, true);
                            auto* debugStrGlobal = new llvm::GlobalVariable(*llvmModCtx.TheModule, debugStrConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, debugStrConst, "debug_str");
                            auto debugArgs = std::array<llvm::Value*, 1>{ debugStrGlobal };
                            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"runtime_debug_report_leave_function"), llvm::ArrayRef<llvm::Value*>(debugArgs));
                        }

                        // return with actual result
                        llvmModCtx.Builder->CreateRet(actualResultVal);
                    }
                }
            }
            moduleIndex ++;
        }
    }

    void LLVMCodegen::generateImportFunctionDeclarations(LLVMModuleContext &llvmModCtx) {
        yoi::indexT moduleIndex = 0;
        for (auto &libraryPair : compilerCtx->getIRFFITable()->importedLibraries) {
            auto &libraryName = libraryPair.first;
            if (libraryName != L"builtin")
                compilerCtx->getBuildConfig()->additionalLinkingFiles.push_back(libraryName);
            for (auto &functionPair : libraryPair.second.importedFunctionTable) {
                auto &funcName = functionPair.first;
                bool noffi = std::find(functionPair.second->attrs.begin(), functionPair.second->attrs.end(), IRFunctionDefinition::FunctionAttrs::NoFFI) != functionPair.second->attrs.end();

                // generate extern function first
                llvm::Type *returnType = yoiTypeToLLVMType(llvmModCtx, functionPair.second->returnType, !noffi);
                yoi::vec<llvm::Type*> argTypes;
                for (auto &argType : functionPair.second->argumentTypes) {
                    argTypes.push_back(yoiTypeToLLVMType(llvmModCtx, argType, !noffi)); // make sure all types converted
                }
                llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
                llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, yoi::wstring2string(funcName), llvmModCtx.TheModule.get());

                // add to function map
                auto mangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName;
                llvmModCtx.functionMap[mangledName] = func;

                // then generate wrapper function decl
                if (!noffi) {
                    auto wrapperReturnYoiType = normalizeForeignType(llvmModCtx, functionPair.second->returnType);
                    llvm::Type *wrapperReturnType = yoiTypeToLLVMType(llvmModCtx, wrapperReturnYoiType, wrapperReturnYoiType->isBasicType());
                    yoi::vec<llvm::Type*> wrapperArgTypes;
                    for (auto &argType : functionPair.second->argumentTypes) {
                        auto paramYoiType = normalizeForeignType(llvmModCtx, argType);  // normalize foreign int32 type to integerObject
                        if (paramYoiType->isBasicType()) {
                            // if parameter is a basic type, we pass it as raw value, so as reduce the FFI cost
                            wrapperArgTypes.push_back(yoiTypeToLLVMType(llvmModCtx, paramYoiType, true));
                        } else {
                            wrapperArgTypes.push_back(yoiTypeToLLVMType(llvmModCtx, paramYoiType, false)); // otherwise, object
                        }
                    }
                    llvm::FunctionType *wrapperFuncType = llvm::FunctionType::get(wrapperReturnType, wrapperArgTypes, false);
                    llvm::Function *wrapperFunc = llvm::Function::Create(wrapperFuncType, llvm::Function::ExternalLinkage, yoi::wstring2string(mangledName), llvmModCtx.TheModule.get());

                    // add to function map
                    auto wrapperMangledName = L"imported#" + std::to_wstring(moduleIndex) + L"#" + funcName + L"#wrapper";
                    llvmModCtx.functionMap[wrapperMangledName] = wrapperFunc;
                }
            }
            moduleIndex ++;
        }
    }

    std::shared_ptr<IRValueType>
    LLVMCodegen::normalizeForeignType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type) {
        switch (type->type) {
            case IRValueType::valueType::foreignFloatType: {
                return compilerCtx->getDeciObjectType();
            }
            case IRValueType::valueType::foreignInt32Type: {
                return compilerCtx->getIntObjectType();
            }
            case IRValueType::valueType::pointer:
            case IRValueType::valueType::pointerObject: {
                return compilerCtx->getUnsignedObjectType();
            }
            default: {
                return type;
            }
        }
    }

    llvm::Value *LLVMCodegen::handleForeignTypeConv(LLVMModuleContext &llvmModCtx, llvm::Value *val,
                                                    const std::shared_ptr<IRValueType> &foreignType,
                                                    bool convertToForeign) {
        yoi_assert(foreignType->isForeignBasicType(), 0, 0, "foreign type must be a basic type");
        switch (foreignType->type) {
            case IRValueType::valueType::foreignFloatType: {
                if (convertToForeign) {
                    // unbox double type and convert to float type
                    // since the default behaviour is changed, we now unbox raw value
                    // auto *doubleVal = unboxValue(llvmModCtx, val, managedPtr(compilerCtx->getDeciObjectType()->getBasicRawType()));
                    // auto *doubleVal = llvmModCtx.Builder->CreateLoad(llvm::Type::getDoubleTy(*llvmModCtx.TheContext), val, "double_val");
                    auto *floatVal = llvmModCtx.Builder->CreateFPTrunc(val, llvm::Type::getFloatTy(*llvmModCtx.TheContext), "float_val");
                    return floatVal;
                } else {
                    // convert float type to double type
                    auto *floatVal = llvmModCtx.Builder->CreateFPExt(val, llvm::Type::getDoubleTy(*llvmModCtx.TheContext), "float_val");
                    return floatVal;
                }
            }
            case IRValueType::valueType::foreignInt32Type: {
                if (convertToForeign) {
                    // unbox integer type and convert to int32 type
                    // auto *intVal = unboxValue(llvmModCtx, val, managedPtr(compilerCtx->getIntObjectType()->getBasicRawType()));
                    // auto *intVal = llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), val, "int_val");
                    auto *int32Val = llvmModCtx.Builder->CreateTrunc(val, llvm::Type::getInt32Ty(*llvmModCtx.TheContext), "int32_val");
                    return int32Val;
                } else {
                    // convert int32 type to integer type
                    auto *int32Val = llvmModCtx.Builder->CreateSExt(val, llvm::Type::getInt64Ty(*llvmModCtx.TheContext), "int32_val");
                    // create new object
                    // auto *newObj = createBasicObject(llvmModCtx, compilerCtx->getIntObjectType(), int32Val);
                    return int32Val;
                }
            }
            case IRValueType::valueType::pointer: {
                if (convertToForeign) {
                    // auto *ptrVal = unboxValue(llvmModCtx, val, managedPtr(compilerCtx->getUnsignedObjectType()->getBasicRawType()));
                    // auto *ptrVal = llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), val, "ptr_val");
                    // bit cast void*
                    auto *voidPtr = llvmModCtx.Builder->CreateIntToPtr(val, llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), 0), "void_ptr");
                    return voidPtr;
                } else {
                    // bitcast to i64
                    auto *voidPtr = llvmModCtx.Builder->CreateBitCast(val, llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), 0), "void_ptr");
                    auto *int64Val = llvmModCtx.Builder->CreatePtrToInt(voidPtr, llvm::Type::getInt64Ty(*llvmModCtx.TheContext), "int64_val");
                    // create new object
                    // auto *newObj = createBasicObject(llvmModCtx, compilerCtx->getIntObjectType(), int64Val);
                    return int64Val;
                }
            }
            default: {
                yoi_assert(false, 0, 0, "unsupported foreign type");
                return nullptr;
            }
        }
    }

    llvm::Type *LLVMCodegen::getArrayLLVMType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, bool enforceForeignType) {
        if (enforceForeignType) {
            return llvm::PointerType::get(yoiTypeToLLVMType(llvmModCtx, type, true), 0);
        } else {
            yoi::indexT size = 1;
            if (type->isArrayType()) {
                for (auto &i : type->dimensions) {
                    size *= i;
                }
            } else if (type->isDynamicArrayType()) {
                size = static_cast<yoi::indexT>(-1);
            }

            std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT> arrayKey = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex, size);
            if (auto it = llvmModCtx.arrayTypeMap.find(arrayKey); it!= llvmModCtx.arrayTypeMap.end()) {
                return it->second;
            }
            yoi::indexT arrayTypeId = -1;
            if (auto it = llvmModCtx.typeIDMap.find(arrayKey); it != llvmModCtx.typeIDMap.end()) {
                arrayTypeId = it->second;
            } else {
                arrayTypeId = llvmModCtx.nextTypeId++;
                llvmModCtx.typeIDMap[arrayKey] = arrayTypeId;
            }

            std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> structKey = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);

            llvm::Type *baseType = nullptr;
            switch (type->type) {
                case IRValueType::valueType::integerObject:
                    baseType = llvm::Type::getInt64Ty(*llvmModCtx.TheContext);
                    break;
                case IRValueType::valueType::decimalObject:
                    baseType = llvm::Type::getDoubleTy(*llvmModCtx.TheContext);
                    break;
                case IRValueType::valueType::unsignedObject:
                    baseType = llvm::Type::getInt64Ty(*llvmModCtx.TheContext);
                    break;
                case IRValueType::valueType::shortObject:
                    baseType = llvm::Type::getInt16Ty(*llvmModCtx.TheContext);
                    break;
                case IRValueType::valueType::booleanObject:
                    baseType = llvm::Type::getInt1Ty(*llvmModCtx.TheContext);
                    break;
                case IRValueType::valueType::characterObject:
                    baseType = llvm::Type::getInt8Ty(*llvmModCtx.TheContext);
                    break;
                case IRValueType::valueType::stringObject:
                    baseType = llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), 0);
                    break;
                case IRValueType::valueType::structObject:
                case IRValueType::valueType::interfaceObject:
                    baseType = llvm::PointerType::get(llvmModCtx.structTypeMap.at(structKey), 0); // only this is a object
                    break;
                default:
                    panic(0, 0, "LLVM Codegen: Unhandled or unmapped array type: " + std::string(magic_enum::enum_name(type->type)));
                    return nullptr;
            }
            auto arrayType = llvm::ArrayType::get(baseType, type->isArrayType() ? size : 1);
            // build struct with ref counter
            auto structType = llvm::StructType::create(*llvmModCtx.TheContext, yoi::vec<llvm::Type*>{
                llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // ref counter
                llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // type id
                llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // array length
                arrayType // array
            });
            auto fullStructName = "array_" + yoi::wstring2string(type->to_string()) + "_" + (type->isArrayType() ? std::to_string(size) : "dynamic");
            generateArrayGCFunctionDeclarations(llvmModCtx, type, structType, baseType);
            llvmModCtx.arrayToGenerateImplementations.emplace_back(type, structType, baseType);
            // add struct to struct map
            llvmModCtx.arrayTypeMap[arrayKey] = structType;
            // clean up the mess, reset the insert point
            return structType;
        }
    }

    llvm::Value *LLVMCodegen::createArrayObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                                const yoi::vec<StackValue> &elements) {
        yoi_assert(type->isArrayType(), 0, 0, "type must be an array type");
        llvm::Type *llvmType = getArrayLLVMType(llvmModCtx, type, false);
        // initialize the llvm struct, allocate memory and store the array
        auto memSize = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(llvmType);
        auto *memoryPointer = llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"object_alloc"), {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), memSize, true)});

        // increase the refcount to 1
        auto *refCounter = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 0, "ref_counter");
        auto *refCounterVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 1, true);
        llvmModCtx.Builder->CreateStore(refCounterVal, refCounter);

        yoi::indexT size = 1;
        for (auto &i : type->dimensions) {
            size *= i;
        }
        std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT, yoi::indexT> arrayKey = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex, size);
        auto typeId = llvmModCtx.typeIDMap.at(arrayKey);
        auto *typeIdPtr = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 1, "type_id_ptr");
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), typeId, true), typeIdPtr);

        // store the array length
        auto arrayLengthPtr = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 2, "array_length_ptr");
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), size, true), arrayLengthPtr);

        // store the array
        yoi::indexT index = 0;
        auto arrayBasePointer = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 3, "array_ptr");
        for (auto &i : elements) {
            // if basic type, unbox it first
            if (type->isBasicType()) {
                auto elementLLVMType = yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType()), true);
                auto arrayPointer = llvmModCtx.Builder->CreateGEP(elementLLVMType, arrayBasePointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), index)}, "array_element_ptr");
                auto val = unboxValue(llvmModCtx, i.llvmValue, i.yoiType);
                llvmModCtx.Builder->CreateStore(val, arrayPointer);
            } else {
                // otherwise, store the pointer directly
                auto arrayPointer = llvmModCtx.Builder->CreateGEP(llvm::PointerType::get(yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType())), 0), arrayBasePointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), index)}, "array_element_ptr");
                llvmModCtx.Builder->CreateStore(i.llvmValue, arrayPointer);
            }
            index ++;
        }
        return memoryPointer;
    }

    llvm::Value *
    LLVMCodegen::loadArrayElement(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, llvm::Value *arrayPtr, llvm::Value *index) {
        yoi_assert(type->isArrayType() || type->isDynamicArrayType(), 0, 0, "type must be an array type");
        llvm::Type *llvmType = getArrayLLVMType(llvmModCtx, type, false);

        auto *arrayPointer = llvmModCtx.Builder->CreateStructGEP(getArrayLLVMType(llvmModCtx, type), arrayPtr, 3, "array_ptr");
        switch (type->type) {
            case IRValueType::valueType::integerObject:
            case IRValueType::valueType::decimalObject:
            case IRValueType::valueType::booleanObject:
            case IRValueType::valueType::stringObject:
            case IRValueType::valueType::shortObject:
            case IRValueType::valueType::unsignedObject:
            case IRValueType::valueType::characterObject: {
                auto elementType = managedPtr(type->getElementType());
                auto elementLLVMType = yoiTypeToLLVMType(llvmModCtx, elementType, true);
                auto pointerToElement = llvmModCtx.Builder->CreateGEP(elementLLVMType, arrayPointer, {
                    index
                }, "element_ptr");
                auto loadedVal = llvmModCtx.Builder->CreateLoad(elementLLVMType, pointerToElement, "loaded_val"); // get unboxed value, so ffi type
                return loadedVal;
            }
            case IRValueType::valueType::structObject:
            case IRValueType::valueType::interfaceObject: {
                auto elementType = managedPtr(type->getElementType());
                if (type->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope))
                    elementType->addAttribute(IRValueType::ValueAttr::PermanentInCurrentScope);
                elementType->addAttribute(IRValueType::ValueAttr::Nullable);
                auto elementLLVMType = yoiTypeToLLVMType(llvmModCtx, elementType);
                auto pointerToElement = llvmModCtx.Builder->CreateGEP(elementLLVMType, arrayPointer, index, "element_ptr");
                auto *loadedVal = llvmModCtx.Builder->CreateLoad(yoiTypeToLLVMType(llvmModCtx, elementType), pointerToElement, "array_element_loaded_val");
                callGcFunction(llvmModCtx, loadedVal, elementType, true); // increase ref count
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

    llvm::DIType *LLVMCodegen::getDIType(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type) {
        if (llvmModCtx.basicDITypeMap.count(L"di_i8_ptr")) {
            // assume if one is there, all are (at least the ones we need at the start)
        } else {
            auto* di_i8 = llvmModCtx.DBuilder->createBasicType("char", 8, llvm::dwarf::DW_ATE_signed_char);
            auto* di_i64 = llvmModCtx.DBuilder->createBasicType("long long", 64, llvm::dwarf::DW_ATE_signed);
            
            llvmModCtx.basicDITypeMap[L"di_i8"] = di_i8;
            llvmModCtx.basicDITypeMap[L"di_i16"] = llvmModCtx.DBuilder->createBasicType("short", 16, llvm::dwarf::DW_ATE_signed);
            llvmModCtx.basicDITypeMap[L"di_i64"] = di_i64;
            llvmModCtx.basicDITypeMap[L"di_i64_u"] = llvmModCtx.DBuilder->createBasicType("unsigned long long", 64, llvm::dwarf::DW_ATE_unsigned);
            llvmModCtx.basicDITypeMap[L"di_double"] = llvmModCtx.DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
            llvmModCtx.basicDITypeMap[L"di_i1"] = llvmModCtx.DBuilder->createBasicType("bool", 8, llvm::dwarf::DW_ATE_boolean);
            llvmModCtx.basicDITypeMap[L"di_i8_ptr"] = llvmModCtx.DBuilder->createPointerType(di_i8, 64);

            auto* unknown_object_struct = llvmModCtx.DBuilder->createStructType(
                llvmModCtx.compileUnits[L"builtin"],
                "unknown_object",
                llvmModCtx.compileUnits[L"builtin"]->getFile(),
                1,
                64 + 64,
                64,
                llvm::DINode::FlagZero,
                nullptr,
                llvmModCtx.DBuilder->getOrCreateArray({
                    llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "refcount", nullptr, 0, 64, 64, 0, llvm::DINode::FlagZero, di_i64),
                    llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "typeid", nullptr, 0, 64, 64, 64, llvm::DINode::FlagZero, di_i64),
                })
            );
            llvmModCtx.basicDITypeMap[L"di_unknown_object_ptr"] = llvmModCtx.DBuilder->createPointerType(unknown_object_struct, 64);
        }

        auto* di_i64_u = llvmModCtx.basicDITypeMap[L"di_i64_u"];
        auto* di_i64 = llvmModCtx.basicDITypeMap[L"di_i64"];
        auto* di_double = llvmModCtx.basicDITypeMap[L"di_double"];
        auto* di_i1 = llvmModCtx.basicDITypeMap[L"di_i1"];
        auto* di_i16 = llvmModCtx.basicDITypeMap[L"di_i16"];
        auto* di_i8 = llvmModCtx.basicDITypeMap[L"di_i8"];
        auto* di_i8_ptr = llvmModCtx.basicDITypeMap[L"di_i8_ptr"];
        auto* di_unknown_object_ptr = llvmModCtx.basicDITypeMap[L"di_unknown_object_ptr"];

        if (type->isArrayType() || type->isDynamicArrayType()) {
            yoi::indexT size = 1;
            llvm::SmallVector<llvm::Metadata*, 8> dimensions;
            if (type->isArrayType()) {
                for (auto &i : type->dimensions) {
                    size *= i;
                    dimensions.push_back(llvmModCtx.DBuilder->getOrCreateSubrange(0, i));
                }
            } else {
                dimensions.push_back(llvmModCtx.DBuilder->getOrCreateSubrange(0, static_cast<int64_t>(0)));
            }
            
            auto arrayKey = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex, type->isArrayType() ? size : static_cast<yoi::indexT>(-1));
            if (llvmModCtx.arrayTypeDIMap.count(arrayKey)) {
                return llvmModCtx.arrayTypeDIMap[arrayKey];
            }

            llvm::DIType *elementDIType = nullptr;
            if (type->isBasicType()) {
                switch (type->type) {
                    case IRValueType::valueType::integerObject:
                        elementDIType = di_i64;
                        break;
                    case IRValueType::valueType::decimalObject:
                        elementDIType = di_double;
                        break;
                    case IRValueType::valueType::booleanObject:
                        elementDIType = di_i1;
                        break;
                    case IRValueType::valueType::characterObject:
                        elementDIType = di_i8;
                        break;
                    case IRValueType::valueType::stringObject:
                        elementDIType = di_i8_ptr;
                        break;
                    case IRValueType::valueType::unsignedObject:
                        elementDIType = di_i64_u;
                        break;
                    case IRValueType::valueType::shortObject:
                        elementDIType = di_i16;
                        break;
                    default:
                        panic(0, 0, "LLVM Codegen: Unhandled or unmapped array element type: " + std::string(magic_enum::enum_name(type->type)));
                        break;
                }
            } else {
                elementDIType = getDIType(llvmModCtx, managedPtr(type->getElementType()));
            }

            auto arraySizeInBits = size * llvmModCtx.TheModule->getDataLayout().getTypeSizeInBits(yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType())));
            auto* diArray = llvmModCtx.DBuilder->createArrayType(arraySizeInBits, 64, elementDIType, {llvmModCtx.DBuilder->getOrCreateArray(dimensions)});
            auto *diArrayStruct = llvmModCtx.DBuilder->createStructType(
                llvmModCtx.compileUnits[L"builtin"],
                "array_" + wstring2string(type->to_string()),
                llvmModCtx.compileUnits[L"builtin"]->getFile(),
                1,
                64 + 64 + 64 + arraySizeInBits,
                64,
                llvm::DINode::FlagZero,
                nullptr,
                llvmModCtx.DBuilder->getOrCreateArray({
                    llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "refcount", nullptr, 0, 64, 64, 0, llvm::DINode::FlagZero, di_i64),
                    llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "typeid", nullptr, 0, 64, 64, 64, llvm::DINode::FlagZero, di_i64),
                    llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "array_length", nullptr, 0, 64, 64, 128, llvm::DINode::FlagZero, di_i64),
                    llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "array", nullptr, 0, arraySizeInBits, 64, 192, llvm::DINode::FlagZero, diArray)
                })
            );
            auto *resultDIType = llvmModCtx.DBuilder->createPointerType(diArrayStruct, 64);
            llvmModCtx.arrayTypeDIMap[arrayKey] = resultDIType;
            return resultDIType;
        } else {
            if (type->isBasicType() && type->hasAttribute(IRValueType::ValueAttr::Raw)) {
                switch (type->type) {
                    case IRValueType::valueType::integerObject:
                        return di_i64;
                    case IRValueType::valueType::decimalObject:
                        return di_double;
                    case IRValueType::valueType::booleanObject:
                        return di_i1;
                    case IRValueType::valueType::characterObject:
                        return di_i8;
                    case IRValueType::valueType::stringObject:
                        return di_i8_ptr;
                    case IRValueType::valueType::unsignedObject:
                        return di_i64_u;
                    case IRValueType::valueType::shortObject:
                        return di_i16;
                    default:
                        panic(0, 0, "LLVM Codegen: Unhandled or unmapped raw type: " + std::string(magic_enum::enum_name(type->type)));
                        break;
                }
            }
            auto key = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex);
            if (llvmModCtx.structTypeDIMap.count(key)) {
                // printf("Existing Type Identifier: %lld %lld %lld, leave.\n", type->type, type->typeAffiliateModule, type->typeIndex);
                return llvmModCtx.structTypeDIMap[key];
            }
            // printf("Current Type Identifier: %lld %lld %lld\n", type->type, type->typeAffiliateModule, type->typeIndex);

            std::string typeName = "yoi." + wstring2string(type->to_string());

            llvm::DICompositeType *diFwdDecl = llvmModCtx.DBuilder->createReplaceableCompositeType(
                llvm::dwarf::DW_TAG_structure_type,
                typeName,
                llvmModCtx.compileUnits[L"builtin"],
                llvmModCtx.compileUnits[L"builtin"]->getFile(),
                1 // Line number
            );

            auto* resultDIType = llvmModCtx.DBuilder->createPointerType(diFwdDecl, 64);
            llvmModCtx.structTypeDIMap[key] = resultDIType;

            // An array to hold the DITypes of the struct members.
            llvm::SmallVector<llvm::Metadata*, 8> MemberTypes;

            // All our objects start with a refcount.
            MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(
                llvmModCtx.compileUnits[L"builtin"],
                "refcount",
                nullptr,
                0,
                64,
                64,
                0,
                llvm::DINode::FlagZero,
                di_i64
            ));
            MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(
                llvmModCtx.compileUnits[L"builtin"],
                "typeid",
                nullptr,
                0,
                64,
                64,
                64,
                llvm::DINode::FlagZero,
                di_i64
            ));
            uint64_t currentSize = 128; // Keep track of struct size

            // 2. Generate the body of the type based on the Yoi type.
            switch (type->type) {
                case IRValueType::valueType::integerObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 64, 64, currentSize, llvm::DINode::FlagZero, di_i64));
                    currentSize += 64;
                    break;
                }
                case IRValueType::valueType::stringObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 64, 64, currentSize, llvm::DINode::FlagZero, di_i8_ptr));
                    currentSize += 64;
                    break;
                }
                case IRValueType::valueType::decimalObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 64, 64, currentSize, llvm::DINode::FlagZero, di_double));
                    currentSize += 64;
                    break;
                }
                case IRValueType::valueType::booleanObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 8, 8, currentSize, llvm::DINode::FlagZero, di_i1));
                    currentSize += 8;
                    break;
                }
                case IRValueType::valueType::characterObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 8, 8, currentSize, llvm::DINode::FlagZero, di_i8));
                    currentSize += 8;
                    break;
                }
                case IRValueType::valueType::unsignedObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 64, 64, currentSize, llvm::DINode::FlagZero, di_i64_u));
                    currentSize += 64;
                    break;
                }
                case IRValueType::valueType::shortObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 16, 16, currentSize, llvm::DINode::FlagZero, di_i16));
                    currentSize += 16;
                    break;
                }
                case IRValueType::valueType::structObject: {
                    auto structDef = yoiModule->structTable[type->typeIndex];
                    yoi::vec<std::string> fieldNames(structDef->fieldTypes.size());
                    for (auto it = structDef->nameIndexMap.begin(); it!= structDef->nameIndexMap.end(); ++it) {
                        if (it->second.type == IRStructDefinition::nameInfo::nameType::method)
                            continue;
                        auto fieldName = wstring2string(it->first);
                        fieldNames[it->second.index] = fieldName;
                    }
                    for (yoi::indexT i = 0; i < fieldNames.size(); i++) {
                        auto fieldYoiType = structDef->fieldTypes[i];
                        // Recursively get the DIType for the field.
                        auto* fieldDIType = getDIType(llvmModCtx, fieldYoiType);
                        uint64_t fieldSize = llvmModCtx.TheModule->getDataLayout().getTypeSizeInBits(yoiTypeToLLVMType(llvmModCtx, fieldYoiType));

                        MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(
                            llvmModCtx.compileUnits[L"builtin"], fieldNames[i], nullptr, 0,
                            fieldSize, fieldSize, currentSize,
                            llvm::DINode::FlagZero, fieldDIType
                        ));
                        currentSize += fieldSize;
                    }
                    break;
                }
                case IRValueType::valueType::interfaceObject: {
                    MemberTypes.push_back(llvmModCtx.DBuilder->createMemberType(llvmModCtx.compileUnits[L"builtin"], "value", nullptr, 0, 64, 64, currentSize, llvm::DINode::FlagZero, di_unknown_object_ptr));
                    currentSize += 64;
                    break;
                }
                case IRValueType::valueType::none: 
                default: {
                    // 'none' object only has a refcount.
                    break;
                }
            }

            // 3. Create the DIStructType for the object itself.
            auto* diStruct = llvmModCtx.DBuilder->createStructType(
                llvmModCtx.compileUnits[L"builtin"], // Scope
                typeName,
                llvmModCtx.compileUnits[L"builtin"]->getFile(), // File
                1, // Line number (can be 0)
                currentSize, // Size in bits
                64, // Alignment in bits
                llvm::DINode::FlagZero,
                nullptr, // Derived from
                llvmModCtx.DBuilder->getOrCreateArray(MemberTypes)
            );

            auto node = llvm::TempMDNode(diFwdDecl);
            llvmModCtx.DBuilder->replaceTemporary(std::move(node), diStruct);
            // diFwdDecl->replaceAllUsesWith(diStruct);
            // llvm::errs() << "  [" << typeName << "] replaceTemporary returned FinalNode: " << finalNode << "\n";

            return resultDIType;
        }
    }

    void LLVMCodegen::generateRTTIImplmentation(LLVMModuleContext &llvmModCtx) {
        // Generate the RTTI for the Yoi types.
        yoi::vec<llvm::Constant *> rttiFields(llvmModCtx.typeIDMap.size());
        auto RTTITableType = llvm::ArrayType::get(llvmModCtx.RTTIEntryType, llvmModCtx.typeIDMap.size());
        for (auto &typeIndexPair : llvmModCtx.typeIDMap) {
            auto typeId = typeIndexPair.second;
            std::string typenameString;
            auto yoiType = yoi::IRValueType{std::get<0>(typeIndexPair.first), std::get<1>(typeIndexPair.first), std::get<2>(typeIndexPair.first)};
            if (yoiType.isBasicType()) {
                typenameString = yoi::wstring2string(yoiType.to_string());
            } else if (yoiType.type == IRValueType::valueType::structObject) {
                typenameString = yoi::wstring2string(yoiModule->structTable[std::get<2>(typeIndexPair.first)]->name);
            } else if (yoiType.type == IRValueType::valueType::interfaceObject) {
                typenameString = yoi::wstring2string(yoiModule->interfaceTable[std::get<2>(typeIndexPair.first)]->name);
            }
            if (std::get<3>(typeIndexPair.first) != 0) {
                if (std::get<3>(typeIndexPair.first) == static_cast<yoi::indexT>(-1)) {
                    typenameString += "[]";
                } else {
                    typenameString +=  "[" + std::to_string(std::get<3>(typeIndexPair.first)) + "]";
                }
            }

            std::array<llvm::Constant *, 6> rtti_entry_field{
                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), typeId),
                llvmModCtx.Builder->CreateGlobalString(typenameString, "rtti_type_name"),
                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), static_cast<yoi::indexT>(std::get<0>(typeIndexPair.first))),
                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), std::get<1>(typeIndexPair.first)),
                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), std::get<2>(typeIndexPair.first)),
                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), std::get<3>(typeIndexPair.first)),
            };
            rttiFields[typeId] = llvm::ConstantStruct::get(llvmModCtx.RTTIEntryType, rtti_entry_field);
        }
        auto RTTIConstantDataArray = llvm::ConstantArray::get(RTTITableType, rttiFields);
        llvmModCtx.RTTITable->setInitializer(RTTIConstantDataArray);
    }

    void LLVMCodegen::generateRTTIDeclaration(LLVMModuleContext &llvmModCtx) {
        for (auto &funcPair : yoiModule->functionTable) {
            auto funcDef = funcPair.second;
            for (auto &blocks : funcDef->codeBlock) {
                for (auto &ins : blocks->getIRArray()) {
                    switch (ins.opcode) {
                        case IR::Opcode::new_array_bool:
                        case IR::Opcode::new_array_int:
                        case IR::Opcode::new_array_deci:
                        case IR::Opcode::new_array_str:
                        case IR::Opcode::new_array_unsigned:
                        case IR::Opcode::new_array_short:
                        case IR::Opcode::new_array_char: {
                            std::shared_ptr<IRValueType> elementType;
                            switch (ins.opcode) {
                                case IR::Opcode::new_array_int:
                                    elementType = compilerCtx->getIntObjectType();
                                    break;
                                case IR::Opcode::new_array_deci:
                                    elementType = compilerCtx->getDeciObjectType();
                                    break;
                                case IR::Opcode::new_array_bool:
                                    elementType = compilerCtx->getBoolObjectType();
                                    break;
                                case IR::Opcode::new_array_str:
                                    elementType = compilerCtx->getStrObjectType();
                                    break;
                                case IR::Opcode::new_array_char:
                                    elementType = compilerCtx->getCharObjectType();
                                    break;
                                case IR::Opcode::new_array_short:
                                    elementType = compilerCtx->getShortObjectType();
                                    break;
                                case IR::Opcode::new_array_unsigned:
                                    elementType = compilerCtx->getUnsignedObjectType();
                                    break;
                                default:
                                    break;
                            }
                            yoi::vec<yoi::indexT> dims;
                            for (yoi::indexT i = 1;i < ins.operands.size(); i++) {
                                dims.push_back(ins.operands[i].value.symbolIndex);
                            }
                            getArrayLLVMType(llvmModCtx, managedPtr(elementType->getArrayType(dims)));
                            break;
                        }
                        case IR::Opcode::new_array_interface:
                        case IR::Opcode::new_array_struct: {
                            yoi::vec<yoi::indexT> dims;
                            for (yoi::indexT i = 3;i < ins.operands.size(); i++) {
                                dims.push_back(ins.operands[i].value.symbolIndex);
                            }
                            auto arrayType = managedPtr(IRValueType{
                                ins.opcode == IR::Opcode::new_array_struct ? IRValueType::valueType::structObject : IRValueType::valueType::interfaceObject,
                                ins.operands[0].value.symbolIndex,
                                ins.operands[1].value.symbolIndex,
                                dims
                            });
                            getArrayLLVMType(llvmModCtx, arrayType);
                            break;
                        }
                        case IR::Opcode::new_dynamic_array_bool:
                        case IR::Opcode::new_dynamic_array_int:
                        case IR::Opcode::new_dynamic_array_deci:
                        case IR::Opcode::new_dynamic_array_str:
                        case IR::Opcode::new_dynamic_array_unsigned:
                        case IR::Opcode::new_dynamic_array_short:
                        case IR::Opcode::new_dynamic_array_char: {
                            std::shared_ptr<IRValueType> elementType;
                            switch (ins.opcode) {
                                case IR::Opcode::new_dynamic_array_int:
                                    elementType = compilerCtx->getIntObjectType();
                                    break;
                                case IR::Opcode::new_dynamic_array_deci:
                                    elementType = compilerCtx->getDeciObjectType();
                                    break;
                                case IR::Opcode::new_dynamic_array_bool:
                                    elementType = compilerCtx->getBoolObjectType();
                                    break;
                                case IR::Opcode::new_dynamic_array_str:
                                    elementType = compilerCtx->getStrObjectType();
                                    break;
                                case IR::Opcode::new_dynamic_array_char:
                                    elementType = compilerCtx->getCharObjectType();
                                    break;
                                case IR::Opcode::new_dynamic_array_unsigned:
                                    elementType = compilerCtx->getUnsignedObjectType();
                                    break;
                                case IR::Opcode::new_dynamic_array_short:
                                    elementType = compilerCtx->getShortObjectType();
                                    break;
                                default:
                                    break;
                            }
                            getArrayLLVMType(llvmModCtx, managedPtr(elementType->getDynamicArrayType()));
                            break;
                        }
                        case IR::Opcode::new_dynamic_array_interface:
                        case IR::Opcode::new_dynamic_array_struct: {
                            auto arrayType = managedPtr(IRValueType{
                                ins.opcode == IR::Opcode::new_dynamic_array_struct ? IRValueType::valueType::structObject : IRValueType::valueType::interfaceObject,
                                ins.operands[0].value.symbolIndex,
                                ins.operands[1].value.symbolIndex,
                                {static_cast<yoi::indexT>(-1)}
                            });
                            getArrayLLVMType(llvmModCtx, arrayType);
                            break;
                        }
                        default: break;
                    }
                }
            }
        }

        llvmModCtx.RTTIEntryType = llvm::StructType::get(*llvmModCtx.TheContext, {
            llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // type id
            llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmModCtx.TheContext), 0), // type name
            llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // type enum
            llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // type affiliate module
            llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // type index
            llvm::Type::getInt64Ty(*llvmModCtx.TheContext), // array size if provided, otherwise 0
        });
        auto RTTITableType = llvm::ArrayType::get(llvmModCtx.RTTIEntryType, llvmModCtx.typeIDMap.size());
        llvmModCtx.RTTITable = new llvm::GlobalVariable(*llvmModCtx.TheModule, RTTITableType, true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, nullptr, "rtti_table");
        is_rtti_table_frozen = true;
    }

    llvm::Value *LLVMCodegen::createDynamicArrayObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                                       const yoi::vec<StackValue> &elements,
                                                       llvm::Value *size) {
        yoi_assert(type->isDynamicArrayType(), 0, 0, "type must be an dynamic array type");
        llvm::Type *llvmType = getArrayLLVMType(llvmModCtx, type);
        auto key = std::make_tuple(type->type, type->typeAffiliateModule, type->typeIndex, type->dimensions.back()); // dims back should always be -1
        auto memSize = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(llvmType);
        auto elementSize = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(llvmModCtx.arrayTypeMap[key]->getElementType(3));
        memSize -= elementSize; // pure header length

        // now calculate the total size of the array
        llvm::Value *totalSize = llvmModCtx.Builder->CreateAdd(
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), memSize),
            llvmModCtx.Builder->CreateMul(size, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), elementSize), "array_size"),
            "total_dyn_array_size"
        );
        // allocate memory
        auto *memoryPointer = llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"object_alloc"), {totalSize});
        // increase the refcount to 1
        auto *refCounter = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 0, "ref_counter");
        auto *refCounterVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 1, true);
        llvmModCtx.Builder->CreateStore(refCounterVal, refCounter);
        // store the type id
        auto typeId = llvmModCtx.typeIDMap.at(key);
        auto *typeIdPtr = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 1, "type_id_ptr");
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), typeId, true), typeIdPtr);
        // store array length
        auto arrayLengthPtr = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 2, "array_length_ptr");
        llvmModCtx.Builder->CreateStore(size, arrayLengthPtr);
        // store array elements
        auto arrayBasePointer = llvmModCtx.Builder->CreateStructGEP(llvmType, memoryPointer, 3, "array_ptr");
        auto index = 0;
        for (auto &element : elements) {
            if (type->isBasicType()) {
                auto elementLLVMType = yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType()), true);
                auto arrayPointer = llvmModCtx.Builder->CreateGEP(elementLLVMType, arrayBasePointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), index)}, "array_element_ptr");
                auto val = unboxValue(llvmModCtx, element.llvmValue, element.yoiType);
                llvmModCtx.Builder->CreateStore(val, arrayPointer);
            } else {
                // otherwise, store the pointer directly
                auto arrayPointer = llvmModCtx.Builder->CreateGEP(llvm::PointerType::get(yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType())), 0), arrayBasePointer, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), index)}, "array_element_ptr");
                llvmModCtx.Builder->CreateStore(element.llvmValue, arrayPointer);
            }
            index ++;
        }
        return memoryPointer;
    }

    void LLVMCodegen::generateArrayGCFunctionDeclarations(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, llvm::StructType *structType, llvm::Type *baseType) {
        // create gc function
        auto incFuncName = "array_" + yoi::wstring2string(type->to_string()) + "_gc_refcount_increase";
        auto decFuncName = "array_" + yoi::wstring2string(type->to_string()) + "_gc_refcount_decrease";

        auto currentInsertPoint = llvmModCtx.Builder->GetInsertBlock();

        if (auto it = llvmModCtx.functionMap.find(yoi::string2wstring(incFuncName)) == llvmModCtx.functionMap.end()) {
            auto gcIncFuncType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(*llvmModCtx.TheContext), {llvm::PointerType::get(structType, 0)}, false);
            auto gcIncFunc =
                llvm::Function::Create(gcIncFuncType, llvm::Function::ExternalLinkage, incFuncName, llvmModCtx.TheModule.get());
            llvmModCtx.functionMap[yoi::string2wstring(incFuncName)] = gcIncFunc;
        }
        if (auto it = llvmModCtx.functionMap.find(yoi::string2wstring(decFuncName)) == llvmModCtx.functionMap.end()) {
            auto gcDecFuncType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(*llvmModCtx.TheContext), {llvm::PointerType::get(structType, 0)}, false);
            auto gcDecFunc =
                llvm::Function::Create(gcDecFuncType, llvm::Function::ExternalLinkage, decFuncName, llvmModCtx.TheModule.get());
            llvmModCtx.functionMap[yoi::string2wstring(decFuncName)] = gcDecFunc;
        }
        llvmModCtx.Builder->SetInsertPoint(currentInsertPoint);
    }

    void LLVMCodegen::storeArrayElement(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                        const std::shared_ptr<IRValueType> &valueToStoreType,
                                        llvm::Value *arrayPtr,
                                        llvm::Value *index,
                                        llvm::Value *value) {
        auto arrayLLVMType = getArrayLLVMType(llvmModCtx, type);
        if (type->isBasicType()) {
            auto elementLLVMType = yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType()), true);
            auto basePointer = llvmModCtx.Builder->CreateStructGEP(arrayLLVMType, arrayPtr, 3, "array_ptr");
            auto elementPointer = llvmModCtx.Builder->CreateGEP(elementLLVMType, basePointer, {index}, "array_element_ptr");
            auto val = unboxValue(llvmModCtx, value, valueToStoreType);
            llvmModCtx.Builder->CreateStore(val, elementPointer);
        } else {
            // otherwise, store the pointer directly
            auto basePointer = llvmModCtx.Builder->CreateStructGEP(arrayLLVMType, arrayPtr, 3, "array_ptr");
            auto elementPointer = llvmModCtx.Builder->CreateGEP(llvm::PointerType::get(yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType())), 0), basePointer, {index}, "array_element_ptr");
            auto loadedPointer = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(yoiTypeToLLVMType(llvmModCtx, managedPtr(type->getElementType())), 0), elementPointer, "loaded_pointer");
            callGcFunction(llvmModCtx, loadedPointer, managedPtr(type->getElementType().addAttribute(IRValueType::ValueAttr::Nullable)), false);

            llvmModCtx.Builder->CreateStore(value, elementPointer);
            // increase the ref count of the object
            callGcFunction(llvmModCtx, value, valueToStoreType, true);
        }
    }

    void LLVMCodegen::generateArrayGCFunctionImplementations(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                                                          llvm::StructType *structType,
                                                                          llvm::Type *baseType) {
        // create gc function
        auto incFuncName = "array_" + yoi::wstring2string(type->to_string()) + "_gc_refcount_increase";
        auto decFuncName = "array_" + yoi::wstring2string(type->to_string()) + "_gc_refcount_decrease";

        auto currentInsertPoint = llvmModCtx.Builder->GetInsertBlock();

        {
            auto gcIncFunc = llvmModCtx.functionMap[yoi::string2wstring(incFuncName)];
            gcIncFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
            // add basic block
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", gcIncFunc);
            llvmModCtx.Builder->SetInsertPoint(BB);
            auto *objPtr = gcIncFunc->arg_begin();
            auto *refCounter = llvmModCtx.Builder->CreateStructGEP(structType, objPtr, 0, "ref_counter");
            auto *newRefCounter =
                llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), refCounter, "new_ref_counter");
            auto *newRefCounterVal =
                llvmModCtx.Builder->CreateAdd(newRefCounter,
                                   llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 1, true),
                                   "new_ref_counter_val");
            llvmModCtx.Builder->CreateStore(newRefCounterVal, refCounter);
            llvmModCtx.Builder->CreateRetVoid();
        }
        {
            auto gcDecFunc = llvmModCtx.functionMap[yoi::string2wstring(decFuncName)];
            gcDecFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
            // add basic block
            auto BB = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "entry", gcDecFunc);
            auto nullFailedBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "null_failed", gcDecFunc);
            auto continueBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "continue", gcDecFunc);
            auto finalizeBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "finalize", gcDecFunc);
            auto retBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "ret", gcDecFunc);

            llvmModCtx.Builder->SetInsertPoint(BB);

            auto objPtr = gcDecFunc->arg_begin();
            auto refCounter = llvmModCtx.Builder->CreateStructGEP(structType, objPtr, 0, "ref_counter");
            // check whether object is null
            auto *isObjNull = llvmModCtx.Builder->CreateIsNull(objPtr, "is_obj_null");
            llvmModCtx.Builder->CreateCondBr(isObjNull, nullFailedBlock, continueBlock);

            llvmModCtx.Builder->SetInsertPoint(continueBlock);
            auto newRefCounter =
                llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), refCounter, "new_ref_counter");
            auto newRefCounterVal =
                llvmModCtx.Builder->CreateSub(newRefCounter,
                                   llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 1, true),
                                   "new_ref_counter_val");
            llvmModCtx.Builder->CreateStore(newRefCounterVal, refCounter);

            auto icmpRes = llvmModCtx.Builder->CreateICmpEQ(newRefCounterVal,
                                                 llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 0, true),
                                                 "ref_counter_zero");
            llvmModCtx.Builder->CreateCondBr(icmpRes, finalizeBlock, retBlock);
            // ret block
            llvmModCtx.Builder->SetInsertPoint(retBlock);
            llvmModCtx.Builder->CreateRetVoid();
            // null failed block
            llvmModCtx.Builder->SetInsertPoint(nullFailedBlock);
            llvmModCtx.Builder->CreateRetVoid();
            // finalize block
            llvmModCtx.Builder->SetInsertPoint(finalizeBlock);
            // free memory
            if (type->type == IRValueType::valueType::structObject ||
                type->type == IRValueType::valueType::interfaceObject) {
                // decrease the ref count of array elements inside
                auto arrayPointer = llvmModCtx.Builder->CreateStructGEP(structType, objPtr, 3, "array_ptr");
                auto arrayLengthPtr = llvmModCtx.Builder->CreateStructGEP(structType, objPtr, 2, "array_length_ptr");
                auto arrayLength =
                    llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), arrayLengthPtr, "array_length");
                auto currentIndex =
                    llvmModCtx.Builder->CreateAlloca(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), nullptr, "current_index");

                llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 0, true),
                                     currentIndex);
                auto loopBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "loop", gcDecFunc);
                auto exitBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "exit", gcDecFunc);
                auto condBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "cond", gcDecFunc);
                llvmModCtx.Builder->CreateBr(loopBlock);
                llvmModCtx.Builder->SetInsertPoint(condBlock);
                auto *loopCond = llvmModCtx.Builder->CreateICmpSLT(
                    llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), currentIndex), arrayLength, "loop_cond");
                llvmModCtx.Builder->CreateCondBr(loopCond, loopBlock, exitBlock);
                // loop block
                llvmModCtx.Builder->SetInsertPoint(loopBlock);
                auto elementPointer =
                    llvmModCtx.Builder->CreateGEP(llvm::PointerType::get(baseType, 0),
                                       arrayPointer,
                                       {llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), currentIndex)},
                                       "element_ptr");
                auto elementPointerVal =
                    llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 0),
                                        elementPointer,
                                        "element_ptr_val"); // just too lazy, so I use int64*
                callGcFunction(llvmModCtx, elementPointerVal, managedPtr(type->getElementType().addAttribute(IRValueType::ValueAttr::Nullable)), false);
                auto nextIndex = llvmModCtx.Builder->CreateLoad(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), currentIndex, "next_index");
                auto nextIndexVal = llvmModCtx.Builder->CreateAdd(
                    nextIndex, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmModCtx.TheContext), 1, true), "next_index_val");
                llvmModCtx.Builder->CreateStore(nextIndexVal, currentIndex);
                llvmModCtx.Builder->CreateBr(condBlock);
                // exit block
                llvmModCtx.Builder->SetInsertPoint(exitBlock);
            }
            llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"finalize_object"), objPtr);
            llvmModCtx.Builder->CreateRetVoid();
        }
        llvmModCtx.Builder->SetInsertPoint(currentInsertPoint);
    }

    std::pair<std::shared_ptr<IRValueType>, llvm::Value *>
    LLVMCodegen::ensureObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type, llvm::Value *val) {
        if (type->hasAttribute(IRValueType::ValueAttr::Raw) || type->isBasicRawType()) {
            auto unboxedValue = unboxValue(llvmModCtx, val, type);
            auto boxedValue = createBasicObject(llvmModCtx, managedPtr(type->getBasicObjectType()), unboxedValue);
            return {managedPtr(type->getBasicObjectType()), boxedValue};
        } else {
            return {type, val};
        }
    }

    void LLVMCodegen::generateIfTargetNotNull(LLVMModuleContext &llvmModCtx, llvm::Value *objectPtr,
                                              const std::shared_ptr<IRValueType> &yoiType,
                                              const std::function<void()> &func, bool enforced) {
        if (!yoiType->hasAttribute(IRValueType::ValueAttr::Nullable) && !enforced) {
            func();
            return;
        }
        auto f = llvmModCtx.Builder->GetInsertBlock()->getParent();
        auto continueBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "if_continue", f);
        auto notNullBlock = llvm::BasicBlock::Create(*llvmModCtx.TheContext, "if_not_null", f);
        auto comparsion = llvmModCtx.Builder->CreateIsNotNull(objectPtr);
        llvmModCtx.Builder->CreateCondBr(comparsion, notNullBlock, continueBlock);
        llvmModCtx.Builder->SetInsertPoint(notNullBlock);
        func();
        llvmModCtx.Builder->CreateBr(continueBlock);
        llvmModCtx.Builder->SetInsertPoint(continueBlock);
    }

    LLVMCodegen::ValueStackWithPhi::ValueStackWithPhi(const ControlFlowAnalysis &cfa, llvm::IRBuilder<> *builder, const std::shared_ptr<IRModule> &yoiModule)
        : cfa(cfa), stackState(StackState::Finalized), currentState(0), builder(builder), yoiModule(yoiModule) {}
        
    void LLVMCodegen::ValueStackWithPhi::enterNode(yoi::indexT currentState,
                                                   yoi::indexT fromState,
                                                   llvm::BasicBlock *currentBlock,
                                                   llvm::BasicBlock *fromBlock) {
        yoi_assert(
            stackState == StackState::Finalized, 0, 0, "llvmCodegen: invoking enterNode on an unfinalized stack");
        stackState = StackState::InEvaluation;
        this->currentState = currentState;

        // check whether the first time to evaluate this block, if so, inherit the stack base from stack top of previous
        // block.
        if (auto it = valueStackStateIn.find(currentState) == valueStackStateIn.end()) {
            valueStackStateIn[currentState] = valueStackStateOut[fromState];
            phiNodes[currentState] = valueStackStateOut[fromState].empty() ? yoi::vec<llvm::PHINode *>{} : phiNodes[fromState];
            // also, for those which is not a phi node but exists in the previous block, create a new phi node for them.
            auto begin = phiNodes[currentState].size();
            for (yoi::indexT begins = phiNodes[currentState].size(); begins < valueStackStateIn[currentState].size(); begins++) {
                auto phiNode = builder->CreatePHI(valueStackStateIn[currentState][begins].llvmValue->getType(), cfa.reverseG.at(currentState).size(), "phi_node");
                phiNode->addIncoming(valueStackStateOut[fromState][begins].llvmValue, fromBlock); // definitely from the previous block.
                phiNodes[currentState].push_back(phiNode);
                valueStackStateIn[currentState][begins].llvmValue = phiNode;
            }
            valueStackStateOut[currentState] = valueStackStateIn[currentState];
        } else {
            // now is the second time to evaluate this block, merge all existing phi nodes from previous block into this
            // block. there would be a chance that the control path of two block, not only differs in the last frame,
            // but also in the middle of the frames, thus we need to iterate from the start. if the onward value collide
            // with the existing phi node, phi the phi node. also check whether the stack depth is the same, if not,
            // panic.
            yoi_assert(valueStackStateIn[currentState].size() == valueStackStateOut[fromState].size(),
                       0,
                       0,
                       "llvmCodegen: incompatiable control flow");
            for (yoi::indexT i = 0; i < phiNodes[currentState].size(); i++) {
                if (valueStackStateOut[fromState][i].llvmValue != phiNodes[currentState][i]) {
                    // merge phi nodes
                    phiNodes[currentState][i]->addIncoming(valueStackStateOut[fromState][i].llvmValue, fromBlock);

                    // merge variable metadata
                    if (valueStackStateIn[currentState][i].yoiType->metadata.hasMetadata(L"regressed_interface_impl") && valueStackStateOut[fromState][i].yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
                        auto implIndex1 = valueStackStateIn[currentState][i].yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                        auto implIndex2 = valueStackStateOut[fromState][i].yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                        auto implDef1 = yoiModule->interfaceImplementationTable[implIndex1.second];
                        auto implDef2 = yoiModule->interfaceImplementationTable[implIndex2.second];
                        if (implIndex1 != implIndex2) {
                            // conflict, remove metadata, and actualize the interface object.
                            // while both sides are fucked, we take the phi node as input node.
                            panic(0, 0, "llvmCodegen: interface implementation conflict");
                            // valueStackStateIn[currentState][i] = actualizeFunc(managedPtr(IRValueType{std::get<0>(implDef1->implStructIndex), std::get<1>(implDef1->implStructIndex), std::get<2>(implDef1->implStructIndex)}), valueStackStateIn[currentState][i].llvmValue, implIndex1.second);
                        }
                    } else if (valueStackStateIn[currentState][i].yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
                        auto implIndex1 = valueStackStateIn[currentState][i].yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                        auto implDef = yoiModule->interfaceImplementationTable[implIndex1.second];
                        // right side is plain, so we normalize the left side.
                        // valueStackStateIn[currentState][i] = actualizeFunc(managedPtr(IRValueType{std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex)}), valueStackStateIn[currentState][i].llvmValue, implIndex1.second);
                        panic(0, 0, "llvmCodegen: interface implementation conflict");
                    } else if (valueStackStateOut[fromState][i].yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
                        auto implIndex2 = valueStackStateOut[fromState][i].yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
                        auto implDef = yoiModule->interfaceImplementationTable[implIndex2.second];
                        // left side is plain, so we normalize the right side.
                        // valueStackStateOut[fromState][i] = actualizeFunc(managedPtr(IRValueType{std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex)}), valueStackStateOut[fromState][i].llvmValue, implIndex2.second);
                        panic(0, 0, "llvmCodegen: interface implementation conflict");
                    } else {
                        // both side is plain, we do nothing.
                    }
                }
            }
            valueStackStateOut[currentState] = valueStackStateIn[currentState];
        }
    }

    void LLVMCodegen::ValueStackWithPhi::finalizeNode() {
        yoi_assert(
            stackState == StackState::InEvaluation, 0, 0, "llvmCodegen: invoking finalizeNode on a finalized stack");
        stackState = StackState::Finalized;
    }

    void LLVMCodegen::ValueStackWithPhi::push_back(const StackValue &value) {
        valueStackStateOut[currentState].push_back(value);
    }

    LLVMCodegen::StackValue &LLVMCodegen::ValueStackWithPhi::back() {
        return valueStackStateOut[currentState].back();
    }

    void LLVMCodegen::ValueStackWithPhi::pop_back() {
        valueStackStateOut[currentState].pop_back();
    }

    void LLVMCodegen::ValueStackWithPhi::clear() {
        valueStackStateOut.clear();
        valueStackStateIn.clear();
        phiNodes.clear();
        stackState = StackState::Finalized;
        currentState = 0;
    }

    void LLVMCodegen::ValueStackWithPhi::enterNode(yoi::indexT currentState, llvm::BasicBlock *currentBlock) {
        yoi_assert(
            stackState == StackState::Finalized, 0, 0, "llvmCodegen: invoking enterNode on an unfinalized stack");
        stackState = StackState::InEvaluation;
        this->currentState = currentState;

        // check whether the first time to evaluate this block, if so, inherit the stack base from stack top of previous
        // block.
        if (auto it = valueStackStateIn.find(currentState) == valueStackStateIn.end()) {
            valueStackStateIn[currentState] = {};
            phiNodes[currentState] = {};
        } else {
            panic(0, 0, "llvmCodegen: jumped at entry block");
        }
    }

    LLVMCodegen::StackValue &LLVMCodegen::ValueStackWithPhi::operator[](yoi::indexT index) {
        return valueStackStateOut[currentState][index];
    }

    yoi::indexT LLVMCodegen::ValueStackWithPhi::size() const {
        return valueStackStateOut.at(currentState).size();
    }

    bool LLVMCodegen::ValueStackWithPhi::empty() const {
        return valueStackStateOut.at(currentState).empty();
    }

    LLVMCodegen::StackValue LLVMCodegen::actualizeInterfaceObject(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type,
                                               llvm::Value *objectPtr,
                                               yoi::indexT implIndex) {

        auto implDef = yoiModule->interfaceImplementationTable[implIndex];
        auto structYoiType = managedPtr(IRValueType{std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex)});

        auto structGcFunc = getGcFunction(llvmModCtx, structYoiType, false);
        yoi_assert(structGcFunc != nullptr, 0, 0, "llvmCodegen: expected gc function for struct but received nullptr");

        auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject,
                                            implDef->implInterfaceIndex.first,
                                            implDef->implInterfaceIndex.second);

        auto key = std::make_tuple(
            IRValueType::valueType::interfaceObject, yoiModule->identifier, implDef->implInterfaceIndex.second);
        auto *interfaceLLVMType = llvmModCtx.structTypeMap.at(key);

        auto size = llvmModCtx.TheModule->getDataLayout().getTypeAllocSize(interfaceLLVMType);
        auto *sizeVal = llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), size);

        auto *allocCall = llvmModCtx.Builder->CreateCall(llvmModCtx.runtimeFunctions.at(L"object_alloc"), sizeVal, "newinterface_alloc");
        auto *bitcast = llvmModCtx.Builder->CreateBitCast(allocCall, llvm::PointerType::get(interfaceLLVMType, 0), "casttmp");

        auto *refCountPtr = llvmModCtx.Builder->CreateStructGEP(interfaceLLVMType, bitcast, 0, "refcount_ptr");
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), 1), refCountPtr);

        auto *typeIdPtr = llvmModCtx.Builder->CreateStructGEP(interfaceLLVMType, bitcast, 1, "typeid_ptr");
        auto typeIdKey = std::make_tuple(
            IRValueType::valueType::interfaceObject, yoiModule->identifier, implDef->implInterfaceIndex.second, 0);
        llvmModCtx.Builder->CreateStore(llvm::ConstantInt::get(llvmModCtx.Builder->getInt64Ty(), llvmModCtx.typeIDMap[typeIdKey]), typeIdPtr);

        auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::interfaceObject,
                                                     implDef->implInterfaceIndex.first,
                                                     implDef->implInterfaceIndex.second);

        auto interfaceShellVal = StackValue{bitcast, yoiType};
        
        /*auto structInstanceVal = llvmModCtx.valueStackPhi.back();
        llvmModCtx.valueStackPhi.pop_back();*/
        auto structInstanceVal = StackValue{objectPtr, type};

        if (structInstanceVal.yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope)) {
            callGcFunction(llvmModCtx, structInstanceVal.llvmValue, structInstanceVal.yoiType, true, true, true);
        }

        // Store `this` pointer at index 1
        auto *thisPtrField =
            llvmModCtx.Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 2, "this_ptr_field");
        auto [objectType, objectValue] = ensureObject(llvmModCtx, structInstanceVal.yoiType, structInstanceVal.llvmValue);
        auto *castedStructPtr =
            llvmModCtx.Builder->CreateBitCast(objectValue, llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), "casted_this");
        llvmModCtx.Builder->CreateStore(castedStructPtr, thisPtrField);

        // Populate GC function pointers at indices 3 and 4 with pointers to the interfaceImpl wrappers
        auto *decVTableSlot =
            llvmModCtx.Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 4, "gc_dec_slot");
        llvmModCtx.Builder->CreateStore(structGcFunc, decVTableSlot);

        // Populate user method pointers starting at index 5
        for (size_t i = 0; i < implDef->virtualMethods.size(); ++i) {
            auto &methodYoiType = implDef->virtualMethods[i];
            yoi_assert(methodYoiType->type == IRValueType::valueType::virtualMethod,
                       0,
                       0,
                       "Expected virtual method type in impl definition");
            auto funcIndex = methodYoiType->typeIndex;
            auto funcDef = yoiModule->functionTable[funcIndex];
            auto *llvmFunction = llvmModCtx.functionMap.at(funcDef->name);

            auto *vtableSlotPtr =
                llvmModCtx.Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, i + 5, "vtable_slot");
            llvmModCtx.Builder->CreateStore(llvmFunction, vtableSlotPtr);
        }

        return interfaceShellVal;
    }

    LLVMCodegen::StackValue LLVMCodegen::wrapInterfaceObjectIfRegressed(LLVMModuleContext &llvmModCtx, const StackValue &objectVal) {
        if (objectVal.yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
            auto impl = objectVal.yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
            auto implDef = yoiModule->interfaceImplementationTable[impl.second];
            if (impl.first != -1) {
                return actualizeInterfaceObject(llvmModCtx, managedPtr(IRValueType{std::get<0>(implDef->implStructIndex), std::get<1>(implDef->implStructIndex), std::get<2>(implDef->implStructIndex), objectVal.yoiType->attributes}), objectVal.llvmValue, impl.second);
            }
        }
        return objectVal;
    }

    llvm::Value * LLVMCodegen::unwrapInterfaceObject(LLVMModuleContext &llvmModCtx, const StackValue &objectVal) {
        yoi_assert(objectVal.yoiType->type == IRValueType::valueType::interfaceObject, 0, 0, "unwrapInterfaceObject(llvmModCtx, ...): Except interface object");
        if (objectVal.yoiType->metadata.hasMetadata(L"regressed_interface_impl")) {
            auto impl = objectVal.yoiType->metadata.getMetadata<std::pair<yoi::indexT, yoi::indexT>>(L"regressed_interface_impl");
            auto implDef = yoiModule->interfaceImplementationTable[impl.second];

            if (impl.first != -1) {
                return objectVal.llvmValue;
            }
        }
        auto interfaceType = llvmModCtx.structTypeMap.at({objectVal.yoiType->type, objectVal.yoiType->typeAffiliateModule, objectVal.yoiType->typeIndex});
        auto *thisPtrField = llvmModCtx.Builder->CreateStructGEP(interfaceType, objectVal.llvmValue, 2);
        auto *thisPtr = llvmModCtx.Builder->CreateLoad(llvm::PointerType::get(llvmModCtx.Builder->getInt8Ty(), 0), thisPtrField);
        return thisPtr;
    }
    LLVMCodegen::StackValue LLVMCodegen::promiseInterfaceObjectIfInterface(LLVMModuleContext &llvmModCtx, const StackValue &objectVal) {
        return objectVal.yoiType->type == IRValueType::valueType::interfaceObject
                   ? wrapInterfaceObjectIfRegressed(llvmModCtx, objectVal)
                   : objectVal;
    }

    void LLVMCodegen::handleIntrinsicCall(LLVMModuleContext &llvmModCtx, const IR &instr) {
        switch (instr.opcode) {
            case IR::Opcode::invoke_imported: {
                auto libIndex = instr.operands[0].value.symbolIndex;
                auto funcIndex = instr.operands[1].value.symbolIndex;
                auto argCount = instr.operands[2].value.symbolIndex;

                auto funcDef = compilerCtx->getIRFFITable()->importedLibraries[libIndex].importedFunctionTable[funcIndex];
                yoi_assert(funcDef->hasAttribute(IRFunctionDefinition::FunctionAttrs::Intrinsic), instr.debugInfo.line, instr.debugInfo.column, "llvmCodegen: expected intrinsic function");

                if (funcDef->name == L"runtime_get_string_array_data_pointer") {
                    // logic of intrinsic, offset to the value address which is the forth member of an array struct definition
                    auto object = llvmModCtx.valueStackPhi.back();
                    llvmModCtx.valueStackPhi.pop_back();
                    yoi_assert(object.yoiType->isArrayType() || object.yoiType->isDynamicArrayType(), instr.debugInfo.line, instr.debugInfo.column, "llvmCodegen: expected array type");
                    auto pointer = llvmModCtx.Builder->CreateStructGEP(getArrayLLVMType(llvmModCtx, object.yoiType), object.llvmValue, 3, "array_ptr");
                    auto toInt = llvmModCtx.Builder->CreatePtrToInt(pointer, llvmModCtx.Builder->getInt64Ty(), "array_ptr_ptrtoint");
                    llvmModCtx.valueStackPhi.push_back(StackValue{toInt, managedPtr(compilerCtx->getUnsignedObjectType()->getBasicRawType())});
                } else {
                    panic(instr.debugInfo.line, instr.debugInfo.column, "llvmCodegen: unsupported intrinsic function");
                }
                break;
            }
            default:
                panic(instr.debugInfo.line, instr.debugInfo.column, "llvmCodegen: unsupported intrinsic call");
                break;
        }
    }

    void LLVMCodegen::generateWrapperForForeignCallablesIfNotExists(LLVMModuleContext &llvmModCtx, const std::shared_ptr<IRValueType> &type) {
        // further implementation details are under discussion, leave this function empty temporarily

        
        // std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> key = {type->type, type->typeAffiliateModule, type->typeIndex};

        // if (foreignTypeMap.find(key) != foreignTypeMap.end()) {
        //     // we have already created the wrapper, just return
        //     return;
        // }
        
        // yoi_assert(type->type == IRValueType::valueType::interfaceObject, llvmModCtx.currentFunctionDef->debugInfo.line, llvmModCtx.currentFunctionDef->debugInfo.column, "llvmCodegen: expected callable interface type for generateWrapperForForeignCallablesIfNotExists");

        // auto interfaceDef = yoiModule->interfaceTable[type->typeIndex];
        // yoi_assert(interfaceDef->functionOverloadIndexies.contains(L"operator()") && interfaceDef->functionOverloadIndexies.at(L"operator()").size() == 1, llvmModCtx.currentFunctionDef->debugInfo.line, llvmModCtx.currentFunctionDef->debugInfo.column, "llvmCodegen: expected operator() in callable interface");

        // auto funcIndex = interfaceDef->functionOverloadIndexies.at(L"operator()")[0];
        // auto funcDef = yoiModule->functionTable[funcIndex];
        // // determine the LLVM type of the foreign type, first
        // yoi::vec<llvm::Type *> argTypes; 

        // for (auto &param : funcDef->argumentTypes) {
        //     argTypes.push_back(yoiTypeToLLVMType(llvmModCtx, param, true));
        // }
        
        // // 
    }

    LLVMCodegen::LLVMModuleContext &LLVMCodegen::getLLVMModuleContext(const yoi::wstr &absolutePath) {
        auto it = llvmModuleContext.find(absolutePath);
        if (it == llvmModuleContext.end()) {
            throw std::runtime_error("Module not found");
        }
        return *it->second;
    }

    LLVMCodegen::LLVMModuleContext::LLVMModuleContext(const std::shared_ptr<IRModule> &yoiModule,
                                                      yoi::indexT hash,
                                                      const yoi::wstr &absolute_path)
        : TheContext(std::make_unique<llvm::LLVMContext>()),
          Builder(std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*TheContext))),
          nextTypeId(0), 
          controlFlowAnalysis({}),
          valueStackPhi(controlFlowAnalysis, Builder.get(), yoiModule),
          absolute_path(absolute_path) {
        TheModule = std::make_unique<llvm::Module>("yoi.module." + std::to_string(hash), *TheContext);
        TheModule->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
        TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 4);
        DBuilder = std::make_unique<llvm::DIBuilder>(*TheModule);
    }

    yoi::vec<yoi::wstr> LLVMCodegen::generate() {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmParser();
        llvm::InitializeNativeTargetDisassembler();
        llvm::InitializeNativeTargetAsmPrinter();

        // first, we create each modules for different source files
        for (auto &module : compilerCtx->getCompiledModules()) {
            llvmModuleContext[module.second->modulePath] = std::make_unique<LLVMModuleContext>(module.second, module.first, module.second->modulePath);
            generateDeclarations(*llvmModuleContext[module.second->modulePath]);
        }
        
        for (auto &module : compilerCtx->getCompiledModules()) {
            if (module.second->modulePath == L"builtin") {
                continue;
            }
            codegenTaskDispatcher.dispatch([this, module]() {
                yoi::indexT last_write_time = std::filesystem::last_write_time(module.second->modulePath).time_since_epoch().count();
                if (last_write_time == codegenObjectCache.get_entry(module.second->modulePath).getLastModification()) {
                    warning(0, 0, "llvmCodegen: skipping module " + wstring2string(module.second->modulePath), "MODULE_NOT_MODIFIED");
                    return;
                }

                auto cache_entry = codegenObjectCache.get_entry(module.second->modulePath);
                
                generateImplementations(*llvmModuleContext[module.second->modulePath]);
                llvmModuleContext[module.second->modulePath]->DBuilder->finalize();
                generateTargetObjectCode(*llvmModuleContext[module.second->modulePath], cache_entry.getObjectFilename());

                // update last modification time thread-safely
                last_write_time = std::filesystem::last_write_time(module.second->modulePath).time_since_epoch().count();
                codegenObjectCache.update_last_modification(module.second->modulePath, last_write_time);
            });
        }

        codegenTaskDispatcher.wait();

        generateImplementations(*llvmModuleContext[L"builtin"]);
        generateRuntimeFunctionImplementations(*llvmModuleContext[L"builtin"]);
        generateBasicTypeImplementations(*llvmModuleContext[L"builtin"]);
        generateImportFunctionImplementations(*llvmModuleContext[L"builtin"]);
        generateDescription(*llvmModuleContext[L"builtin"]);
        generateExportFunctionDecls(*llvmModuleContext[L"builtin"]);
        generateMainFunction(*llvmModuleContext[L"builtin"]);
        generateRTTIImplmentation(*llvmModuleContext[L"builtin"]);

        for (auto &arr : llvmModuleContext[L"builtin"]->arrayToGenerateImplementations) {
            generateArrayGCFunctionImplementations(*llvmModuleContext[L"builtin"], std::get<0>(arr), std::get<1>(arr), std::get<2>(arr));
        }

        llvmModuleContext[L"builtin"]->DBuilder->finalize();
        generateTargetObjectCode(*llvmModuleContext[L"builtin"], codegenObjectCache.get_entry(L"builtin").getObjectFilename());

        // save codegen object cache
        if (!compilerCtx->getBuildConfig()->immediatelyClearupCache) {
            auto cache_path = std::filesystem::path(compilerCtx->getBuildConfig()->buildCachePath);
            auto cache_file = fopen((cache_path / "hoshi.cache.tsuki").string().c_str(), "wb+");
            yoi_assert(cache_file, 0, 0, "llvmCodegen: failed to open cache file");
            serialization::write(cache_file, codegenObjectCache);
            fclose(cache_file);
        }

        yoi::vec<yoi::wstr> objectFileNames;
        for (auto &module : compilerCtx->getCompiledModules()) {
            objectFileNames.push_back(codegenObjectCache.get_entry(module.second->modulePath).getObjectFilename());
        }
        return objectFileNames;
    }

    void LLVMCodegen::dumpIR(const yoi::wstr& modulePath, const std::string& filename) {
        if (llvmModuleContext.count(modulePath)) {
            std::error_code ec;
            llvm::raw_fd_ostream os(filename, ec);
            if (!ec) {
                llvmModuleContext[modulePath]->TheModule->print(os, nullptr);
            }
        }
    }


    void LLVMCodegen::callGcFunction(LLVMModuleContext &llvmModCtx,
                                       llvm::Value *objectPtr,
                                       const std::shared_ptr<IRValueType> &yoiType,
                                       bool isIncrease,
                                       bool forceForPermanent,
                                       bool forceForBorrow) {
        if (yoiType->type == IRValueType::valueType::none || yoiType->hasAttribute(IRValueType::ValueAttr::Raw) || yoiType->isBasicRawType()) {
            return;
        }
        if (yoiType->hasAttribute(IRValueType::ValueAttr::PermanentInCurrentScope) && !forceForPermanent) {
            return;
        }
        if (yoiType->hasAttribute(IRValueType::ValueAttr::Borrow) && !forceForBorrow)
            return;
        
        auto gcFunc = getGcFunction(llvmModCtx, yoiType, isIncrease);
        if (gcFunc == nullptr) return;

        auto f = [&]() {
            auto* ptrArg = llvmModCtx.Builder->CreateBitCast(objectPtr, gcFunc->getFunctionType()->getParamType(0));
            llvmModCtx.Builder->CreateCall(gcFunc, ptrArg);
        };
        generateIfTargetNotNull(llvmModCtx, objectPtr, yoiType, f);
    }
} // namespace yoi