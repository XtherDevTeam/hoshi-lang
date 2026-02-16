#include "share/defines.h"
#include <compiler/frontend/ast.hpp>
#include <compiler/frontend/lexer.hpp>
#include <compiler/frontend/formatter.hpp>
#include <compiler/frontend/parser.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

void printUsage(const char* programName) {
    std::cerr << "hoshi-format tool\n";
    std::cout << "Usage: " << programName << " [options] <input_file>\n"
              << "Options:\n"
              << "  -o <path>                           Set output file path. If not specified, prints to stdout.\n"
              << "  --indent-size <n>                   Set indentation size (default: 4).\n"
              << "  --indent-type <space|tab>           Set indentation type (default: space).\n"
              << "  --brace-style <attached|newline>    Set brace style (default: attached).\n"
              << "  --max-width <n>                     Set maximum line width (default: 80).\n"
              << "  -h, --help                          Display this help message.\n";
}

int main(int argc, const char **argv) {
    std::string inputFile;
    std::string outputPath;
    yoi::FormatOption opt;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 < argc) outputPath = argv[++i];
            else { std::cerr << "Error: -o requires a path.\n"; return 1; }
        } else if (arg == "--indent-size") {
            if (i + 1 < argc) opt.indentSize = std::stoul(argv[++i]);
            else { std::cerr << "Error: --indent-size requires a number.\n"; return 1; }
        } else if (arg == "--indent-type") {
            if (i + 1 < argc) {
                std::string type = argv[++i];
                if (type == "space") opt.indentType = yoi::FormatOption::IndentType::Space;
                else if (type == "tab") opt.indentType = yoi::FormatOption::IndentType::Tab;
                else { std::cerr << "Error: Invalid indent-type '" << type << "'.\n"; return 1; }
            } else { std::cerr << "Error: --indent-type requires space|tab.\n"; return 1; }
        } else if (arg == "--brace-style") {
            if (i + 1 < argc) {
                std::string style = argv[++i];
                if (style == "attached") opt.braceType = yoi::FormatOption::BraceType::Attached;
                else if (style == "newline") opt.braceType = yoi::FormatOption::BraceType::NewLine;
                else { std::cerr << "Error: Invalid brace-style '" << style << "'.\n"; return 1; }
            } else { std::cerr << "Error: --brace-style requires attached|newline.\n"; return 1; }
        } else if (arg == "--max-width") {
            if (i + 1 < argc) opt.maxWidth = std::stoul(argv[++i]);
            else { std::cerr << "Error: --max-width requires a number.\n"; return 1; }
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (!arg.starts_with("-")) {
            if (inputFile.empty()) inputFile = arg;
            else { std::cerr << "Error: Multiple input files specified.\n"; return 1; }
        } else {
            std::cerr << "Error: Unknown argument '" << arg << "'.\n";
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

    try {
        std::ifstream ifs(inputFile, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open input file '" << inputFile << "'.\n";
            return 1;
        }
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        yoi::wstr wcontent = yoi::string2wstring(content);
        yoi::lexer l{std::wstringstream(wcontent)};
        l.scan();

        yoi::hoshiModule *mod = nullptr;
        yoi::parse(mod, l);

        if (!mod) {
            std::cerr << "Error: Parsing failed for '" << inputFile << "'.\n";
            return 1;
        }

        if (outputPath.empty()) {
            yoi::Formatter formatter(std::wcout, opt, l.comments);
            formatter.format(mod);
            std::wcout << std::endl;
        } else {
            std::wofstream ofs(outputPath);
            if (!ofs.is_open()) {
                std::cerr << "Error: Could not open output file '" << outputPath << "'.\n";
                return 1;
            }
            yoi::Formatter formatter(ofs, opt, l.comments);
            formatter.format(mod);
            ofs << std::endl;
            ofs.close();
            std::cout << "Formatted output written to '" << outputPath << "'.\n";
        }

    } catch (const std::exception &e) {
        std::cerr << "Error during formatting: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}