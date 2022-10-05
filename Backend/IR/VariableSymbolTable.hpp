//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_VARIABLESYMBOLTABLE_HPP
#define XSCRIPT2_VARIABLESYMBOLTABLE_HPP

#include <Config.hpp>
#include <IR/Operand.hpp>
#include <Type/TypeInfo.hpp>

namespace Hoshi {
    struct VariableSymbolInfo {
        XString Name;
        Operand Value;
        Type *type;
    };

    class VariableSymbolTable {
        XMap<XString, VariableSymbolInfo> SymbolTable;
    public:
        void AddSymbol(XString SymbolName, VariableSymbolInfo SymbolInfo);

        bool Exists(XString SymbolName);

        VariableSymbolInfo &GetSymbol(XString SymbolName);

        void RemoveSymbol(XString SymbolName);

        XArray<VariableSymbolInfo> GetAllSymbols();
    };
}

#endif