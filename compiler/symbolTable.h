//
// Created by XIaokang00010 on 2024/9/4.
//

#ifndef HOSHI_LANG_SYMBOLTABLE_H
#define HOSHI_LANG_SYMBOLTABLE_H

#include <map>
#include "share/def.hpp"

namespace yoi {

    class symbol {
    public:

        yoi::indexT identifier;
        enum class symbolType : yoi::indexT {
            Unknown = 0,
            Func,
            Struct,
            Interface,
            Var,
        } type;

        symbol();

        symbol(yoi::indexT id, symbolType type);
    };

    class symbolTable {
    public:
        yoi::indexTable<yoi::wstr, symbol> symbols;

        symbolTable();

        yoi::indexT insert(const yoi::wstr &name, symbol sym);

        symbol &get(const yoi::wstr &name);

        symbol &get(yoi::indexT id);
    };

} // hoshi

#endif //HOSHI_LANG_SYMBOLTABLE_H
