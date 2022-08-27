//
// Created by theflysong on 2022/8/27.
//

#include <Codegen/LiteralsCodegen.hpp>

namespace Hoshi {
    LiteralsCodegen LiteralsCodegen::INSTANCE;

    /**
     * @brief visit an literal ast and gen the code
     * @return string form of the value of literal
     */
    std::string LiteralsCodegen::visit(AST &ast) {
        if ((! ast.IsLeafNode()) || (ast.Type != AST::Literals)) {
            throw "Wrong ast has been given to the literals codegen!";
        }
        return ast.Node.Value;
    }
}