//
// Document Store implementation for hoshi-lang LSP
//

#include "document.h"
#include <compiler/frontend/lexer.hpp>
#include <compiler/frontend/parser.hpp>
#include <compiler/visitor/visitor.h>
#include <share/def.hpp>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace lsp {

Document::~Document() {
    if (ast) {
        yoi::finalizeAST(ast);
        ast = nullptr;
    }
}

void DocumentStore::openDocument(const std::string &uri, const std::string &text,
                                  const std::string &languageId, int version) {
    Document doc;
    doc.uri = uri;
    doc.languageId = languageId;
    doc.version = version;
    doc.text = yoi::string2wstring(text);
    doc.projectIndex = projectIndex;
    documents[uri] = std::move(doc);
    reparseDocument(uri);
}

void DocumentStore::updateDocument(const std::string &uri, const std::string &text, int version) {
    auto it = documents.find(uri);
    if (it == documents.end()) return;

    it->second.text = yoi::string2wstring(text);
    it->second.version = version;
    it->second.projectIndex = projectIndex;
    reparseDocument(uri);
}

void DocumentStore::closeDocument(const std::string &uri) {
    documents.erase(uri);
}

Document *DocumentStore::getDocument(const std::string &uri) {
    auto it = documents.find(uri);
    if (it == documents.end()) return nullptr;
    return &it->second;
}

void DocumentStore::reparseDocument(const std::string &uri) {
    auto it = documents.find(uri);
    if (it == documents.end()) return;

    Document &doc = it->second;

    // Clean up previous AST
    if (doc.ast) {
        yoi::finalizeAST(doc.ast);
        doc.ast = nullptr;
    }
    doc.symbols.clear();
    doc.crossModuleSymbols.clear();
    doc.diagnostics.clear();
    doc.parseSucceeded = false;

    // Create a diagnostic engine in Collect mode
    yoi::DiagnosticEngine engine(yoi::DiagnosticEngine::Mode::Collect);
    engine.setCurrentFilePath(yoi::string2wstring(uri));
    yoi::set_diagnostic_engine(&engine);

    // Set current file for error reporting
    auto savedFilePath = yoi::__current_file_path;
    yoi::set_current_file_path(yoi::string2wstring(uri));

    try {
        // Lex
        std::wstringstream stream(doc.text);
        yoi::lexer lex(std::move(stream));
        lex.scan();

        // Parse
        yoi::parse(doc.ast, lex);

        if (doc.ast) {
            doc.parseSucceeded = true;

            // Collect parse diagnostics now, before the visitor pollutes the engine
            doc.diagnostics = engine.getDiagnostics();
            engine.clear();

            // Extract symbols from this document
            SymbolExtractor extractor;
            doc.symbols = extractor.extract(doc.ast);

            // Run the visitor in a separate diagnostic scope so its
            // scope-fallback panics don't become user-facing diagnostics
            yoi::DiagnosticEngine visitorEngine(yoi::DiagnosticEngine::Mode::Collect);
            yoi::set_diagnostic_engine(&visitorEngine);

            if (compilerCtx && doc.ast && compilerCtx->getBuildConfig()) {
                if (!compilerCtx->isInitialized()) {
                    compilerCtx->initializeSharedObjects();
                }
                std::string fp = uri;
                if (fp.rfind("file://", 0) == 0) fp = fp.substr(7);
                auto &searchPaths = compilerCtx->getBuildConfig()->searchPaths;
                auto parentPath = std::filesystem::path(fp).parent_path().wstring();
                searchPaths.push_back(parentPath);
                searchPaths.push_back((std::filesystem::path(fp).parent_path() / ".tsuki_modules").wstring());

                auto irMod = std::make_shared<yoi::IRModule>();
                irMod->modulePath = yoi::string2wstring(fp);
                // Register in moduleImported with a unique index (mirrors compileModule)
                static std::atomic<yoi::indexT> nextModuleId{1000};
                yoi::indexT modIdx = nextModuleId++;
                irMod->identifier = modIdx;
                compilerCtx->registerModule(modIdx, irMod);
                auto modCtx = std::make_shared<yoi::moduleContext>(
                    compilerCtx, yoi::string2wstring(fp), doc.ast);
                yoi::visitor vis(modCtx, irMod, modIdx);
                try {
                    vis.visit();
                } catch (const std::runtime_error &e) {
                    std::cerr << "[lsp] visitor error: " << e.what() << std::endl;
                }

                searchPaths.pop_back();
                searchPaths.pop_back();
                doc.irModule = irMod;
                doc.compilerCtx = compilerCtx;
            }

            // Restore the parse diagnostic engine
            yoi::set_diagnostic_engine(&engine);

            // Resolve and index imported modules
            resolveAndIndexImports(uri, doc);
        }
    } catch (const std::runtime_error &e) {
        // Parse failed — diagnostics are already in the engine
        doc.parseSucceeded = false;
        std::cerr << "[hoshi-lsp] parse failed for " << uri << ": " << e.what() << std::endl;
    }

    // Collect diagnostics
    doc.diagnostics = engine.getDiagnostics();

    // Restore state
    yoi::set_diagnostic_engine(nullptr);
    yoi::set_current_file_path(savedFilePath);
}

void DocumentStore::resolveAndIndexImports(const std::string &uri, Document &doc) {
    if (!projectIndex || !doc.ast) return;

    // Convert file:// URI to filesystem path
    std::string filePath = uri;
    if (filePath.rfind("file://", 0) == 0) {
        filePath = filePath.substr(7);
    }

    // Walk the AST to find use and import statements
    for (auto *stmt : doc.ast->stmts) {
        if (!stmt) continue;

        std::string importPathStr;
        yoi::wstr aliasName;

        if (stmt->kind == yoi::globalStmt::vKind::useStmt) {
            auto *useStmt = static_cast<yoi::useStmt *>(stmt->value.ptr);
            if (!useStmt) continue;
            importPathStr = yoi::wstring2string(useStmt->path.strVal);
            aliasName = useStmt->name->node.strVal;
        } else if (stmt->kind == yoi::globalStmt::vKind::importDecl) {
            auto *importStmt = static_cast<yoi::importDecl *>(stmt->value.ptr);
            if (!importStmt) continue;
            importPathStr = yoi::wstring2string(importStmt->from_path.strVal);
            if (importStmt->inner && importStmt->inner->name && importStmt->inner->name->id) {
                aliasName = importStmt->inner->name->id->node.strVal;
            }
        } else {
            continue;
        }

        if (importPathStr.empty()) continue;

        // Resolve the module path
        std::string resolvedPath = projectIndex->resolveModule(importPathStr, filePath);
        if (resolvedPath.empty()) continue;

        // Index it and get its symbols
        const auto &modSymbols = projectIndex->indexAndGet(resolvedPath);

        // Tag symbols with module alias and add to crossModuleSymbols
        for (const auto &sym : modSymbols) {
            if (sym.isLocal) continue;
            Symbol tagged = sym;
            tagged.sourceFile = yoi::string2wstring(resolvedPath);
            // Set module alias as parent for top-level symbols only.
            // Symbols that already have a parent (struct methods, etc.)
            // keep their original parentName so the hierarchy is preserved.
            if (!aliasName.empty() && tagged.parentName.empty()) {
                tagged.parentName = aliasName;
            }
            doc.crossModuleSymbols.push_back(tagged);
        }
    }
}

} // namespace lsp
