//
// Symbol Extractor for hoshi-lang LSP server
// Walks the AST and extracts symbol definitions
//

#ifndef HOSHI_LANG_LSP_SYMBOL_EXTRACTOR_H
#define HOSHI_LANG_LSP_SYMBOL_EXTRACTOR_H

#include <string>
#include <vector>
#include <share/def.hpp>
#include <compiler/frontend/ast.hpp>

namespace lsp {

enum class HoshiSymbolKind {
    Function,
    Struct,
    Interface,
    Variable,
    TypeAlias,
    Enum,
    Module,
    ModuleAlias,    // use alias "path" — the alias name
    Method,
    Field,
    Constructor,
    Finalizer,
    EnumMember,
    Import,         // import func from "path"
    Export,
    Concept_,
    Lambda,
};

struct Symbol {
    yoi::wstr name;
    HoshiSymbolKind kind = HoshiSymbolKind::Variable;
    yoi::indexT line = 0;       // 0-based
    yoi::indexT column = 0;     // 0-based
    yoi::indexT endLine = 0;    // 0-based
    yoi::indexT endColumn = 0;  // 0-based
    yoi::wstr detail;       // e.g. "func foo(a: int, b: string): bool"
    yoi::wstr typeInfo;     // e.g. "int" or "string"
    yoi::wstr parentName;   // e.g. struct name for methods/fields
    yoi::wstr sourceFile;   // absolute path of the defining module (for cross-module tracking)
    yoi::wstr importPath;   // for ModuleAlias/Import: the resolved module path
    // Locals are retained for editor features, but must not be exported as module symbols.
    bool isLocal = false;
    yoi::indexT ownerLine = 0; // Declaration line of the callable that owns this local.
    std::vector<Symbol> children;
};

inline bool isCallableSymbol(const Symbol &symbol) {
    return symbol.kind == HoshiSymbolKind::Function ||
           symbol.kind == HoshiSymbolKind::Method ||
           symbol.kind == HoshiSymbolKind::Constructor ||
           symbol.kind == HoshiSymbolKind::Finalizer;
}

// A local is visible only in the callable body that owns it.
// This keeps locals out of global lookups while preserving completion/navigation in bodies.
inline bool isSymbolVisibleAt(const yoi::vec<Symbol> &symbols, const Symbol &symbol,
                              yoi::indexT line) {
    if (!symbol.isLocal) return true;
    if (symbol.line > line) return false;

    const Symbol *owner = nullptr;
    for (const auto &candidate : symbols) {
        if (candidate.isLocal || !isCallableSymbol(candidate) || candidate.line > line ||
            candidate.endLine < line) {
            continue;
        }
        if (!owner || candidate.line > owner->line) owner = &candidate;
    }

    return owner && owner->line == symbol.ownerLine && owner->name == symbol.parentName;
}

class SymbolExtractor {
public:
    yoi::vec<Symbol> extract(yoi::hoshiModule *module);

private:
    yoi::vec<Symbol> symbols;

    void visitGlobal(yoi::globalStmt *stmt);
    void visitFuncDef(yoi::funcDefStmt *stmt);
    void visitStructDef(yoi::structDefStmt *stmt);
    void visitInterfaceDef(yoi::interfaceDefStmt *stmt);
    void visitLetStmt(yoi::letStmt *stmt, const yoi::wstr &inParent = L"",
                      yoi::indexT ownerLine = 0);
    void visitTypeAlias(yoi::typeAliasStmt *stmt);
    void visitEnum(yoi::enumerationDefinition *stmt);
    void visitImport(yoi::importDecl *stmt);
    void visitExport(yoi::exportDecl *stmt);
    void visitImpl(yoi::implStmt *stmt);
    void visitUse(yoi::useStmt *stmt);
    void visitDataStruct(yoi::dataStructDefStmt *stmt);
    void visitConceptDef(yoi::conceptDefinition *stmt);
    void visitInCodeBlock(yoi::inCodeBlockStmt *stmt, const yoi::wstr &inParent = L"",
                          yoi::indexT ownerLine = 0);

    yoi::wstr formatTypeSpec(yoi::typeSpec *spec);
    yoi::wstr formatDefinitionArgs(yoi::definitionArguments *args);
    yoi::wstr formatIdentifierWithDefTemplateArg(yoi::identifierWithDefTemplateArg *id);
    yoi::wstr formatIdentifierWithTemplateArg(yoi::identifierWithTemplateArg *id);
    yoi::wstr formatExternModuleAccessExpression(yoi::externModuleAccessExpression *expr);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_SYMBOL_EXTRACTOR_H
