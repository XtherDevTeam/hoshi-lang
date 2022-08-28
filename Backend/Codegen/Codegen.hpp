//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_CODEGEN_HPP
#define XSCRIPT2_CODEGEN_HPP

#include <AST.hpp>
#include <IR/IRBlock.hpp>

namespace Hoshi {
    template<typename Result> class Codegen {
    public:
        /**
         * @brief visit an ast and gen the code
         * @return result
         */
        virtual Result Visit(AST &ast, IRBlock::Builder &block) = 0;
    };
}

#endif