#include <iostream>
#include <compiler/frontend/lexer.hpp>
#include <share/def.hpp>
#include <sstream>
#include <compiler/compilerContext.h>
#include <compiler/frontend/ast.hpp>
#include <compiler/ir/IR.h>
#include <stdexcept>

int main(int argc, const char **argv) {
    try {
        std::shared_ptr<yoi::compilerContext> compilerCtx = std::make_shared<yoi::compilerContext>();
        compilerCtx->initializeSharedObjects();
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
            return 1;
        }
        std::string in = argv[1];
        yoi::wstr input = yoi::string2wstring(in);
        auto idx = compilerCtx->compileModule(input);
        auto str = compilerCtx->getImportedModule(idx)->to_string();

        std::cout << yoi::wstring2string(str) << std::endl;
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
