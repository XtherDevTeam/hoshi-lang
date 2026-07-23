//
// Project Index implementation — module resolution and cross-file parsing
//

#include "projectIndex.h"
#include "builtinModule.h"
#include <compiler/frontend/lexer.hpp>
#include <compiler/frontend/parser.hpp>
#include <share/def.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace lsp {

ProjectIndex::ProjectIndex() {}

void ProjectIndex::setSearchPaths(const std::vector<std::string> &paths) {
    searchPaths = paths;
}

void ProjectIndex::addSearchPath(const std::string &path) {
    searchPaths.push_back(path);
}

std::string ProjectIndex::resolveModule(const std::string &importPath,
                                         const std::string &relativeToFile) {
    // Special case: "builtin" is the compiler's internal module, embedded in the binary
    if (importPath == "builtin") {
        return "builtin";
    }

    // Build search directory list:
    // 1. Parent directory of relativeToFile
    // 2. .tsuki_modules subdirectory of relativeToFile's parent
    // 3. Global search paths
    std::vector<std::string> searchDirs;

    if (!relativeToFile.empty()) {
        fs::path parentDir = fs::path(relativeToFile).parent_path();
        searchDirs.push_back(parentDir.string());
        searchDirs.push_back((parentDir / ".tsuki_modules").string());
    }

    for (auto &sp : searchPaths) {
        searchDirs.push_back(sp);
    }

    for (auto &dir : searchDirs) {
        std::string resolved = tryResolve(dir, importPath);
        if (!resolved.empty()) return resolved;
    }

    return ""; // Not found
}

std::string ProjectIndex::tryResolve(const std::string &searchDir,
                                      const std::string &importPath) {
    fs::path base(searchDir);
    fs::path target(importPath);

    // Try: searchDir / importPath (as-is)
    fs::path candidate = base / target;
    if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
        std::error_code ec;
        auto absPath = fs::absolute(candidate, ec);
        if (!ec) return absPath.string();
    }

    // Try: searchDir / importPath.hoshi
    fs::path withExt = base / (target.wstring() + L".hoshi");
    if (fs::exists(withExt) && fs::is_regular_file(withExt)) {
        std::error_code ec;
        auto absPath = fs::absolute(withExt, ec);
        if (!ec) return absPath.string();
    }

    // Try: searchDir / importPath / index.hoshi
    fs::path asDir = candidate / "index.hoshi";
    if (fs::exists(asDir) && fs::is_regular_file(asDir)) {
        std::error_code ec;
        auto absPath = fs::absolute(asDir, ec);
        if (!ec) return absPath.string();
    }

    return "";
}

void ProjectIndex::indexModule(const std::string &absolutePath) {
    if (absolutePath.empty()) return;
    if (modules.find(absolutePath) != modules.end()) return; // Already indexed

    std::wstring wContent;

    if (absolutePath == "builtin") {
        // Use the embedded builtin module content (prebuilt into the compiler)
        std::string builtinStr(__lsp_builtin_module);
        wContent = yoi::string2wstring(builtinStr);
    } else {
        // Read the file from disk
        std::ifstream file(absolutePath, std::ios::binary);
        if (!file.is_open()) return;

        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::string content(size, '\0');
        file.read(&content[0], size);
        file.close();

        wContent = yoi::string2wstring(content);
    }

    IndexedModule mod;
    mod.text = wContent;
    parseAndIndex(absolutePath, wContent, mod);
    modules[absolutePath] = std::move(mod);
}

void ProjectIndex::parseAndIndex(const std::string &absolutePath,
                                  const std::wstring &text,
                                  IndexedModule &mod) {
    auto savedFilePath = yoi::__current_file_path;
    yoi::set_current_file_path(yoi::string2wstring(absolutePath));

    try {
        std::wstringstream stream(text);
        yoi::lexer lex(std::move(stream));
        lex.scan();
        yoi::parse(mod.ast, lex);

        if (mod.ast) {
            SymbolExtractor extractor;
            mod.symbols = extractor.extract(mod.ast);
        }
    } catch (const std::runtime_error &) {
        // Parse failed — leave AST as nullptr, symbols empty
    }

    yoi::set_current_file_path(savedFilePath);
}

const yoi::vec<Symbol> *ProjectIndex::getModuleSymbols(const std::string &absolutePath) {
    auto it = modules.find(absolutePath);
    if (it == modules.end()) return nullptr;
    return &it->second.symbols;
}

void ProjectIndex::getAllSymbols(yoi::vec<Symbol> &out) {
    for (auto &[path, mod] : modules) {
        for (auto &sym : mod.symbols) {
            // Tag with source module path
            Symbol tagged = sym;
            // Store the source file path so we can navigate to it
            if (tagged.sourceFile.empty()) {
                tagged.sourceFile = yoi::string2wstring(path);
            }
            out.push_back(tagged);
        }
    }
}

const yoi::vec<Symbol> &ProjectIndex::indexAndGet(const std::string &absolutePath) {
    indexModule(absolutePath);
    auto it = modules.find(absolutePath);
    if (it != modules.end()) {
        return it->second.symbols;
    }
    static yoi::vec<Symbol> empty;
    return empty;
}

void ProjectIndex::invalidateModule(const std::string &absolutePath) {
    auto it = modules.find(absolutePath);
    if (it != modules.end()) {
        if (it->second.ast) {
            yoi::finalizeAST(it->second.ast);
        }
        modules.erase(it);
    }
}

} // namespace lsp
