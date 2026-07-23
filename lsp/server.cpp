//
// LSP Server implementation — JSON-RPC I/O and message dispatch
//

#include "server.h"
#include "completion.h"
#include "hover.h"
#include "definition.h"
#include <climits>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <share/def.hpp>

namespace lsp {

LspServer::LspServer() {
    documents.setProjectIndex(&projectIndex);
    compilerCtx = std::make_shared<yoi::compilerContext>();
    // Set a minimal build config immediately so the visitor doesn't crash
    auto buildConfig = yoi::IRBuildConfig::Builder()
        .setBuildType(yoi::IRBuildConfig::BuildType::executable)
        .setBuildPlatform(yoi::string2wstring(YOI_PLATFORM))
        .setBuildArch(yoi::string2wstring(YOI_ARCH))
        .setBuildMode(yoi::IRBuildConfig::BuildMode::debug)
        .setSearchPaths({L"", (std::filesystem::path(yoi::whereIsHoshiLang()) / ".." / "lib").wstring()})
        .setMarco(L"platform", yoi::string2wstring(YOI_PLATFORM))
        .setMarco(L"arch", yoi::string2wstring(YOI_ARCH))
        .setMarco(L"hoshi_feature_version", yoi::string2wstring(HOSHI_LANG_VERSION))
        .setMarco(L"hoshi_lang_commit", yoi::string2wstring(HOSHI_LANG_GIT_COMMIT_HASH))
        .setImmediatelyClearupCache(true)
        .yield();
    compilerCtx->setBuildConfig(buildConfig);
    // Defer initializeSharedObjects() — heavy, done in handleInitialized
    documents.setCompilerContext(compilerCtx);
}

LspServer::~LspServer() = default;

// ============================================================
// JSON-RPC I/O (stdio transport)
// ============================================================

std::string LspServer::readMessage() {
    // Read Content-Length header
    std::string line;
    int contentLength = 0;

    while (std::getline(std::cin, line)) {
        // Trim trailing \r
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty()) break;  // End of headers

        if (line.rfind("Content-Length:", 0) == 0) {
            std::string lenStr = line.substr(15);
            // Trim leading space
            size_t pos = lenStr.find_first_not_of(" \t");
            if (pos != std::string::npos) lenStr = lenStr.substr(pos);
            contentLength = std::stoi(lenStr);
        }
    }

    if (contentLength <= 0) return "";

    // Read the JSON body
    std::string content(contentLength, '\0');
    std::cin.read(&content[0], contentLength);
    return content;
}

void LspServer::writeMessage(const std::string &content) {
    std::ostringstream header;
    header << "Content-Length: " << content.size() << "\r\n\r\n";
    std::cout << header.str() << content << std::flush;
}

void LspServer::sendResponse(const json &id, const json &result) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };
    writeMessage(response.dump());
}

void LspServer::sendError(const json &id, int code, const std::string &message) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {{"code", code}, {"message", message}}}
    };
    writeMessage(response.dump());
}

void LspServer::sendNotification(const std::string &method, const json &params) {
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    writeMessage(notification.dump());
}

// ============================================================
// Message dispatch
// ============================================================

void LspServer::handleMessage(const json &msg) {
    std::string method = msg.value("method", "");
    json id = msg.contains("id") ? msg["id"] : json();
    json params = msg.contains("params") ? msg["params"] : json::object();

    bool isRequest = msg.contains("id");

    if (method == "initialize") {
        handleInitialize(id, params);
    } else if (method == "initialized") {
        handleInitialized(params);
    } else if (method == "shutdown") {
        handleShutdown(id);
    } else if (method == "exit") {
        handleExit();
    } else if (method == "textDocument/didOpen") {
        handleDidOpen(params);
    } else if (method == "textDocument/didChange") {
        handleDidChange(params);
    } else if (method == "textDocument/didClose") {
        handleDidClose(params);
    } else if (method == "textDocument/completion") {
        handleCompletion(id, params);
    } else if (method == "textDocument/hover") {
        handleHover(id, params);
    } else if (method == "textDocument/definition") {
        handleDefinition(id, params);
    } else if (method == "textDocument/references") {
        handleReferences(id, params);
    } else if (method == "textDocument/documentSymbol") {
        handleDocumentSymbol(id, params);
    } else if (isRequest) {
        // Unhandled request — return method not found
        sendError(id, -32601, "Method not found: " + method);
    }
    // Notifications with no handler are silently ignored
}

// ============================================================
// LSP Lifecycle
// ============================================================

