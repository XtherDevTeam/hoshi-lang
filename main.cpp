#include "share/defines.h"
#include <compiler/compilerContext.h>
#include <compiler/frontend/ast.hpp>
#include <compiler/frontend/lexer.hpp>
#include <compiler/ir/IR.h>
#include <compiler/ir/IRLinker.hpp>
#include <compiler/llvmCodegen/llvmCodegenContext.hpp>
#include <compiler/objectLinker/ccObjectLinker.h>
#include <compiler/objectLinker/clObjectLinker.h>
#include <compiler/objectLinker/objectLinker.h>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <share/def.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem> 


namespace fs = std::filesystem;


std::string getOutputExtension(yoi::IRBuildConfig::BuildType type,
                               std::wstring platform) {
    if (type == yoi::IRBuildConfig::BuildType::executable) {
        if (platform == L"windows") return ".exe";
        return ""; 
    } else if (type == yoi::IRBuildConfig::BuildType::library) {
        if (platform == L"windows") return ".dll";
        if (platform == L"darwin") return ".dylib";
        return ".so"; 
    }
    return ""; 
}


void printUsage(const char* programName) {
    std::cerr << "hoshi-lang compiler\n";
    std::cout << "Made with love by Jerry Chou (This project is licensed under the MIT license.)\n";
    std::cerr << "Usage: " << programName << " [options] <input_file> ...\n"
              << "Options:\n"
              << "  -o <path>, --output <path>          Set output file path (e.g., build/my_app).\n"
              << "                                      If <path> is a directory (ends with / or \\), input filename is used.\n"
              << "                                      If not specified, derived from input_file in the current directory.\n"
              << "  --build-type <type>                 Specify build type (executable, static-lib, shared-lib). Default: executable\n"
              << "  --build-mode <mode>                 Specify build mode (debug, release). Default: debug\n"
              << "  --linker <linker>                   Specify object linker (cc, cl, none). Default: cc (cl on Windows platform)\n"
              << "                                      'none' will generate .o file but skip final linking.\n"
              << "  --clean, --remove-intermediate      Remove intermediate files (.yoi, .ll, .o) after compilation.\n"
              << "                                      Default: do not preserve intermediate files.\n"
              << "  -I <path>, --include <path>         Add an include directory to search for header files and dynamic libraries.\n"
              << "  -D <k> <v>, --define <k> <v>        Add a macro definition.\n"
              << "  -W <key>, --warning <key>           Enable warning for a specific category.\n"
              << "  -S <key>, --suppress <key>          Suppress warning for a specific category.\n"
              << "  -E <key>, --error <key>             Treat error for a specific category as a warning.\n"
              << "  -C <path>, --project-cache <path>   Set project cache directory.\n"
              << "  --preserve-intermediate             Explicitly preserve intermediate files.\n"
              << "  --whereami, -w                      Print the path to the hoshi-lang installation directory.\n"
              << "  --build-number                      Print the build number of hoshi-lang.\n"
              << "  -h, --help                          Display this help message.\n";
}

