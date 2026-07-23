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
    HoshiSymbolKind kind;
    yoi::indexT line;       // 0-based
    yoi::indexT column;     // 0-based
    yoi::indexT endLine;    // 0-based
    yoi::indexT endColumn;  // 0-based
    yoi::wstr detail;       // e.g. "func foo(a: int, b: string): bool"
    yoi::wstr typeInfo;     // e.g. "int" or "string"
    yoi::wstr parentName;   // e.g. struct name for methods/fields
    yoi::wstr sourceFile;   // absolute path of the defining module (for cross-module tracking)
    yoi::wstr importPath;   // for ModuleAlias/Import: the resolved module path
    std::vector<Symbol> children;
};

class SymbolExtractor {
public:
    yoi::vec<Symbol> extract(yoi::hoshiModule *module);

private:
    yoi::vec<Symbol> symbols;

    void visitGlobal(yoi::globalStmt *stmt);
    void visitFuncDef(yoi::funcDefStmt *stmt);
    void visitStructDef(yoi::structDefStmt *stmt);
    void visitInterfaceDef(yoi::interfaceDefStmt *stmt);
    void visitLetStmt(yoi::letStmt *stmt, const yoi::wstr &inParent = L"");
    void visitTypeAlias(yoi::typeAliasStmt *stmt);
    void visitEnum(yoi::enumerationDefinition *stmt);
    void visitImport(yoi::importDecl *stmt);
    void visitExport(yoi::exportDecl *stmt);
    void visitImpl(yoi::implStmt *stmt);
    void visitUse(yoi::useStmt *stmt);
    void visitDataStruct(yoi::dataStructDefStmt *stmt);
    void visitConceptDef(yoi::conceptDefinition *stmt);
    void visitInCodeBlock(yoi::inCodeBlockStmt *stmt, const yoi::wstr &inParent = L"");

    yoi::wstr formatTypeSpec(yoi::typeSpec *spec);
    yoi::wstr formatDefinitionArgs(yoi::definitionArguments *args);
    yoi::wstr formatIdentifierWithDefTemplateArg(yoi::identifierWithDefTemplateArg *id);
    yoi::wstr formatIdentifierWithTemplateArg(yoi::identifierWithTemplateArg *id);
    yoi::wstr formatExternModuleAccessExpression(yoi::externModuleAccessExpression *expr);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_SYMBOL_EXTRACTOR_H
