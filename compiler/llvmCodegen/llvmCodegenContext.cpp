//
// Created by XIaokang00010 on 2024/10/9.
//

#include "llvmCodegenContext.hpp"
#include "compiler/ir/IR.h"
#include "share/def.hpp"
#include <llvm/Support/raw_ostream.h>
#include <tuple>

namespace yoi {

LLVMCodegen::LLVMCodegen(std::shared_ptr<compilerContext> compilerCtx, std::shared_ptr<IRModule> yoiModule)
    : TheContext(std::make_unique<llvm::LLVMContext>()),
      Builder(std::make_unique<llvm::IRBuilder<>>(*TheContext)),
      compilerCtx(std::move(compilerCtx)),
      yoiModule(std::move(yoiModule)) {
    TheModule = std::make_unique<llvm::Module>("yoi.module", *TheContext);
}

void LLVMCodegen::generate() {
    generateDeclarations();
    generateImplementations();
}

llvm::Module* LLVMCodegen::getModule() {
    return TheModule.get();
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
        auto structName = "struct." + wstring2string(structDef->name);
        structTypeMap[key] = llvm::StructType::create(*TheContext, structName);
    }
    for (auto& interfaceDefPair : yoiModule->interfaceTable) {
        auto interfaceDef = interfaceDefPair.second;
        auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, yoiModule->interfaceTable.getIndex(interfaceDef->name));
        auto interfaceName = "interface." + wstring2string(interfaceDef->name);
        structTypeMap[key] = llvm::StructType::create(*TheContext, interfaceName);
    }
}

