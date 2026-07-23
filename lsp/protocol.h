//
// LSP Protocol Type Definitions for hoshi-lang
//

#ifndef HOSHI_LANG_LSP_PROTOCOL_H
#define HOSHI_LANG_LSP_PROTOCOL_H

#include <share/json.hpp>
#include <string>
#include <vector>
#include <optional>

namespace lsp {

using json = nlohmann::json;

// ============================================================
// Base JSON-RPC types
// ============================================================

// ============================================================
// Position & Range
// ============================================================

struct Position {
    int line = 0;       // 0-based
    int character = 0;  // 0-based
};

inline void to_json(json &j, const Position &p) {
    j = json{{"line", p.line}, {"character", p.character}};
}

inline void from_json(const json &j, Position &p) {
    j.at("line").get_to(p.line);
    j.at("character").get_to(p.character);
}

struct Range {
    Position start;
    Position end;
};

inline void to_json(json &j, const Range &r) {
    j = json{{"start", r.start}, {"end", r.end}};
}

inline void from_json(const json &j, Range &r) {
    j.at("start").get_to(r.start);
    j.at("end").get_to(r.end);
}

struct Location {
    std::string uri;
    Range range;
};

inline void to_json(json &j, const Location &l) {
    j = json{{"uri", l.uri}, {"range", l.range}};
}

inline void from_json(const json &j, Location &l) {
    j.at("uri").get_to(l.uri);
    j.at("range").get_to(l.range);
}

// ============================================================
// Text Document types
// ============================================================

struct TextDocumentIdentifier {
    std::string uri;
};

inline void to_json(json &j, const TextDocumentIdentifier &t) {
    j = json{{"uri", t.uri}};
}

inline void from_json(const json &j, TextDocumentIdentifier &t) {
    j.at("uri").get_to(t.uri);
}

struct VersionedTextDocumentIdentifier {
    std::string uri;
    int version = 0;
};

inline void to_json(json &j, const VersionedTextDocumentIdentifier &t) {
    j = json{{"uri", t.uri}, {"version", t.version}};
}

inline void from_json(const json &j, VersionedTextDocumentIdentifier &t) {
    j.at("uri").get_to(t.uri);
    j.at("version").get_to(t.version);
}

struct TextDocumentItem {
    std::string uri;
    std::string languageId;
    int version;
    std::string text;
};

inline void from_json(const json &j, TextDocumentItem &t) {
    j.at("uri").get_to(t.uri);
    j.at("languageId").get_to(t.languageId);
    j.at("version").get_to(t.version);
    j.at("text").get_to(t.text);
}

struct TextDocumentContentChangeEvent {
    std::optional<Range> range;
    std::optional<int> rangeLength;
    std::string text;
};

inline void from_json(const json &j, TextDocumentContentChangeEvent &t) {
    if (j.contains("range") && !j["range"].is_null())
        t.range = j["range"].get<Range>();
    if (j.contains("rangeLength") && !j["rangeLength"].is_null())
        t.rangeLength = j["rangeLength"].get<int>();
    j.at("text").get_to(t.text);
}

// ============================================================
// DidOpen / DidChange / DidClose Params
// ============================================================

struct DidOpenTextDocumentParams {
    TextDocumentItem textDocument;
};

inline void from_json(const json &j, DidOpenTextDocumentParams &p) {
    j.at("textDocument").get_to(p.textDocument);
}

struct DidChangeTextDocumentParams {
    VersionedTextDocumentIdentifier textDocument;
    std::vector<TextDocumentContentChangeEvent> contentChanges;
};

inline void from_json(const json &j, DidChangeTextDocumentParams &p) {
    j.at("textDocument").get_to(p.textDocument);
    j.at("contentChanges").get_to(p.contentChanges);
}

struct DidCloseTextDocumentParams {
    TextDocumentIdentifier textDocument;
};

inline void from_json(const json &j, DidCloseTextDocumentParams &p) {
    j.at("textDocument").get_to(p.textDocument);
}

// ============================================================
// Completion types
// ============================================================

struct CompletionParams {
    TextDocumentIdentifier textDocument;
    Position position;
};

inline void from_json(const json &j, CompletionParams &p) {
    j.at("textDocument").get_to(p.textDocument);
    j.at("position").get_to(p.position);
}

enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25,
};

struct CompletionItem {
    std::string label;
    CompletionItemKind kind = CompletionItemKind::Text;
    std::optional<std::string> detail;
    std::optional<std::string> documentation;
    std::optional<std::string> insertText;
    std::optional<std::string> sortText;
};

inline void to_json(json &j, const CompletionItem &c) {
    j = json{{"label", c.label}, {"kind", static_cast<int>(c.kind)}};
    if (c.detail) j["detail"] = *c.detail;
    if (c.documentation) j["documentation"] = *c.documentation;
    if (c.insertText) j["insertText"] = *c.insertText;
    if (c.sortText) j["sortText"] = *c.sortText;
}

struct CompletionList {
    bool isIncomplete = false;
    std::vector<CompletionItem> items;
};

inline void to_json(json &j, const CompletionList &cl) {
    j = json{{"isIncomplete", cl.isIncomplete}, {"items", cl.items}};
}

// ============================================================
// Hover types
// ============================================================

struct HoverParams {
    TextDocumentIdentifier textDocument;
    Position position;
};

inline void from_json(const json &j, HoverParams &p) {
    j.at("textDocument").get_to(p.textDocument);
    j.at("position").get_to(p.position);
}

struct MarkupContent {
    std::string kind;   // "markdown" or "plaintext"
    std::string value;
};

inline void to_json(json &j, const MarkupContent &m) {
    j = json{{"kind", m.kind}, {"value", m.value}};
}

