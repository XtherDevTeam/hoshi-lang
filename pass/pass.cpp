//
// Created by XIaokang00010 on 2023/2/18.
//

#include "pass.hpp"

namespace hoshi {
    template<typename fromT, typename toT>
    passResult<toT> pass(passResult<fromT> from) {
        return {};
    }

    template<>
    passResult<lexer> pass<wstr, lexer>(passResult<wstr> from) {
        std::wstringstream ss(from.get());
        return {std::move(lexer(std::move(ss)))};
    }

    template<>
    passResult<hoshiModule *> pass<lexer, hoshiModule *>(passResult<lexer> from) {
        passResult<hoshiModule *> result{nullptr};
        from.get().scan();
        parse(result.get(), from.get());
        return result;
    }
}