void LLVMCodegen::generateGlobalDeclarations() {
    for (auto& globalPair : yoiModule->globalVariables) {
        auto globalName = wstring2string(globalPair.first);
        auto globalType = yoiTypeToLLVMType(globalPair.second);
        auto initializer = getGlobalInitializer(globalPair.second);
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
    generateFunctionImplementations();
}

void LLVMCodegen::generateStructImplementations() {
    for (auto& structDefPair : yoiModule->structTable) {
        auto structDef = structDefPair.second;
        auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, yoiModule->structTable.getIndex(structDef->name));
        auto* llvmStructType = structTypeMap.at(key);

        std::vector<llvm::Type*> fieldTypes;
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
        memberTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0));
        memberTypes.push_back(llvm::PointerType::get(Builder->getVoidTy(), 0));
        memberTypes.push_back(llvm::PointerType::get(Builder->getVoidTy(), 0));

        for (const auto& methodPair : interfaceDef->methodMap) {
            auto funcType = getFunctionType(methodPair.second);
            std::vector<llvm::Type*> virtualArgTypes;
            virtualArgTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0));
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

    Builder->SetInsertPoint(blockMap[0]);
    
    namedValues.clear();
    auto& varTableRef = funcDef.getVariableTable();
    
    auto arg_it = currentFunction->arg_begin();
    for (yoi::indexT i = 0; i < funcDef.argumentTypes.size(); ++i, ++arg_it) {
        auto& argType = funcDef.argumentTypes[i];
        const auto& argName = varTableRef.getReversedVariableNameMap().at(i);
        auto* alloca = Builder->CreateAlloca(yoiTypeToLLVMType(argType), nullptr, wstring2string(argName));
        Builder->CreateStore(arg_it, alloca);
        namedValues[i] = alloca;
    }

    const auto& vars = varTableRef.getVariables();
    const auto& names = varTableRef.getReversedVariableNameMap();
    for(yoi::indexT i = 0; i < vars.size(); ++i) {
        if (namedValues.count(i)) continue;
        auto* alloca = Builder->CreateAlloca(yoiTypeToLLVMType(vars[i]), nullptr, wstring2string(names.at(i)));
        namedValues[i] = alloca;
    }

    for (yoi::indexT i = 0; i < funcDef.codeBlock.size(); ++i) {
        generateCodeBlock(*funcDef.codeBlock[i], i);
    }

    if (llvm::verifyFunction(*currentFunction, &llvm::errs())) {
        TheModule->print(llvm::errs(), nullptr);
        panic(0, 0, "LLVM function verification failed for: " + wstring2string(funcDef.name));
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
    switch(instr.opcode) {
        case IR::Opcode::push_integer: {
            valueStack.push_back({
                llvm::ConstantInt::get(Builder->getInt64Ty(), instr.operands[0].value.integer, true),
                compilerCtx->getIntObjectType()
            });
            break;
        }
        case IR::Opcode::push_decimal: {
            valueStack.push_back({
                llvm::ConstantFP::get(Builder->getDoubleTy(), instr.operands[0].value.decimal),
                compilerCtx->getDeciObjectType()
            });
            break;
        }
        case IR::Opcode::push_boolean: {
            valueStack.push_back({
                llvm::ConstantInt::get(Builder->getInt1Ty(), instr.operands[0].value.boolean),
                compilerCtx->getBoolObjectType()
            });
            break;
        }
        case IR::Opcode::push_string: {
            auto& str = yoiModule->stringLiteralPool.getStringLiteral(instr.operands[0].value.stringLiteralIndex);
            valueStack.push_back({
                Builder->CreateGlobalString(wstring2string(str)),
                compilerCtx->getStrObjectType()
            });
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
            auto val = valueStack.back();
            valueStack.pop_back();
            valueStack.push_back({Builder->CreateNeg(val.llvmValue, "negtmp"), val.yoiType});
            break;
        }
        case IR::Opcode::bitwise_not: {
            auto val = valueStack.back();
            valueStack.pop_back();
            valueStack.push_back({Builder->CreateNot(val.llvmValue, "nottmp"), val.yoiType});
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
            auto loadedVal = Builder->CreateLoad(alloca->getAllocatedType(), alloca, "loadtmp");
            valueStack.push_back({loadedVal, yoiType});
            break;
        }
        case IR::Opcode::store_local: {
            auto val = valueStack.back();
            valueStack.pop_back();
            auto* alloca = namedValues.at(instr.operands[0].value.symbolIndex);
            Builder->CreateStore(val.llvmValue, alloca);
            break;
        }
        case IR::Opcode::load_global: {
            auto varIndex = instr.operands[0].value.symbolIndex;
            auto* global = globalValues.at(varIndex);
            auto yoiType = yoiModule->globalVariables[varIndex];
            auto loadedVal = Builder->CreateLoad(global->getValueType(), global, "loadglobaltmp");
            valueStack.push_back({loadedVal, yoiType});
            break;
        }
        case IR::Opcode::store_global: {
            auto val = valueStack.back();
            valueStack.pop_back();
            auto* global = globalValues.at(instr.operands[0].value.symbolIndex);
            Builder->CreateStore(val.llvmValue, global);
            break;
        }
        case IR::Opcode::load_member: {
            auto structVal = valueStack.back();
            valueStack.pop_back();
            auto memberIndex = instr.operands[0].value.symbolIndex;
            auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
            auto* llvmStructType = structTypeMap.at(key);
            auto* gep = Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, memberIndex, "memberptr");
            auto yoiStructDef = compilerCtx->getImportedModule(std::get<1>(key))->structTable[std::get<2>(key)];
            auto memberYoiType = yoiStructDef->fieldTypes[memberIndex];
            llvm::Type* loadedType = yoiTypeToLLVMType(memberYoiType);
            valueStack.push_back({Builder->CreateLoad(loadedType, gep, "loadmember"), memberYoiType});
            break;
        }
        case IR::Opcode::store_member: {
            auto structVal = valueStack.back();
            valueStack.pop_back();
            auto valueToStore = valueStack.back();
            valueStack.pop_back();
            auto memberIndex = instr.operands[0].value.symbolIndex;
            auto key = std::make_tuple(IRValueType::valueType::structObject, structVal.yoiType->typeAffiliateModule, structVal.yoiType->typeIndex);
            auto* llvmStructType = structTypeMap.at(key);
            auto* gep = Builder->CreateStructGEP(llvmStructType, structVal.llvmValue, memberIndex, "memberptr");
            Builder->CreateStore(valueToStore.llvmValue, gep);
            break;
        }

        // Control Flow
        case IR::Opcode::jump: {
            Builder->CreateBr(blockMap.at(instr.operands[0].value.codeBlockIndex));
            break;
        }
        case IR::Opcode::jump_if_true:
        case IR::Opcode::jump_if_false: {
             auto cond = valueStack.back();
             valueStack.pop_back();
             auto* destBlock = blockMap.at(instr.operands[0].value.codeBlockIndex);
             auto* nextBlock = llvm::BasicBlock::Create(*TheContext, "fallthrough", currentFunction);
             
             if (instr.opcode == IR::Opcode::jump_if_true) {
                 Builder->CreateCondBr(cond.llvmValue, destBlock, nextBlock);
             } else { // jump_if_false
                 Builder->CreateCondBr(cond.llvmValue, nextBlock, destBlock);
             }
             Builder->SetInsertPoint(nextBlock);
             break;
        }

        case IR::Opcode::ret: {
            auto retVal = valueStack.back();
            valueStack.pop_back();
            Builder->CreateRet(retVal.llvmValue);
            break;
        }
        case IR::Opcode::ret_none: {
            auto* noneType = llvm::PointerType::get(Builder->getInt8Ty(), 0);
            Builder->CreateRet(llvm::ConstantPointerNull::get(noneType));
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
                args.push_back(valueStack.back().llvmValue);
                valueStack.pop_back();
            }
            std::reverse(args.begin(), args.end());

            if (function->getReturnType()->isVoidTy()) {
                Builder->CreateCall(function, args);
            } else {
                auto* call = Builder->CreateCall(function, args, "calltmp");
                valueStack.push_back({call, funcDef->returnType});
            }
            break;
        }
        case IR::Opcode::new_struct: {
            auto structIndex = instr.operands[0].value.symbolIndex;
            auto key = std::make_tuple(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
            auto* structType = structTypeMap.at(key);

            llvm::Type* i8PtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
            llvm::Type* sizeTy = Builder->getInt64Ty();
            llvm::Constant* size = llvm::ConstantExpr::getSizeOf(structType);
            llvm::Function* mallocFunc = TheModule->getFunction("malloc");
            if (!mallocFunc) {
                llvm::FunctionType* mallocType = llvm::FunctionType::get(i8PtrTy, {sizeTy}, false);
                mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", TheModule.get());
            }

            auto* mallocCall = Builder->CreateCall(mallocFunc, size, "newtmp");
            auto* bitcast = Builder->CreateBitCast(mallocCall, llvm::PointerType::get(structType, 0), "casttmp");
            
            auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::structObject, yoiModule->identifier, structIndex);
            valueStack.push_back({bitcast, yoiType});
            break;
        }
        case IR::Opcode::new_interface: {
            // 1. Get the interface index from the yoi::IR instruction.
            auto interfaceIndex = instr.operands[0].value.symbolIndex;
            
            // 2. Look up the corresponding LLVM StructType from our map.
            auto key = std::make_tuple(IRValueType::valueType::interfaceObject, yoiModule->identifier, interfaceIndex);
            auto* interfaceLLVMType = structTypeMap.at(key);

            // 3. Generate a call to malloc to allocate memory on the heap.
            //    (This logic is identical to new_struct).
            llvm::Type* i8PtrTy = llvm::PointerType::get(Builder->getInt8Ty(), 0);
            llvm::Type* sizeTy = Builder->getInt64Ty();
            llvm::Constant* size = llvm::ConstantExpr::getSizeOf(interfaceLLVMType);
            llvm::Function* mallocFunc = TheModule->getFunction("malloc");
            if (!mallocFunc) {
                llvm::FunctionType* mallocType = llvm::FunctionType::get(i8PtrTy, {sizeTy}, false);
                mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", TheModule.get());
            }
            auto* mallocCall = Builder->CreateCall(mallocFunc, size, "newinterface");

            // 4. Cast the returned i8* to a pointer of the correct interface struct type.
            auto* bitcast = Builder->CreateBitCast(mallocCall, llvm::PointerType::get(interfaceLLVMType, 0), "casttmp");

            // 5. Push the allocated pointer and its yoi type onto the value stack.
            //    The subsequent `construct_interface_impl` will use this value.
            auto yoiType = std::make_shared<IRValueType>(IRValueType::valueType::interfaceObject, yoiModule->identifier, interfaceIndex);
            valueStack.push_back({bitcast, yoiType});
            break;
        }
        case IR::Opcode::construct_interface_impl: {
            // This instruction populates an allocated interface shell.
            // The stack at this point should contain (from top):
            // 1. The concrete struct instance that implements the interface.
            // 2. The newly allocated (but empty) interface shell.
            
            // 1. Pop the struct instance and the interface shell from the stack.
            auto structInstanceVal = valueStack.back();
            valueStack.pop_back();
            auto interfaceShellVal = valueStack.back();
            // This instruction modifies the interface shell in place, but doesn't push a new value.
            // The pointer to the interface shell remains on the stack from the new_interface call.
            valueStack.pop_back();
            valueStack.push_back(interfaceShellVal);


            // 2. Get the definition of this specific interface implementation.
            auto interfaceImplIndex = instr.operands[0].value.symbolIndex;
            auto implDef = yoiModule->interfaceImplementationTable[interfaceImplIndex];

            // 3. Get the LLVM type for the interface struct.
            auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject, interfaceShellVal.yoiType->typeAffiliateModule, interfaceShellVal.yoiType->typeIndex);
            auto* interfaceLLVMType = structTypeMap.at(interfaceKey);

            // 4. Store the `this` pointer.
            // Cast the concrete struct pointer to i8* and store it in the first field (index 0) of the interface shell.
            auto* thisPtrField = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 0, "this_ptr_field");
            auto* castedStructPtr = Builder->CreateBitCast(structInstanceVal.llvmValue, llvm::PointerType::get(Builder->getInt8Ty(), 0), "casted_this");
            Builder->CreateStore(castedStructPtr, thisPtrField);

            // 5. Populate the virtual method table (v-table).
            // The layout is: { this_ptr, gc_increase, gc_decrease, method0, method1, ... }
            // So, user methods start at index 3.
            // TODO: Populate GC function pointers at indices 1 and 2.
            for (size_t i = 0; i < implDef->virtualMethods.size(); ++i) {
                auto& methodYoiType = implDef->virtualMethods[i];
                
                // Find the concrete LLVM function for this method.
                yoi_assert(methodYoiType->type == IRValueType::valueType::virtualMethod, 0, 0, "Expected virtual method type in impl definition");
                auto funcIndex = methodYoiType->typeIndex;
                auto funcDef = yoiModule->functionTable[funcIndex];
                auto* llvmFunction = functionMap.at(funcDef->name);

                // Get a pointer to the v-table slot and store the function pointer.
                auto* vtableSlotPtr = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, i + 3, "vtable_slot");
                Builder->CreateStore(llvmFunction, vtableSlotPtr);
            }
            
            break;
        }
        case IR::Opcode::invoke_virtual: {
            // This instruction performs a virtual call through an interface object.
            // The stack at this point should contain (from top):
            // 1. All user-provided arguments for the method.
            // 2. The interface object itself.

            // 1. Get instruction operands.
            auto methodVTableIndex = instr.operands[0].value.symbolIndex;
            auto userArgCount = instr.operands[1].value.symbolIndex;

            // 2. Pop user arguments from the value stack.
            std::vector<llvm::Value*> userArgs;
            for (size_t i = 0; i < userArgCount - 1; ++i) {
                userArgs.push_back(valueStack.back().llvmValue);
                valueStack.pop_back();
            }
            std::reverse(userArgs.begin(), userArgs.end());

            // 3. Pop the interface object itself.
            auto interfaceShellVal = valueStack.back();
            valueStack.pop_back();

            // 4. Look up the LLVM type for the interface struct.
            auto interfaceKey = std::make_tuple(IRValueType::valueType::interfaceObject, interfaceShellVal.yoiType->typeAffiliateModule, interfaceShellVal.yoiType->typeIndex);
            auto* interfaceLLVMType = structTypeMap.at(interfaceKey);

            // 5. Load the concrete `this` pointer from the first field (index 0).
            auto* thisPtrField = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, 0, "this_ptr_field");
            auto* concreteThisPtrRaw = Builder->CreateLoad(llvm::PointerType::get(Builder->getInt8Ty(), 0), thisPtrField, "concrete_this_raw");

            // 6. Load the function pointer to call from the v-table.
            // Layout: { this_ptr, gc_increase, gc_decrease, method0, method1, ... }
            // User methods start at index 3.
            auto vtableSlotIndex = methodVTableIndex + 3;
            auto* vtableSlotPtr = Builder->CreateStructGEP(interfaceLLVMType, interfaceShellVal.llvmValue, vtableSlotIndex, "vtable_slot_ptr");
            
            // First, determine the type of the function pointer we need to load.
            auto interfaceDef = compilerCtx->getImportedModule(std::get<1>(interfaceKey))->interfaceTable[std::get<2>(interfaceKey)];
            auto methodDef = interfaceDef->methodMap[methodVTableIndex];
            auto* funcType = getFunctionType(methodDef);
            // The vtable stores pointers to functions where the first arg is i8*
            std::vector<llvm::Type*> virtualArgTypes;
            virtualArgTypes.push_back(llvm::PointerType::get(Builder->getInt8Ty(), 0));
            for (size_t i = 1; i < funcType->getNumParams(); ++i) {
                virtualArgTypes.push_back(funcType->getParamType(i));
            }
            auto* virtualFuncType = llvm::FunctionType::get(funcType->getReturnType(), virtualArgTypes, false);
            auto* virtualFuncPtrType = llvm::PointerType::get(virtualFuncType, 0);

            // Now, load the function pointer.
            auto* funcPtrToCall = Builder->CreateLoad(virtualFuncPtrType, vtableSlotPtr, "func_ptr");

            // 7. Assemble the final argument list for the call.
            std::vector<llvm::Value*> finalArgs;
            finalArgs.push_back(concreteThisPtrRaw); // The first argument is the raw `this` pointer.
            finalArgs.insert(finalArgs.end(), userArgs.begin(), userArgs.end());

            // 8. Perform the indirect call.
            llvm::CallInst* call = Builder->CreateCall(virtualFuncType, funcPtrToCall, finalArgs, "virtcall");

            // 9. Push the result back onto the stack if the function is not void.
            if (!virtualFuncType->getReturnType()->isVoidTy()) {
                valueStack.push_back({call, methodDef->returnType});
            }

            break;
        }
        case IR::Opcode::nop:
            break; // Do nothing
            
        default:
            panic(0, 0, "LLVM Codegen: Unhandled yoi::IR opcode: " + std::string(magic_enum::enum_name(instr.opcode)));
    }
}

