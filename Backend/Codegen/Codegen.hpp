//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_CODEGEN_HPP
#define XSCRIPT2_CODEGEN_HPP

#include <IR/IRBlock.hpp>
#include <IR/IRProgram.hpp>
#include <Exceptions/CompilerError.hpp>

namespace Hoshi {
    template<typename Result, typename NodeType>
    class Codegen {
    public:
        /**
         * @brief visit an ast and gen the code
         * @return result
         */
        virtual Result Visit(NodeType &Node, IRProgram::Builder &Program) = 0;
    };
}

#endif