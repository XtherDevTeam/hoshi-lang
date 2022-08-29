//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_PROGRAMCONTEXT_HPP
#define XSCRIPT2_PROGRAMCONTEXT_HPP

#include <IR/VariableSymbolTable.hpp>

namespace Hoshi {
    class ProgramContext {
    public:
        VariableSymbolTable VariableTable;
    };
}

#endif