llvm::Type* LLVMCodegen::yoiTypeToLLVMType(const std::shared_ptr<IRValueType>& type) {
    switch (type->type) {
        case IRValueType::valueType::integerObject:
        case IRValueType::valueType::integerRaw:
            return Builder->getInt64Ty();
        case IRValueType::valueType::decimalObject:
        case IRValueType::valueType::decimalRaw:
            return Builder->getDoubleTy();
        case IRValueType::valueType::booleanObject:
        case IRValueType::valueType::booleanRaw:
            return Builder->getInt1Ty();
        case IRValueType::valueType::stringObject:
            return llvm::PointerType::get(Builder->getInt8Ty(), 0);
        case IRValueType::valueType::charRaw:
        case IRValueType::valueType::characterObject:
            return Builder->getInt8Ty();
        case IRValueType::valueType::none:
            return llvm::PointerType::get(Builder->getInt8Ty(), 0);
        case IRValueType::valueType::structObject:
        case IRValueType::valueType::interfaceObject: {
            auto key = std::make_tuple(type->type, type->typeAffiliateModule == (yoi::indexT)-1 ? yoiModule->identifier : type->typeAffiliateModule, type->typeIndex);
            if (structTypeMap.count(key)) {
                return llvm::PointerType::get(structTypeMap.at(key), 0);
            }
            panic(0, 0, "Struct/Interface type not found in map.");
            return nullptr;
        }
        case IRValueType::valueType::pointerObject:
            return llvm::PointerType::get(Builder->getInt8Ty(), 0);
        default:
             panic(0, 0, "LLVM Codegen: Unhandled yoi::IRValueType: " + std::to_string(static_cast<int>(type->type)));
             return nullptr;
    }
}

