//
// LSP Server — JSON-RPC transport and message dispatch
//

#ifndef HOSHI_LANG_LSP_SERVER_H
#define HOSHI_LANG_LSP_SERVER_H

#include <string>
#include <share/json.hpp>
#include "protocol.h"
#include "document.h"
#include "projectIndex.h"
#include <compiler/compilerContext.h>

namespace lsp {

class LspServer {
public:
    LspServer();
    ~LspServer();
    void run();

private:
    // ---- JSON-RPC I/O ----
    std::string readMessage();
    void writeMessage(const std::string &content);
    void handleMessage(const json &msg);

    void sendResponse(const json &id, const json &result);
    void sendError(const json &id, int code, const std::string &message);
    void sendNotification(const std::string &method, const json &params);

    // ---- LSP Lifecycle ----
    void handleInitialize(const json &id, const json &params);
    void handleInitialized(const json &params);
    void handleShutdown(const json &id);
    void handleExit();

    // ---- Document Sync ----
    void handleDidOpen(const json &params);
    void handleDidChange(const json &params);
    void handleDidClose(const json &params);

    // ---- Language Features ----
    void handleCompletion(const json &id, const json &params);
    void handleHover(const json &id, const json &params);
    void handleDefinition(const json &id, const json &params);
    void handleReferences(const json &id, const json &params);
    void handleDocumentSymbol(const json &id, const json &params);

    // ---- Diagnostics ----
    void publishDiagnostics(const std::string &uri, int version);

    // ---- State ----
    DocumentStore documents;
    ProjectIndex projectIndex;
    std::shared_ptr<yoi::compilerContext> compilerCtx;
    std::string workspaceRoot;
    bool isInitialized = false;
    bool compilerInitialized = false;
    bool isShuttingDown = false;
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_SERVER_H
