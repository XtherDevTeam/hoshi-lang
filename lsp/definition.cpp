//
// Go-to-Definition Provider — supports cross-module navigation and use-statement file jumps
//

#include "definition.h"
#include <algorithm>
#include <share/def.hpp>

namespace lsp {

static std::string getLineText(Document *doc, int line) {
    int lineIdx = 0;
    size_t start = 0;
    std::string text = yoi::wstring2string(doc->text);
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            if (lineIdx == line) return text.substr(start, i - start);
            lineIdx++;
            start = i + 1;
        }
    }
    return (lineIdx == line) ? text.substr(start) : "";
}

static std::string extractWordAtPosition(const std::string &lineText, int col) {
    if (col > static_cast<int>(lineText.size())) return "";
    int wordStart = col, wordEnd = col;
    while (wordStart > 0 && (std::isalnum(lineText[wordStart - 1]) || lineText[wordStart - 1] == '_'))
        wordStart--;
    while (wordEnd < static_cast<int>(lineText.size()) && (std::isalnum(lineText[wordEnd]) || lineText[wordEnd] == '_'))
        wordEnd++;
    if (wordStart == wordEnd) return "";
    return lineText.substr(wordStart, wordEnd - wordStart);
}

static Location makeLocation(const std::string &uri, yoi::indexT line, yoi::indexT column, size_t nameLen) {
    Location loc;
    loc.uri = uri;
    loc.range.start.line = static_cast<int>(line);
    loc.range.start.character = static_cast<int>(column);
    loc.range.end.line = static_cast<int>(line);
    loc.range.end.character = static_cast<int>(column + nameLen);
    return loc;
}

DefinitionResult DefinitionProvider::provide(Document *doc, const Position &pos, const std::string &uri) {
    DefinitionResult result;
    if (!doc || !doc->parseSucceeded) return result;

    int line = static_cast<int>(pos.line);
    int col = static_cast<int>(pos.character);
    std::string fileUri = uri;
    std::string lineText = getLineText(doc, line);

    // ---- "use <name> \"<path>\"" — jump to the module file ----
    std::string trimmed = lineText;
    while (!trimmed.empty() && std::isspace(trimmed.front())) trimmed.erase(0, 1);
    if (trimmed.rfind("use ", 0) == 0) {
        size_t pathStart = trimmed.find('"');
        if (pathStart != std::string::npos) {
            size_t pathEnd = trimmed.find('"', pathStart + 1);
            if (pathEnd != std::string::npos) {
                std::string importPath = trimmed.substr(pathStart + 1, pathEnd - pathStart - 1);
                if (!importPath.empty() && doc->projectIndex) {
                    std::string fp = uri;
                    if (fp.rfind("file://", 0) == 0) fp = fp.substr(7);
                    std::string resolved = doc->projectIndex->resolveModule(importPath, fp);
                    if (!resolved.empty()) {
                        result.push_back(makeLocation("file://" + resolved, 0, 0, 0));
                        return result;
                    }
                }
            }
        }
    }

    // ---- Extract word at cursor ----
    std::string word = extractWordAtPosition(lineText, col);
    if (word.empty()) return result;
    yoi::wstr wWord = yoi::string2wstring(word);

    // Check for dot-access prefix
    yoi::wstr parentName;
    {
        std::string beforeCursor = lineText.substr(0, col);
        size_t lastDot = beforeCursor.rfind('.');
        if (lastDot != std::string::npos) {
            std::string before = beforeCursor.substr(0, lastDot);
            size_t idEnd = before.size(), idStart = idEnd;
            while (idStart > 0 && (std::isalnum(before[idStart - 1]) || before[idStart - 1] == '_'))
                idStart--;
            if (idStart < idEnd)
                parentName = yoi::string2wstring(before.substr(idStart, idEnd - idStart));
        }
    }

    // Search document-local symbols
    for (auto &sym : doc->symbols) {
        if (sym.name == wWord) {
            result.push_back(makeLocation(fileUri, sym.line, sym.column, word.size()));
            return result;
        }
        for (auto &child : sym.children) {
            if (child.name == wWord) {
                result.push_back(makeLocation(fileUri, child.line, child.column, word.size()));
                return result;
            }
        }
    }

    // Dot-access: search cross-module symbols
    if (!parentName.empty()) {
        for (auto &cross : doc->crossModuleSymbols) {
            if (cross.name == wWord && cross.parentName == parentName) {
                if (!cross.sourceFile.empty()) {
                    result.push_back(makeLocation("file://" + yoi::wstring2string(cross.sourceFile), cross.line, cross.column, word.size()));
                    return result;
                }
                result.push_back(makeLocation(fileUri, cross.line, cross.column, word.size()));
                return result;
            }
        }
    }

    // Cross-module search without parent
    for (auto &cross : doc->crossModuleSymbols) {
        if (cross.name == wWord) {
            if (!cross.sourceFile.empty()) {
                result.push_back(makeLocation("file://" + yoi::wstring2string(cross.sourceFile), cross.line, cross.column, word.size()));
            } else {
                result.push_back(makeLocation(fileUri, cross.line, cross.column, word.size()));
            }
            return result;
        }
    }

    return result;
}

} // namespace lsp