int main(int argc, const char **argv) {
    
    
    std::string inputFile;
    std::string outputPathStr; 
    yoi::IRBuildConfig::BuildType buildType = yoi::IRBuildConfig::BuildType::executable;
    yoi::IRBuildConfig::BuildMode buildMode = yoi::IRBuildConfig::BuildMode::debug;
    yoi::wstr projectCacheDir;
    std::wstring targetPlatform = yoi::string2wstring(YOI_PLATFORM); 
    std::wstring targetArch = yoi::string2wstring(YOI_ARCH);         
    yoi::IRBuildConfig::UseObjectLinker useObjectLinker = strcmp(YOI_PLATFORM, "win32") == 0 ? yoi::IRBuildConfig::UseObjectLinker::cl : yoi::IRBuildConfig::UseObjectLinker::cc;
    yoi::vec<yoi::wstr> includeDirs{L"", (std::filesystem::path(yoi::whereIsHoshiLang()) / ".." / "lib").wstring()};
    yoi::vec<yoi::wstr> additionalLinkingFiles = yoi::ObjectLinker::defaultAdditionalLinkingFiles();
    yoi::vec<std::pair<yoi::wstr, yoi::wstr>> macroDefs;
    bool preserveIntermediateFiles = false; 

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputPathStr = argv[++i];
            } else {
                std::cerr << "Error: " << arg << " requires a path argument.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "--build-type") {
            if (i + 1 < argc) {
                std::string typeStr = argv[++i];
                if (typeStr == "executable") buildType = yoi::IRBuildConfig::BuildType::executable;
                else if (typeStr == "library") buildType = yoi::IRBuildConfig::BuildType::library;
                else {
                    std::cerr << "Error: Invalid build type '" << typeStr << "'. Valid types: executable, library.\n";
                    printUsage(argv[0]);
                    return 1;
                }
            } else {
                std::cerr << "Error: " << arg << " requires a type argument.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "--build-mode") {
            if (i + 1 < argc) {
                std::string modeStr = argv[++i];
                if (modeStr == "debug") buildMode = yoi::IRBuildConfig::BuildMode::debug;
                else if (modeStr == "release") buildMode = yoi::IRBuildConfig::BuildMode::release;
                else {
                    std::cerr << "Error: Invalid build mode '" << modeStr << "'. Valid modes: debug, release.\n";
                    printUsage(argv[0]);
                    return 1;
                }
            } else {
                std::cerr << "Error: " << arg << " requires a mode argument.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "--linker") {
            if (i + 1 < argc) {
                std::string linkerStr = argv[++i];
                if (linkerStr == "cc") useObjectLinker = yoi::IRBuildConfig::UseObjectLinker::cc;
                else if (linkerStr == "cl") useObjectLinker = yoi::IRBuildConfig::UseObjectLinker::cl;
                else if (linkerStr == "none") useObjectLinker = yoi::IRBuildConfig::UseObjectLinker::none;
                else {
                    std::cerr << "Error: Invalid linker type '" << linkerStr << "'. Valid types: cc, cl, none.\n";
                    printUsage(argv[0]);
                    return 1;
                }
            } else {
                std::cerr << "Error: " << arg << " requires a linker argument.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "--clean" || arg == "--remove-intermediate") {
            preserveIntermediateFiles = false;
        } else if (arg == "-C" || arg == "--project-cache") {
            if (i + 1 < argc) {
                projectCacheDir = yoi::string2wstring(argv[++i]);
            } else {
                std::cerr << "Error: " << arg << " requires a path argument.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "-I" || arg == "--include") {
            yoi::wstr includeDir = yoi::string2wstring(argv[++i]);
            includeDirs.push_back(includeDir);
        } else if (arg == "--preserve-intermediate") {
            preserveIntermediateFiles = true;
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0; 
        } else if (arg == "--whereami" || arg == "-w") {
            std::cout << yoi::wstring2string(yoi::realpath(yoi::whereIsHoshiLang()));
            return 0; 
        } else if (arg == "--build-number") {
            std::cout << HOSHI_LANG_VERSION << "\n";
            return 0;
        } else if (arg == "-D" || arg == "--define") {
            if (i + 2 < argc) {
                yoi::wstr key = yoi::string2wstring(argv[++i]);
                yoi::wstr value = yoi::string2wstring(argv[++i]);
                macroDefs.emplace_back(key, value);
            } else {
                std::cerr << "Error: " << arg << " requires two arguments.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "-W" || arg == "--warning") {
            if (i + 1 < argc) {
                std::string key = argv[++i];
                yoi::exception_categories[key] = yoi::ExceptionHandleType::Warning;
            } else {
                std::cerr << "Error: " << arg << " requires one arguments.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "-S" || arg == "--suppress") {
            if (i + 1 < argc) {
                std::string key = argv[++i];
                yoi::exception_categories[key] = yoi::ExceptionHandleType::Suppress;
            } else {
                std::cerr << "Error: " << arg << " requires one arguments.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "-E" || arg == "--error") {
            if (i + 1 < argc) {
                std::string key = argv[++i];
                yoi::exception_categories[key] = yoi::ExceptionHandleType::Panic;
            } else {
                std::cerr << "Error: " << arg << " requires one arguments.\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (!arg.starts_with("-")) { 
            if (arg.ends_with(".hoshi") && inputFile.empty())
                inputFile = arg;
            else if (arg.ends_with(".hoshi") && !inputFile.empty()) {
                std::cerr << "Warning: Multiple input files specified: " << inputFile << " and " << arg << "\n";
                printUsage(argv[0]);
                return 1;
            }
            else
                additionalLinkingFiles.push_back(yoi::string2wstring(arg));
        } else {
            std::cerr << "Error: Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    
    if (inputFile.empty()) {
        std::cerr << "Error: No input file provided.\n";
        printUsage(argv[0]);
        return 1;
    }
    if (!fs::exists(inputFile)) {
        std::cerr << "Error: Input file '" << inputFile << "' does not exist.\n";
        return 1;
    }
    if (!fs::is_regular_file(inputFile)) {
        std::cerr << "Error: Input file '" << inputFile << "' is not a regular file.\n";
        return 1;
    }

    
    fs::path inputFilePath(inputFile);
    fs::path outputDir;
    fs::path outputBaseName;
    
    if (outputPathStr.empty()) {
        
        outputDir = fs::current_path();
        outputBaseName = inputFilePath.stem(); 
    } else {
        fs::path specifiedOutputPath(outputPathStr);
        if (specifiedOutputPath.has_filename()) {
            
            outputDir = specifiedOutputPath.parent_path();
            outputBaseName = specifiedOutputPath.stem();
        } else { 
            outputDir = specifiedOutputPath;
            outputBaseName = inputFilePath.stem(); 
        }
    }

    
    try {
        if (!outputDir.empty() && !fs::exists(outputDir)) {
            fs::create_directories(outputDir);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Could not create output directory '" << outputDir.string() << "': " << e.what() << "\n";
        return 1;
    }

    
    fs::path yoiIRFile = outputDir / (outputBaseName.string() + ".yoi");
    fs::path llvmIRFile = outputDir / (outputBaseName.string() + ".ll");
    fs::path objectFile = outputDir / (outputBaseName.string() + ".o");
    fs::path finalOutput = outputDir / (outputBaseName.string() + getOutputExtension(buildType, targetPlatform));

    
    std::vector<fs::path> intermediateFilesToClean;
    if (!preserveIntermediateFiles) {
        intermediateFilesToClean.push_back(yoiIRFile);
        intermediateFilesToClean.push_back(llvmIRFile);
        intermediateFilesToClean.push_back(objectFile);
    }

    int exitCode = 0; 

    
    std::shared_ptr<yoi::compilerContext> compilerCtx =
        std::make_shared<yoi::compilerContext>();
    try {
        compilerCtx->initializeSharedObjects();

        
        compilerCtx->setBuildConfig(yoi::IRBuildConfig::Builder()
                                        .setBuildType(buildType)
                                        .setBuildPlatform(yoi::string2wstring(YOI_PLATFORM))
                                        .setBuildMode(buildMode)
                                        .setBuildArch(yoi::string2wstring(YOI_ARCH))
                                        .setUseObjectLinker(useObjectLinker)
                                        .setPreserveIntermediateFiles(preserveIntermediateFiles) 
                                        .setImmediatelyClearupCache(!preserveIntermediateFiles)
                                        .setSearchPaths(includeDirs)
                                        .setBuildCachePath(projectCacheDir)
                                        .setMarco(L"platform", yoi::string2wstring(YOI_PLATFORM))
                                        .setMarco(L"arch", yoi::string2wstring(YOI_ARCH))
                                        .setMarco(L"hoshi_feature_version", yoi::string2wstring(HOSHI_LANG_VERSION))
                                        .setMarco(L"hoshi_lang_commit", yoi::string2wstring(HOSHI_LANG_GIT_COMMIT_HASH))
                                        .setAdditionalLinkingFiles(additionalLinkingFiles)
                                        .yield());
        
        for (auto &macro : macroDefs) {
            compilerCtx->getBuildConfig()->marcos[macro.first] = macro.second;
        }

        yoi::wstr input = yoi::string2wstring(inputFile);

        auto entryModuleId = compilerCtx->compileModule(input);
        compilerCtx->runOptimizer();

        std::cout << "Linking Yoi IR modules...\n";
        yoi::IRLinker linker;
        auto objectIRFile = linker.link(compilerCtx, entryModuleId);
        compilerCtx->setIRObjectFile(objectIRFile);
        auto unifiedModule = objectIRFile->compiledModule;

        auto yoiIRStr = unifiedModule->to_string();
        std::error_code ec_yoi;
        llvm::raw_fd_stream yoi_file(yoiIRFile.string(), ec_yoi);
        if (ec_yoi) {
            throw std::runtime_error("Could not open YOI IR output file '" + yoiIRFile.string() + "': " + ec_yoi.message());
        }
        yoi_file << yoi::wstring2string(yoiIRStr);
        yoi_file.close();

        yoi::LLVMCodegen llvmCodegen(compilerCtx, unifiedModule);
        std::cout << "Generating target object files...\n";

        yoi::vec<yoi::wstr> objectFileNames;

        TIMER("Generating target object files", objectFileNames = llvmCodegen.generate());
        
        if (preserveIntermediateFiles) {
            // For debug verification, we print the IR of the main input module
            llvmCodegen.dumpIR(L"builtin", llvmIRFile.string());
        }
        
        if (useObjectLinker != yoi::IRBuildConfig::UseObjectLinker::none) {
            yoi::ObjectLinker *objectLinker = nullptr;
            switch (useObjectLinker) {
                case yoi::IRBuildConfig::UseObjectLinker::cc: {
                    objectLinker = new yoi::ccObjectLinker(objectFileNames, compilerCtx->getBuildConfig());
                    break;
                }
                case yoi::IRBuildConfig::UseObjectLinker::cl: {
                    objectLinker = new yoi::clObjectLinker(objectFileNames, compilerCtx->getBuildConfig());
                    break;
                }
                default:
                    break;
            }

            if (objectLinker) {
                std::cout << "Linking final " << yoi::wstring2string(targetPlatform) << " "
                          << (buildType == yoi::IRBuildConfig::BuildType::executable ? "executable" : "library")
                          << "...\n";
                objectLinker->searchAndSetupLinker();
                objectLinker->setElysiaRuntimePath(yoi::whereIsHoshiLang()); 
                objectLinker->link(yoi::string2wstring(finalOutput.string()));
                delete objectLinker;
            }
        } else {
            std::cout << "Object linking skipped (--linker none).\n";
        }
        std::cout << "Compilation successful!\n";

        if (compilerCtx->getBuildConfig()->immediatelyClearupCache) {
            std::error_code ec_remove;
            std::filesystem::remove_all(compilerCtx->getBuildConfig()->buildCachePath, ec_remove);
            if (ec_remove) {
                std::cerr << "Warning: Could not remove cache file '" << yoi::wstring2string(compilerCtx->getBuildConfig()->buildCachePath) << "': " << ec_remove.message() << "\n";
            }
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exitCode = 1; 
    } /* catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        exitCode = 1; 
    } */
    
    if (!preserveIntermediateFiles && exitCode == 0) {
        for (const auto& file : intermediateFilesToClean) {
            std::error_code ec_remove;
            fs::remove(file, ec_remove);
            if (ec_remove) {
                std::cerr << "Warning: Could not remove intermediate file '" << file.string() << "': " << ec_remove.message() << "\n";
            }
        }
    } else if (preserveIntermediateFiles) {
        std::cout << "Intermediate files preserved.\n";
    }

    return exitCode;
}