//
// Created by theflysong on 2022/8/27.
//

#include <Codegen/LiteralsCodegen.hpp>
#include <Exceptions/CompilerError.hpp>

namespace Hoshi {
    LiteralsCodegen LiteralsCodegen::INSTANCE;

    /**
     * @brief visit an literal ast and gen the code
     * @return string form of the value of literal
     */
    XString LiteralsCodegen::visit(AST &ast) {
        if ((! ast.IsLeafNode()) || (ast.Type != AST::TreeType::Literals)) {
            throw CompilerError(ast.Node.Line, ast.Node.Column, L"Wrong ast has been given to the literals codegen!");
        }
        return ast.Node.Value;
    }
}