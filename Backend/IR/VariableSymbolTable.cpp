//
// Created by theflysong on 2022/8/29.
//

#include <IR/VariableSymbolTable.hpp>

namespace Hoshi {
    void VariableSymbolTable::AddSymbol(XString SymbolName, VariableSymbolInfo SymbolInfo) {
        SymbolTable.insert(std::make_pair(SymbolName, SymbolInfo));
    }

    bool VariableSymbolTable::Exists(XString SymbolName) {
        return SymbolTable.find(SymbolName) != SymbolTable.end();
    }

    VariableSymbolInfo &VariableSymbolTable::GetSymbol(XString SymbolName) {
        return SymbolTable.at(SymbolName);
    }

    void VariableSymbolTable::RemoveSymbol(XString SymbolName) {
        SymbolTable.erase(SymbolName);
    }

    XArray<VariableSymbolInfo> VariableSymbolTable::GetAllSymbols() {
        XArray<VariableSymbolInfo> Symbols;
        for (const auto& Pair: SymbolTable) {
            Symbols.push_back(Pair.second);
        }
        return Symbols;
    }
}