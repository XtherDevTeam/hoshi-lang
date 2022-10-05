//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_NOPIR_HPP
#define XSCRIPT2_NOPIR_HPP

#include <IR/IR.hpp>

namespace Hoshi {
    class NopIR : public IR {
    public:
        /**
         * @brief Construct a Nop IR
         */
        NopIR();
    };
}

#endif