//
// Created by XIaokang00010 on 2023/2/18.
//

#ifndef HOSHI_LANG_PASS_HPP
#define HOSHI_LANG_PASS_HPP

#include "share/def.hpp"
#include "compiler/frontend/lexer.hpp"
#include "compiler/frontend/ast.hpp"
#include "compiler/frontend/parser.hpp"

namespace hoshi {
    template<typename resultT>
    class passResult {
    protected:
        resultT r;
    public:
        passResult(resultT r) : r(std::move(r)) {
        }

        resultT &get() {
            return r;
        }
    };

    template<typename fromT, typename toT>
    passResult<toT> pass(passResult<fromT> from);


    template<>
    passResult<lexer> pass<wstr, lexer>(passResult<wstr> from);


    template<>
    passResult<hoshiModule *> pass<lexer, hoshiModule *>(passResult<lexer> from);


}

#endif //HOSHI_LANG_PASS_HPP