llvm::FunctionType* LLVMCodegen::getFunctionType(const std::shared_ptr<IRFunctionDefinition>& funcDef) {
    auto* returnType = yoiTypeToLLVMType(funcDef->returnType);
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
    auto R = valueStack.back();
    valueStack.pop_back();
    auto L = valueStack.back();
    valueStack.pop_back();

    llvm::Value* lVal = L.llvmValue;
    llvm::Value* rVal = R.llvmValue;

    bool typesAreFloats = lVal->getType()->isDoubleTy() || rVal->getType()->isDoubleTy();
    std::shared_ptr<IRValueType> resultYoiType = typesAreFloats ? compilerCtx->getDeciObjectType() : compilerCtx->getIntObjectType();


    if (typesAreFloats) {
        if (lVal->getType()->isIntegerTy()) lVal = Builder->CreateSIToFP(lVal, Builder->getDoubleTy(), "inttofp");
        if (rVal->getType()->isIntegerTy()) rVal = Builder->CreateSIToFP(rVal, Builder->getDoubleTy(), "inttofp");
        auto fop = op;
        switch(op) {
            case llvm::Instruction::Add: fop = llvm::Instruction::FAdd; break;
            case llvm::Instruction::Sub: fop = llvm::Instruction::FSub; break;
            case llvm::Instruction::Mul: fop = llvm::Instruction::FMul; break;
            case llvm::Instruction::SDiv: fop = llvm::Instruction::FDiv; break;
            case llvm::Instruction::SRem: fop = llvm::Instruction::FRem; break;
            default: panic(0,0, "Unsupported float binary op");
        }
        valueStack.push_back({Builder->CreateBinOp(fop, lVal, rVal, "fbinop"), resultYoiType});
    } else {
        valueStack.push_back({Builder->CreateBinOp(op, lVal, rVal, "ibinop"), resultYoiType});
    }
}

void LLVMCodegen::handleComparison(llvm::CmpInst::Predicate pred, bool isFloat) {
    auto R = valueStack.back();
    valueStack.pop_back();
    auto L = valueStack.back();
    valueStack.pop_back();

    llvm::Value* lVal = L.llvmValue;
    llvm::Value* rVal = R.llvmValue;

    bool typesAreFloats = lVal->getType()->isDoubleTy() || rVal->getType()->isDoubleTy();

    llvm::Value* result;
    if (typesAreFloats) {
        if (lVal->getType()->isIntegerTy()) lVal = Builder->CreateSIToFP(lVal, Builder->getDoubleTy(), "inttofp");
        if (rVal->getType()->isIntegerTy()) rVal = Builder->CreateSIToFP(rVal, Builder->getDoubleTy(), "inttofp");
        
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
        result = Builder->CreateFCmp(fpred, lVal, rVal, "fcmp");
    } else {
        result = Builder->CreateICmp(pred, lVal, rVal, "icmp");
    }
    valueStack.push_back({result, compilerCtx->getBoolObjectType()});
}


} // yoi