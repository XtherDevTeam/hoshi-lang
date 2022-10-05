//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_PROGRAMCONTEXT_HPP
#define XSCRIPT2_PROGRAMCONTEXT_HPP

#include <IR/VariableSymbolTable.hpp>
#include <IR/IRFunction.hpp>
#include <IR/IRBlock.hpp>

namespace Hoshi {
    class ProgramContext {
    public:
        VariableSymbolTable VariableTable;
        IRBlock::Builder *CurrentBlock;
        IRFunction::Builder *CurrentFunction;
    };
}

#endif