void LspServer::handleInitialize(const json &id, const json &params) {
    // Extract workspace root for module search paths
    if (params.contains("rootUri") && params["rootUri"].is_string()) {
        workspaceRoot = params["rootUri"].get<std::string>();
        // Strip file:// prefix
        if (workspaceRoot.rfind("file://", 0) == 0) {
            workspaceRoot = workspaceRoot.substr(7);
        }
        projectIndex.addSearchPath(workspaceRoot);
        projectIndex.addSearchPath(workspaceRoot + "/.tsuki_modules");
        projectIndex.addSearchPath(workspaceRoot + "/lib");
    }

    // Also check rootPath (older LSP clients)
    if (params.contains("rootPath") && params["rootPath"].is_string() && workspaceRoot.empty()) {
        workspaceRoot = params["rootPath"].get<std::string>();
        projectIndex.addSearchPath(workspaceRoot);
        projectIndex.addSearchPath(workspaceRoot + "/.tsuki_modules");
        projectIndex.addSearchPath(workspaceRoot + "/lib");
    }

    // Mirror the compiler's default include paths (main.cpp line 77):
    //   includeDirs{L"", (whereIsHoshiLang() / ".." / "lib").wstring()}
    // "" = current working directory
    projectIndex.addSearchPath(".");

    // Global hoshi library: ~/.hoshi/lib/  (whereIsHoshiLang() returns ~/.hoshi/bin)
    std::filesystem::path hoshiBinPath(yoi::wstring2string(yoi::whereIsHoshiLang()));
    std::filesystem::path hoshiLibPath = hoshiBinPath / ".." / "lib";
    std::error_code ec;
    // canonical() resolves .. and symlinks for a clean path
    std::string globalLib = std::filesystem::canonical(hoshiLibPath, ec).string();
    if (!ec) {
        projectIndex.addSearchPath(globalLib);
        projectIndex.addSearchPath(globalLib + "/.tsuki_modules");
    }

    InitializeResult result;
    sendResponse(id, json(result));
}

void LspServer::handleInitialized(const json &) {
    isInitialized = true;
    if (compilerCtx && !compilerInitialized) {
        try {
            compilerCtx->initializeSharedObjects();
            compilerInitialized = true;
        } catch (const std::exception &e) {
            std::cerr << "[lsp] compiler init failed: " << e.what() << std::endl;
        }
    }
}

void LspServer::handleShutdown(const json &id) {
    isShuttingDown = true;
    sendResponse(id, json(nullptr));
}

void LspServer::handleExit() {
    if (isShuttingDown)
        std::exit(0);
    else
        std::exit(1);
}

// ============================================================
// Document Sync
// ============================================================

void LspServer::handleDidOpen(const json &params) {
    DidOpenTextDocumentParams p = params.get<DidOpenTextDocumentParams>();
    documents.openDocument(p.textDocument.uri, p.textDocument.text,
                           p.textDocument.languageId, p.textDocument.version);
    publishDiagnostics(p.textDocument.uri, p.textDocument.version);
}

void LspServer::handleDidChange(const json &params) {
    DidChangeTextDocumentParams p = params.get<DidChangeTextDocumentParams>();
    if (!p.contentChanges.empty()) {
        // Full document sync (change = 1): take the full text from the last change
        std::string newText = p.contentChanges.back().text;
        documents.updateDocument(p.textDocument.uri, newText, p.textDocument.version);
    }
    publishDiagnostics(p.textDocument.uri, p.textDocument.version);
}

void LspServer::handleDidClose(const json &params) {
    DidCloseTextDocumentParams p = params.get<DidCloseTextDocumentParams>();
    documents.closeDocument(p.textDocument.uri);

    // Clear diagnostics for closed document
    PublishDiagnosticsParams pub;
    pub.uri = p.textDocument.uri;
    sendNotification("textDocument/publishDiagnostics", json(pub));
}

// ============================================================
// Diagnostics
// ============================================================

void LspServer::publishDiagnostics(const std::string &uri, int version) {
    Document *doc = documents.getDocument(uri);
    if (!doc) return;

    PublishDiagnosticsParams pub;
    pub.uri = uri;
    pub.version = version;

    for (auto &diag : doc->diagnostics) {
        LspDiagnostic ld;
        // Convert 1-based (internal) to 0-based (LSP)
        ld.range.start.line = static_cast<int>(diag.line) - 1;
        ld.range.start.character = static_cast<int>(diag.column) - 1;
        ld.range.end.line = ld.range.start.line;
        ld.range.end.character = ld.range.start.character + 1;

        switch (diag.severity) {
            case yoi::DiagnosticSeverity::Error:
                ld.severity = DiagnosticSeverity::Error;
                break;
            case yoi::DiagnosticSeverity::Warning:
                ld.severity = DiagnosticSeverity::Warning;
                break;
            case yoi::DiagnosticSeverity::Note:
                ld.severity = DiagnosticSeverity::Information;
                break;
            case yoi::DiagnosticSeverity::Hint:
                ld.severity = DiagnosticSeverity::Hint;
                break;
        }

        ld.message = diag.message;
        ld.source = "hoshi-lang";
        pub.diagnostics.push_back(ld);
    }

    sendNotification("textDocument/publishDiagnostics", json(pub));
}

// ============================================================
// Language Features
// ============================================================

void LspServer::handleCompletion(const json &id, const json &params) {
    CompletionParams p = params.get<CompletionParams>();
    Document *doc = documents.getDocument(p.textDocument.uri);

    CompletionProvider provider;
    CompletionList result = provider.provide(doc, p.position);

    sendResponse(id, json(result));
}

