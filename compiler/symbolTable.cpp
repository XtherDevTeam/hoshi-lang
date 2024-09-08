//
// Created by XIaokang00010 on 2024/9/4.
//

#include "symbolTable.h"

namespace yoi {
    symbol::symbol() : type(symbolType::Unknown), identifier(0) {

    }

    symbol::symbol(yoi::indexT id, symbol::symbolType type) : type(type), identifier(id) {

    }

    symbolTable::symbolTable() = default;

    yoi::indexT symbolTable::insert(const wstr &name, symbol sym) {
        return symbols.put(name, sym);
    }

    symbol &symbolTable::get(yoi::indexT id) {
        return symbols[id];
    }

    symbol &symbolTable::get(const wstr &name) {
        return symbols[name];
    }
} // hoshi