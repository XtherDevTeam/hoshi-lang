#include <compiler/compilerContext.h>
#include <compiler/frontend/ast.hpp>
#include <compiler/frontend/lexer.hpp>
#include <compiler/ir/IR.h>
#include <compiler/ir/IRLinker.hpp>
#include <compiler/llvmCodegen/llvmCodegenContext.hpp>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <share/def.hpp>
#include <sstream>
#include <stdexcept>

int main(int argc, const char **argv) {
    try {
        std::shared_ptr<yoi::compilerContext> compilerCtx =
            std::make_shared<yoi::compilerContext>();
        compilerCtx->initializeSharedObjects();

        compilerCtx->setBuildConfig(yoi::IRBuildConfig::Builder()
                                        .setBuildType(yoi::IRBuildConfig::BuildType::executable)
                                        .setBuildPlatform(yoi::string2wstring(YOI_PLATFORM))
                                        .setBuildArch(yoi::string2wstring(YOI_ARCH))
                                        .yield());

        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
            return 1;
        }
        std::string in = argv[1];
        yoi::wstr input = yoi::string2wstring(in);

        // Compile the entry module and all its dependencies.
        auto entryModuleId = compilerCtx->compileModule(input);

        // Link all compiled modules into a single object file.
        yoi::IRLinker linker;
        auto objectFile = linker.link(compilerCtx, entryModuleId);
        compilerCtx->setIRObjectFile(objectFile);
        auto unifiedModule = objectFile->compiledModule;

        std::cout << "--- yoi-lang IR (Unified) ---\n";
        auto str = unifiedModule->to_string();

        std::error_code ec;
        llvm::raw_fd_stream yoi_file("cmake-build-debug/test.yoi", ec);
        yoi_file << yoi::wstring2string(str);
        yoi_file.close();


        std::cout << "--- End yoi-lang IR ---\n\n";

        std::cout << "--- LLVM IR ---\n";
        // Pass the unified module to the LLVM codegen.
        yoi::LLVMCodegen llvmCodegen(compilerCtx, unifiedModule);
        llvmCodegen.generate();
        // Print the LLVM IR to file
        llvm::raw_fd_stream ll_file("cmake-build-debug/test.ll", ec);
        llvmCodegen.getModule()->print(ll_file, nullptr);
        ll_file.close();
        std::cout << "\n--- End LLVM IR ---\n";
        llvmCodegen.generateTargetObjectCode(L"cmake-build-debug/test.o");
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}