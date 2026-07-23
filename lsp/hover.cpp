//
// Hover Provider implementation
//

#include "hover.h"
#include <algorithm>
#include <share/def.hpp>

namespace lsp {

std::optional<Hover> HoverProvider::provide(Document *doc, const Position &pos) {
    if (!doc || !doc->parseSucceeded) return std::nullopt;
    return hoverOnSymbol(doc, pos);
}

static Hover makeHoverForSymbol(const Symbol &sym) {
    Hover hover;
    hover.contents.kind = "markdown";

    std::string kindStr;
    switch (sym.kind) {
        case HoshiSymbolKind::Function:      kindStr = "**Function**"; break;
        case HoshiSymbolKind::Struct:        kindStr = "**Struct**"; break;
        case HoshiSymbolKind::Interface:     kindStr = "**Interface**"; break;
        case HoshiSymbolKind::Variable:      kindStr = "**Variable**"; break;
        case HoshiSymbolKind::TypeAlias:     kindStr = "**Type Alias**"; break;
        case HoshiSymbolKind::Enum:          kindStr = "**Enum**"; break;
        case HoshiSymbolKind::Module:
        case HoshiSymbolKind::ModuleAlias:  kindStr = "**Module**"; break;
        case HoshiSymbolKind::Import:        kindStr = "**Import**"; break;
        case HoshiSymbolKind::Export:        kindStr = "**Export**"; break;
        case HoshiSymbolKind::Concept_:      kindStr = "**Concept**"; break;
        case HoshiSymbolKind::Method:        kindStr = "**Method**"; break;
        case HoshiSymbolKind::Field:         kindStr = "**Field**"; break;
        case HoshiSymbolKind::Constructor:   kindStr = "**Constructor**"; break;
        case HoshiSymbolKind::Finalizer:     kindStr = "**Finalizer**"; break;
        case HoshiSymbolKind::EnumMember:    kindStr = "**Enum Member**"; break;
        default:                             kindStr = "**Symbol**"; break;
    }

    hover.contents.value = "```hoshi\n" + yoi::wstring2string(sym.detail) + "\n```\n" + kindStr;
    if (!sym.parentName.empty() && sym.kind != HoshiSymbolKind::ModuleAlias)
        hover.contents.value += "\n\nParent: `" + yoi::wstring2string(sym.parentName) + "`";
    if (!sym.typeInfo.empty())
        hover.contents.value += "\n\nType: `" + yoi::wstring2string(sym.typeInfo) + "`";
    if (!sym.sourceFile.empty())
        hover.contents.value += "\n\nDefined in: `" + yoi::wstring2string(sym.sourceFile) + "`";
    if (!sym.importPath.empty())
        hover.contents.value += "\n\nModule path: `" + yoi::wstring2string(sym.importPath) + "`";

    return hover;
}

std::optional<Hover> HoverProvider::hoverOnSymbol(Document *doc, const Position &pos) {
    int line = static_cast<int>(pos.line);
    int col = static_cast<int>(pos.character);

    // Extract the word at the cursor position
    std::string lineText;
    {
        int lineIdx = 0;
        size_t start = 0;
        std::string text = yoi::wstring2string(doc->text);
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '\n') {
                if (lineIdx == line) {
                    lineText = text.substr(start, i - start);
                    break;
                }
                lineIdx++;
                start = i + 1;
            }
        }
        if (lineText.empty() && lineIdx == line) {
            lineText = text.substr(start);
        }
    }

    if (col > static_cast<int>(lineText.size())) return std::nullopt;

    // Find word boundaries at cursor
    int wordStart = col;
    int wordEnd = col;
    while (wordStart > 0 && (std::isalnum(lineText[wordStart - 1]) || lineText[wordStart - 1] == '_'))
        wordStart--;
    while (wordEnd < static_cast<int>(lineText.size()) && (std::isalnum(lineText[wordEnd]) || lineText[wordEnd] == '_'))
        wordEnd++;

    if (wordStart == wordEnd) return std::nullopt;

    std::string word = lineText.substr(wordStart, wordEnd - wordStart);
    yoi::wstr wWord = yoi::string2wstring(word);

    // Check for dot-access: if there's an identifier before a dot, it's a module/struct member
    std::string beforeWord = lineText.substr(0, wordStart);
    yoi::wstr parentName;
    size_t lastDot = beforeWord.rfind('.');
    if (lastDot != std::string::npos) {
        // Extract the identifier before the dot
        std::string before = beforeWord.substr(0, lastDot);
        size_t idEnd = before.size();
        size_t idStart = idEnd;
        while (idStart > 0 && (std::isalnum(before[idStart - 1]) || before[idStart - 1] == '_'))
            idStart--;
        if (idStart < idEnd) {
            parentName = yoi::string2wstring(before.substr(idStart, idEnd - idStart));
        }
    }

    // Search in document-local symbols
    for (auto &sym : doc->symbols) {
        if (sym.name == wWord && static_cast<int>(sym.line) == line &&
            isSymbolVisibleAt(doc->symbols, sym, static_cast<yoi::indexT>(line))) {
            return makeHoverForSymbol(sym);
        }
        // Check children
        for (auto &child : sym.children) {
            if (child.name == wWord && static_cast<int>(child.line) == line) {
                return makeHoverForSymbol(child);
            }
        }
    }

    // If we have a parent (dot access), search cross-module symbols under that parent
    if (!parentName.empty()) {
        for (auto &cross : doc->crossModuleSymbols) {
            if (cross.isLocal) continue;
            if (cross.name == wWord && cross.parentName == parentName) {
                return makeHoverForSymbol(cross);
            }
        }
    }

    // Broader search: any local symbol with matching name (any line)
    for (auto &sym : doc->symbols) {
        if (!isSymbolVisibleAt(doc->symbols, sym, static_cast<yoi::indexT>(line))) continue;
        if (sym.name == wWord) {
            return makeHoverForSymbol(sym);
        }
        for (auto &child : sym.children) {
            if (child.name == wWord) {
                return makeHoverForSymbol(child);
            }
        }
    }

    // Cross-module search (any parent)
    for (auto &cross : doc->crossModuleSymbols) {
        if (cross.isLocal) continue;
        if (cross.name == wWord) {
            return makeHoverForSymbol(cross);
        }
    }

    // Search all indexed modules (builtin, etc.)
    if (doc->projectIndex) {
        doc->projectIndex->indexAndGet("builtin");
        yoi::vec<Symbol> allSyms;
        doc->projectIndex->getAllSymbols(allSyms);
        for (auto &sym : allSyms) {
            if (sym.name == wWord) {
                return makeHoverForSymbol(sym);
            }
            for (auto &child : sym.children) {
                if (child.name == wWord) {
                    return makeHoverForSymbol(child);
                }
            }
        }
    }

    return std::nullopt;
}

} // namespace lsp
