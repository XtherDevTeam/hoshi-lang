//
// Document Store for hoshi-lang LSP server
//

#ifndef HOSHI_LANG_LSP_DOCUMENT_H
#define HOSHI_LANG_LSP_DOCUMENT_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <share/def.hpp>
#include <compiler/compilerContext.h>
#include <compiler/frontend/ast.hpp>
#include <compiler/ir/IR.h>
#include <compiler/moduleContext.h>
#include <compiler/diagnostics/diagnosticEngine.h>
#include "symbolExtractor.h"
#include "projectIndex.h"

namespace lsp {

struct Document {
    std::string uri;
    std::string languageId;
    int version = 0;
    yoi::wstr text;
    yoi::hoshiModule *ast = nullptr;
    yoi::vec<Symbol> symbols;        // Symbols defined in this document
    yoi::vec<Symbol> crossModuleSymbols; // Symbols from imported/used modules (flattened)
    yoi::vec<yoi::Diagnostic> diagnostics;
    bool parseSucceeded = false;
    ProjectIndex *projectIndex = nullptr; // For fallback module resolution
    std::shared_ptr<yoi::IRModule> irModule;     // Type info from the visitor
    std::shared_ptr<yoi::compilerContext> compilerCtx; // For running the visitor

    ~Document();
};

class DocumentStore {
public:
    void setProjectIndex(ProjectIndex *index) { projectIndex = index; }
    ProjectIndex *getProjectIndex() { return projectIndex; }
    void setCompilerContext(std::shared_ptr<yoi::compilerContext> ctx) { compilerCtx = std::move(ctx); }

    void openDocument(const std::string &uri, const std::string &text,
                      const std::string &languageId, int version);
    void updateDocument(const std::string &uri, const std::string &text, int version);
    void closeDocument(const std::string &uri);

    Document *getDocument(const std::string &uri);
    void reparseDocument(const std::string &uri);

    const std::map<std::string, Document> &getDocuments() const { return documents; }

private:
    std::map<std::string, Document> documents;
    ProjectIndex *projectIndex = nullptr;
    std::shared_ptr<yoi::compilerContext> compilerCtx;

    /// Resolve import/use references in the document AST and index referenced modules.
    void resolveAndIndexImports(const std::string &uri, Document &doc);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_DOCUMENT_H