struct Hover {
    MarkupContent contents;
    std::optional<Range> range;
};

inline void to_json(json &j, const Hover &h) {
    j = json{{"contents", h.contents}};
    if (h.range) j["range"] = *h.range;
}

// ============================================================
// Definition types
// ============================================================

struct DefinitionParams {
    TextDocumentIdentifier textDocument;
    Position position;
};

inline void from_json(const json &j, DefinitionParams &p) {
    j.at("textDocument").get_to(p.textDocument);
    j.at("position").get_to(p.position);
}

// Definition result is either a single Location or vector of Locations
using DefinitionResult = std::vector<Location>;

inline void to_json(json &j, const DefinitionResult &r) {
    if (r.size() == 1)
        j = r[0];
    else
        j = r;
}

// ============================================================
// References types
// ============================================================

struct ReferenceParams {
    TextDocumentIdentifier textDocument;
    Position position;
    bool includeDeclaration = true;
};

inline void from_json(const json &j, ReferenceParams &p) {
    j.at("textDocument").get_to(p.textDocument);
    j.at("position").get_to(p.position);
    if (j.contains("context") && j["context"].contains("includeDeclaration"))
        p.includeDeclaration = j["context"]["includeDeclaration"].get<bool>();
}

// ============================================================
// Document Symbol types
// ============================================================

enum class SymbolKind {
    File = 1,
    Module = 2,
    Namespace = 3,
    Package = 4,
    Class = 5,
    Method = 6,
    Property = 7,
    Field = 8,
    Constructor = 9,
    Enum = 10,
    Interface = 11,
    Function = 12,
    Variable = 13,
    Constant = 14,
    String = 15,
    Number = 16,
    Boolean = 17,
    Array = 18,
    Object = 19,
    Key = 20,
    Null = 21,
    EnumMember = 22,
    Struct = 23,
    Event = 24,
    Operator = 25,
    TypeParameter = 26,
};

struct DocumentSymbol {
    std::string name;
    std::string detail;
    SymbolKind kind;
    Range range;
    Range selectionRange;
    std::vector<DocumentSymbol> children;
};

inline void to_json(json &j, const DocumentSymbol &s) {
    j = {
        {"name", s.name},
        {"detail", s.detail},
        {"kind", static_cast<int>(s.kind)},
        {"range", s.range},
        {"selectionRange", s.selectionRange}
    };
    if (!s.children.empty())
        j["children"] = s.children;
}

// ============================================================
// PublishDiagnostics types
// ============================================================

enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

struct LspDiagnostic {
    Range range;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string message;
    std::optional<std::string> source;
};

inline void to_json(json &j, const LspDiagnostic &d) {
    j = {
        {"range", d.range},
        {"severity", static_cast<int>(d.severity)},
        {"message", d.message}
    };
    if (d.source) j["source"] = *d.source;
}

struct PublishDiagnosticsParams {
    std::string uri;
    int version = 0;
    std::vector<LspDiagnostic> diagnostics;
};

inline void to_json(json &j, const PublishDiagnosticsParams &p) {
    j = json{
        {"uri", p.uri},
        {"diagnostics", p.diagnostics}
    };
    if (p.version > 0) j["version"] = p.version;
}

// ============================================================
// Initialize types
// ============================================================

struct TextDocumentSyncOptions {
    bool openClose = true;
    int change = 1;   // 1 = full, 2 = incremental
    bool willSave = false;
    bool willSaveWaitUntil = false;
    bool didSave = false;
};

inline void to_json(json &j, const TextDocumentSyncOptions &o) {
    j = json{
        {"openClose", o.openClose},
        {"change", o.change},
        {"willSave", o.willSave},
        {"willSaveWaitUntil", o.willSaveWaitUntil}
    };
}

struct CompletionOptions {
    bool resolveProvider = false;
    std::vector<std::string> triggerCharacters = {".", ":", "\"", "/"};
};

inline void to_json(json &j, const CompletionOptions &o) {
    j = json{
        {"resolveProvider", o.resolveProvider},
        {"triggerCharacters", o.triggerCharacters}
    };
}

struct ServerCapabilities {
    TextDocumentSyncOptions textDocumentSync;
    CompletionOptions completionProvider;
    bool hoverProvider = true;
    bool definitionProvider = true;
    bool referencesProvider = false;
    bool documentSymbolProvider = true;
};

inline void to_json(json &j, const ServerCapabilities &c) {
    j = json{
        {"textDocumentSync", c.textDocumentSync},
        {"completionProvider", c.completionProvider},
        {"hoverProvider", c.hoverProvider},
        {"definitionProvider", c.definitionProvider},
        {"referencesProvider", c.referencesProvider},
        {"documentSymbolProvider", c.documentSymbolProvider}
    };
}

struct ServerInfo {
    std::string name = "hoshi-lsp";
    std::string version = "0.1.0";
};

inline void to_json(json &j, const ServerInfo &s) {
    j = json{{"name", s.name}, {"version", s.version}};
}

struct InitializeResult {
    ServerCapabilities capabilities;
    ServerInfo serverInfo;
};

inline void to_json(json &j, const InitializeResult &r) {
    j = json{
        {"capabilities", r.capabilities},
        {"serverInfo", r.serverInfo}
    };
}

// ============================================================
// Document Symbol params
// ============================================================

struct DocumentSymbolParams {
    std::string textDocument;  // uri
};

inline void from_json(const json &j, DocumentSymbolParams &p) {
    j.at("textDocument").at("uri").get_to(p.textDocument);
}

} // namespace lsp

#endif // HOSHI_LANG_LSP_PROTOCOL_H
