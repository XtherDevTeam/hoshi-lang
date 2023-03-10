//
// Created by XIaokang00010 on 2023/3/4.
//

#include "visitor.hpp"

namespace hoshi {
    void visitor::visitModule(const wstr &moduleName, hoshiModule *target) {
        int64_t modId = cxt->newModule(moduleName);
        int64_t initFuncId = cxt->getModule(modId).funcTable.putFunc(L"hoshiModuleInit", {});
        cxt->getModule(modId).funcTable.getFunc(initFuncId).setFuncType({});
        cxt->getModule(modId).funcTable.getFunc(initFuncId).getFuncType().setResultType({});
        cxt->getModule(modId).funcTable.getFunc(initFuncId).getFuncType().getResultType().set(irValueType::type::vInt,
                                                                                              {});
        int64_t initFuncEntryId = cxt->getModule(modId).funcTable.getFunc(initFuncId).newBasicBlock(L"entry");
        int64_t initFuncMergeBId = cxt->getModule(modId).funcTable.getFunc(initFuncId).newBasicBlock(L"mergeB");
    }

    void visitor::setContext(irContext *c) {
        cxt = c;
    }

    void visitor::visitInFunc(basicLiterals *literals) {
        switch (literals->get().kind) {
            case lexer::token::tokenKind::integer:
                builder->putInst({irInstruction::type::tempVar,
                                  {{irInstructionArg::type::vUnknown, builder->getFunc().tempVarTable.put(
                                          {irValueType::type::vInt})}, {irInstructionArg::type::vConst,
                                                                        builder->getFunc().constPool.put(
                                                                                {irConstVal::type::vInt,
                                                                                 literals->get().basicVal.vInt})}}});
                break;
            case lexer::token::tokenKind::decimal:
                builder->putInst({irInstruction::type::tempVar,
                                  {{irInstructionArg::type::vUnknown, builder->getFunc().tempVarTable.put(
                                          {irValueType::type::vInt})}, {irInstructionArg::type::vConst,
                                                                        builder->getFunc().constPool.put(
                                                                                {irConstVal::type::vDeci,
                                                                                 builder->getModule().constPool.putDeci(
                                                                                         literals->get().basicVal.vDeci)})}}});
                break;
            case lexer::token::tokenKind::boolean:
                builder->putInst({irInstruction::type::tempVar,
                                  {{irInstructionArg::type::vUnknown, builder->getFunc().tempVarTable.put(
                                          {irValueType::type::vInt})}, {irInstructionArg::type::vConst,
                                                                        builder->getFunc().constPool.put(
                                                                                {irConstVal::type::vBool,
                                                                                 literals->get().basicVal.vBool})}}});
                break;
            default:
                break;
        }
    }
} // hoshi