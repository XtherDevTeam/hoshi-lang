//
// Project Index — multi-file module resolution and symbol cache
//

#ifndef HOSHI_LANG_LSP_PROJECT_INDEX_H
#define HOSHI_LANG_LSP_PROJECT_INDEX_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <share/def.hpp>
#include <compiler/frontend/ast.hpp>
#include "symbolExtractor.h"

namespace lsp {

class ProjectIndex {
public:
    ProjectIndex();

    void setSearchPaths(const std::vector<std::string> &paths);
    void addSearchPath(const std::string &path);
    const std::vector<std::string> &getSearchPaths() const { return searchPaths; }

    /// Resolve an import path string to an absolute file path.
    /// @param importPath  The path string from use/import statement (e.g. "lib/math")
    /// @param relativeToFile  Absolute path of the file containing the import (for local search dirs)
    /// @return Resolved absolute path, or empty string if not found
    std::string resolveModule(const std::string &importPath,
                              const std::string &relativeToFile);

    /// Parse and index a module file at an absolute path.
    /// Does nothing if the module is already indexed.
    void indexModule(const std::string &absolutePath);

    /// Get symbols for a specific module.
    const yoi::vec<Symbol> *getModuleSymbols(const std::string &absolutePath);

    /// Collect all symbols from all indexed modules into flat list.
    void getAllSymbols(yoi::vec<Symbol> &out);

    /// Parse and index a module, then return its symbols.
    const yoi::vec<Symbol> &indexAndGet(const std::string &absolutePath);

    /// Clear a module from the cache (e.g., when it changes on disk).
    void invalidateModule(const std::string &absolutePath);

private:
    struct IndexedModule {
        yoi::hoshiModule *ast = nullptr;
        yoi::vec<Symbol> symbols;
        yoi::wstr text;
    };

    std::vector<std::string> searchPaths;
    std::map<std::string, IndexedModule> modules;

    std::string tryResolve(const std::string &searchDir, const std::string &importPath);

    void parseAndIndex(const std::string &absolutePath, const std::wstring &text,
                       IndexedModule &mod);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_PROJECT_INDEX_H
