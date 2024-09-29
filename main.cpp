#include <iostream>
#include <compiler/frontend/lexer.hpp>
#include <share/def.hpp>
#include <sstream>
#include <compiler/compilerContext.h>
#include <compiler/frontend/ast.hpp>

int main(int argc, const char **argv) {
    std::shared_ptr<yoi::compilerContext> compilerCtx = std::make_shared<yoi::compilerContext>();
    compilerCtx->initializeSharedObjects();
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    std::string in = argv[1];
    yoi::wstr input = yoi::string2wstring(in);
    compilerCtx->compileModule(input);
    std::cout << "Compile successfully!" << std::endl;
    return 0;
}