void LspServer::handleHover(const json &id, const json &params) {
    HoverParams p = params.get<HoverParams>();
    Document *doc = documents.getDocument(p.textDocument.uri);

    HoverProvider provider;
    std::optional<Hover> result = provider.provide(doc, p.position);

    if (result) {
        sendResponse(id, json(*result));
    } else {
        sendResponse(id, json(nullptr));
    }
}

void LspServer::handleDefinition(const json &id, const json &params) {
    DefinitionParams p = params.get<DefinitionParams>();
    Document *doc = documents.getDocument(p.textDocument.uri);

    DefinitionProvider provider;
    DefinitionResult result = provider.provide(doc, p.position, p.textDocument.uri);

    if (result.empty()) {
        sendResponse(id, json(nullptr));
    } else if (result.size() == 1) {
        sendResponse(id, json(result[0]));
    } else {
        sendResponse(id, json(result));
    }
}

void LspServer::handleReferences(const json &id, const json &) {
    // References not fully implemented yet
    sendResponse(id, json::array());
}

// Safe cast: yoi::indexT (uint64_t) → int for LSP positions.
// Clamps to [0, INT_MAX] to avoid negative values from overflow.
static int safeInt(yoi::indexT v) {
    return static_cast<int>(v > static_cast<yoi::indexT>(INT_MAX) ? INT_MAX : v);
}

// Map internal HoshiSymbolKind → LSP SymbolKind
static lsp::SymbolKind mapSymbolKind(HoshiSymbolKind k) {
    switch (k) {
        case HoshiSymbolKind::Function:    return SymbolKind::Function;
        case HoshiSymbolKind::Struct:      return SymbolKind::Struct;
        case HoshiSymbolKind::Interface:   return SymbolKind::Interface;
        case HoshiSymbolKind::Variable:    return SymbolKind::Variable;
        case HoshiSymbolKind::TypeAlias:   return SymbolKind::TypeParameter;
        case HoshiSymbolKind::Enum:        return SymbolKind::Enum;
        case HoshiSymbolKind::Module:
        case HoshiSymbolKind::ModuleAlias: return SymbolKind::Module;
        case HoshiSymbolKind::Method:      return SymbolKind::Method;
        case HoshiSymbolKind::Field:       return SymbolKind::Field;
        case HoshiSymbolKind::Constructor: return SymbolKind::Constructor;
        case HoshiSymbolKind::Finalizer:   return SymbolKind::Method;
        case HoshiSymbolKind::EnumMember:  return SymbolKind::EnumMember;
        case HoshiSymbolKind::Import:
        case HoshiSymbolKind::Export:      return SymbolKind::File;
        case HoshiSymbolKind::Concept_:    return SymbolKind::Interface;
        default:                           return SymbolKind::Object;
    }
}

void LspServer::handleDocumentSymbol(const json &id, const json &params) {
    auto uri = params["textDocument"]["uri"].get<std::string>();
    Document *doc = documents.getDocument(uri);

    std::vector<DocumentSymbol> result;

    if (doc && doc->parseSucceeded) {
        for (auto &sym : doc->symbols) {
            DocumentSymbol ds;
            ds.name = yoi::wstring2string(sym.name);
            ds.detail = yoi::wstring2string(sym.detail);
            ds.kind = mapSymbolKind(sym.kind);

            int sl = safeInt(sym.line);
            int sc = safeInt(sym.column);
            int el = safeInt(sym.endLine > 0 ? sym.endLine : sym.line);
            int ec = safeInt(sym.endColumn > 0 ? sym.endColumn : sym.column + sym.name.size());
            // Ensure end >= start
            if (ec <= sc) ec = sc + 1;
            if (el < sl) el = sl;

            ds.range.start.line = sl;
            ds.range.start.character = sc;
            ds.range.end.line = el;
            ds.range.end.character = ec;
            ds.selectionRange = ds.range;

            for (auto &child : sym.children) {
                DocumentSymbol cs;
                cs.name = yoi::wstring2string(child.name);
                cs.detail = yoi::wstring2string(child.detail);
                cs.kind = mapSymbolKind(child.kind);

                int cl = safeInt(child.line);
                int cc = safeInt(child.column);
                int cel = cl;
                int cec = cc + safeInt(child.name.size());
                if (cec <= cc) cec = cc + 1;

                cs.range.start.line = cl;
                cs.range.start.character = cc;
                cs.range.end.line = cel;
                cs.range.end.character = cec;
                cs.selectionRange = cs.range;
                ds.children.push_back(cs);
            }

            result.push_back(ds);
        }
    }

    sendResponse(id, json(result));
}

// ============================================================
// Main loop
// ============================================================

void LspServer::run() {
    while (!std::cin.eof() && !isShuttingDown) {
        try {
            std::string content = readMessage();
            if (content.empty()) {
                if (std::cin.eof()) break;
                continue;
            }
            json msg = json::parse(content);
            handleMessage(msg);
        } catch (const std::exception &e) {
            std::cerr << "[lsp] error: " << e.what() << std::endl;
        }
    }
}

} // namespace lsp
