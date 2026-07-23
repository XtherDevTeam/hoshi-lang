//
// Completion Provider implementation
//

#include "completion.h"
#include <algorithm>
#include <compiler/ir/IR.h>
#include <filesystem>
#include <set>
#include <share/def.hpp>

namespace lsp {

namespace fs = std::filesystem;

static std::string documentFilePath(const Document *doc) {
    if (!doc) return "";
    std::string path = doc->uri;
    if (path.rfind("file://", 0) == 0) path = path.substr(7);
    return path;
}

static std::vector<fs::path> moduleSearchRoots(const Document *doc) {
    std::vector<fs::path> roots;
    const std::string documentPath = documentFilePath(doc);
    if (!documentPath.empty()) {
        const fs::path parent = fs::path(documentPath).parent_path();
        roots.push_back(parent);
        roots.push_back(parent / ".tsuki_modules");
    }
    if (doc && doc->projectIndex) {
        for (const auto &searchPath : doc->projectIndex->getSearchPaths()) {
            if (!searchPath.empty()) roots.emplace_back(searchPath);
        }
    }
    return roots;
}

// Look up a function's return type from the visitor's IR module.
static std::string getReturnTypeFromIR(Document *doc, const std::string &funcName) {
    if (!doc || !doc->irModule) return "";
    yoi::wstr wName = yoi::string2wstring(funcName);
    // Check function overloads first
    if (doc->irModule->functionOverloadIndexies.contains(wName)) {
        auto &overloads = doc->irModule->functionOverloadIndexies[wName];
        if (!overloads.empty()) {
            auto idx = overloads[0];
            auto &funcDef = *doc->irModule->functionTable[idx];
            // funcDef has returnType info; parse from the name string
            std::string detail = yoi::wstring2string(funcDef.name);
            size_t colon = detail.rfind(':');
            if (colon != std::string::npos) {
                std::string retType = detail.substr(colon + 1);
                while (!retType.empty() && std::isspace(retType.front())) retType.erase(0, 1);
                size_t tmpl = retType.find('<');
                if (tmpl != std::string::npos) retType = retType.substr(0, tmpl);
                size_t dot = retType.rfind('.');
                if (dot != std::string::npos) retType = retType.substr(dot + 1);
                return retType;
            }
        }
    }
    return "";
}

CompletionList CompletionProvider::provide(Document *doc, const Position &pos) {
    CompletionList result;
    result.isIncomplete = false;

    if (!doc) return result;

    int charPos = static_cast<int>(pos.character);
    int lineNum = static_cast<int>(pos.line);

    // Find the line text
    std::string lineText;
    {
        int lineIdx = 0;
        size_t start = 0;
        std::string text = yoi::wstring2string(doc->text);
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '\n') {
                if (lineIdx == lineNum) {
                    lineText = text.substr(start, i - start);
                    break;
                }
                lineIdx++;
                start = i + 1;
            }
        }
        if (lineText.empty() && lineIdx == lineNum) {
            lineText = text.substr(start);
        }
    }

    // Get the prefix (word before cursor)
    std::string prefix;
    if (charPos > 0 && charPos <= static_cast<int>(lineText.size())) {
        std::string before = lineText.substr(0, charPos);
        size_t wordStart = before.size();
        while (wordStart > 0 && (std::isalnum(before[wordStart - 1]) || before[wordStart - 1] == '_'))
            wordStart--;
        prefix = before.substr(wordStart);
    }

    // Detect dot-access: cursor after a dot or after an identifier preceded by a dot
    bool afterDot = false;
    std::string dotPrefix; // The identifier before the dot (module alias / struct name)
    if (charPos > 0 && charPos <= static_cast<int>(lineText.size())) {
        std::string before = lineText.substr(0, charPos);
        while (!before.empty() && std::isspace(before.back()))
            before.pop_back();
        // Check if the character immediately before the cursor is a dot,
        // or if there's a dot before the current word
        if (!before.empty()) {
            // Find the last dot position
            size_t lastDot = before.rfind('.');
            if (lastDot != std::string::npos) {
                // Check if cursor is right after the dot (no word yet)
                if (lastDot == before.size() - 1) {
                    afterDot = true;
                    // Extract the identifier before the dot
                    std::string beforeDot = before.substr(0, lastDot);
                    // Get the last identifier before the dot
                    size_t idEnd = beforeDot.size();
                    size_t idStart = idEnd;
                    while (idStart > 0 && (std::isalnum(beforeDot[idStart - 1]) || beforeDot[idStart - 1] == '_'))
                        idStart--;
                    if (idStart < idEnd) {
                        dotPrefix = beforeDot.substr(idStart, idEnd - idStart);
                    }
                } else {
                    // There's a word after the dot — check if it's part of the current identifier
                    std::string afterDotStr = before.substr(lastDot + 1);
                    // If prefix starts at or after lastDot+1, we're doing member completion
                    if (prefix.rfind(afterDotStr, 0) == 0 || afterDotStr.rfind(prefix, 0) == 0) {
                        afterDot = true;
                        prefix = afterDotStr; // Use the part after dot as prefix
                        std::string beforeDot = before.substr(0, lastDot);
                        size_t idEnd = beforeDot.size();
                        size_t idStart = idEnd;
                        while (idStart > 0 && (std::isalnum(beforeDot[idStart - 1]) || beforeDot[idStart - 1] == '_'))
                            idStart--;
                        if (idStart < idEnd) {
                            dotPrefix = beforeDot.substr(idStart, idEnd - idStart);
                        }
                    }
                }
            }
        }
    }

    // Gather completions
    const std::string beforeCursor = charPos >= 0 && charPos <= static_cast<int>(lineText.size())
        ? lineText.substr(0, charPos) : lineText;
    const size_t firstNonSpace = beforeCursor.find_first_not_of(" \t");
    const std::string useText = firstNonSpace == std::string::npos ? "" : beforeCursor.substr(firstNonSpace);
    if (useText.rfind("use", 0) == 0 &&
        (useText.size() == 3 || std::isspace(static_cast<unsigned char>(useText[3])))) {
        const std::string useTail = useText.substr(3);
        const size_t quote = useTail.find('"');
        if (quote != std::string::npos && useTail.find('"', quote + 1) == std::string::npos) {
            result.items = usePathCompletions(doc, useTail.substr(quote + 1));
        } else if (quote == std::string::npos) {
            result.items = useModuleCompletions(doc, prefix);
        }
        return result;
    } else if (!doc->parseSucceeded && afterDot && !dotPrefix.empty()) {
        // Parse failed but we have a dot-prefix — try to resolve the module
        // from the raw text. This handles incomplete code like "str." where
        // the parser throws on the trailing dot.
        result.items = resolveModuleCompletionsFromText(doc, dotPrefix, prefix);
        // Fall back to keywords if module resolution failed
        if (result.items.empty()) {
            result.items = keywordCompletions(prefix);
        }
        return result;
    } else if (!doc->parseSucceeded) {
        // Parse failed — return keywords as a fallback
        result.items = keywordCompletions(prefix);
        return result;
    } else if (afterDot && !dotPrefix.empty()) {
        // Member access — try to match dotPrefix to a module alias or struct
        result.items = memberCompletions(doc, dotPrefix, prefix, lineNum);
        // If member completions returned nothing, fall back to global completions
        if (result.items.empty()) {
            result.items = symbolCompletions(doc, prefix, lineNum);
            auto crossItems = crossModuleCompletions(doc, prefix);
            result.items.insert(result.items.end(), crossItems.begin(), crossItems.end());
            auto kwCompletions = keywordCompletions(prefix);
            result.items.insert(result.items.end(), kwCompletions.begin(), kwCompletions.end());
        }
    } else if (afterDot) {
        // Dot without a clear prefix (e.g., at start of line, or after a number)
        // Fall through to global completions instead of returning empty
        result.items = symbolCompletions(doc, prefix, lineNum);
        auto crossItems = crossModuleCompletions(doc, prefix);
        result.items.insert(result.items.end(), crossItems.begin(), crossItems.end());
        auto kwCompletions = keywordCompletions(prefix);
        result.items.insert(result.items.end(), kwCompletions.begin(), kwCompletions.end());
    } else {
        // Symbol + keyword completions (global scope)
        result.items = symbolCompletions(doc, prefix, lineNum);
        auto crossItems = crossModuleCompletions(doc, prefix);
        result.items.insert(result.items.end(), crossItems.begin(), crossItems.end());
        auto kwCompletions = keywordCompletions(prefix);
        result.items.insert(result.items.end(), kwCompletions.begin(), kwCompletions.end());
    }

    // Sort alphabetically
    std::sort(result.items.begin(), result.items.end(),
              [](const CompletionItem &a, const CompletionItem &b) {
                  return a.sortText.value_or(a.label) < b.sortText.value_or(b.label);
              });

    return result;
}

std::vector<CompletionItem> CompletionProvider::memberCompletions(Document *doc, const std::string &parent, const std::string &prefix, int cursorLine) {
    std::vector<CompletionItem> result;
    yoi::wstr wParent = yoi::string2wstring(parent);

    // Find all symbols matching parent name
    std::vector<const Symbol *> matching;
    for (auto &sym : doc->symbols) {
        if (cursorLine >= 0 &&
            !isSymbolVisibleAt(doc->symbols, sym, static_cast<yoi::indexT>(cursorLine))) {
            continue;
        }
        if (sym.name == wParent) matching.push_back(&sym);
    }
    // If multiple matches, pick the one closest to cursorLine (innermost scope)
    const Symbol *bestMatch = nullptr;
    if (!matching.empty()) {
        bestMatch = matching[0];
        if (cursorLine >= 0 && matching.size() > 1) {
            for (auto *m : matching) {
                if (static_cast<int>(m->line) <= cursorLine &&
                    static_cast<int>(m->line) > static_cast<int>(bestMatch->line)) {
                    bestMatch = m;
                }
            }
        }
    }

    if (bestMatch) {
        const auto &sym = *bestMatch;
        if (sym.name == wParent) {
            // Case 1: struct/interface — offer its children
            if (!sym.children.empty()) {
                for (auto &child : sym.children) {
                    std::string name = yoi::wstring2string(child.name);
                    if (name.rfind(prefix, 0) != 0) continue;
                    CompletionItem item = symbolToCompletionItem(child);
                    item.label = name;
                    item.insertText = name;
                    result.push_back(item);
                }
                return result; // Found struct/interface with children
            }
            // Case 2: typed variable — look up its type and offer methods
            if (sym.kind == HoshiSymbolKind::Variable && !sym.typeInfo.empty()) {
                result = completionsForType(doc, sym.typeInfo, prefix);
                if (!result.empty()) return result;
            }
            // If it's a ModuleAlias or unresolved, fall through to cross-module search
        }
    }

    // Check cross-module symbols: find a ModuleAlias with name == parent
    for (auto &sym : doc->symbols) {
        if (sym.kind == HoshiSymbolKind::ModuleAlias && sym.name == wParent) {
            // This is a module alias — offer all symbols from the resolved module
            for (auto &cross : doc->crossModuleSymbols) {
                std::string name = yoi::wstring2string(cross.name);
                if (name.rfind(prefix, 0) != 0) continue;
                CompletionItem item = symbolToCompletionItem(cross);
                item.label = name;
                item.insertText = name;
                item.documentation = std::string("From module ") + yoi::wstring2string(sym.importPath);
                result.push_back(item);
            }
            return result;
        }
    }

    // Check cross-module symbols: find a struct/interface by name and offer children
    for (auto &cross : doc->crossModuleSymbols) {
        if (cross.name == wParent && !cross.children.empty() &&
            (cross.kind == HoshiSymbolKind::Struct || cross.kind == HoshiSymbolKind::Interface)) {
            for (auto &child : cross.children) {
                std::string name = yoi::wstring2string(child.name);
                if (name.rfind(prefix, 0) != 0) continue;
                CompletionItem item = symbolToCompletionItem(child);
                item.label = name;
                item.insertText = name;
                item.documentation = std::string("From ") + yoi::wstring2string(cross.sourceFile);
                result.push_back(item);
            }
            if (!result.empty()) return result;
        }
    }

    // Also check cross-module symbols directly by parentName (for methods/fields)
    for (auto &cross : doc->crossModuleSymbols) {
        if (cross.parentName == wParent) {
            std::string name = yoi::wstring2string(cross.name);
            if (name.rfind(prefix, 0) != 0) continue;
            CompletionItem item = symbolToCompletionItem(cross);
            item.label = name;
            item.insertText = name;
            result.push_back(item);
        }
    }

    return result;
}

std::vector<CompletionItem> CompletionProvider::crossModuleCompletions(Document *doc, const std::string &prefix) {
    std::vector<CompletionItem> result;

    if (!doc) return result;

    for (auto &sym : doc->crossModuleSymbols) {
        if (sym.isLocal) continue;
        std::string name = yoi::wstring2string(sym.name);
        if (name.rfind(prefix, 0) != 0) continue;

        CompletionItem item = symbolToCompletionItem(sym);
        item.label = name;
        item.insertText = name;
        item.sortText = name;
        if (!sym.sourceFile.empty()) {
            item.documentation = std::string("From ") + yoi::wstring2string(sym.sourceFile);
        }
        result.push_back(item);
    }

    return result;
}

CompletionItem CompletionProvider::symbolToCompletionItem(const Symbol &sym) {
    CompletionItem item;
    item.label = yoi::wstring2string(sym.name);
    item.insertText = yoi::wstring2string(sym.name);
    item.sortText = yoi::wstring2string(sym.name);
    item.detail = yoi::wstring2string(sym.detail);

    switch (sym.kind) {
        case HoshiSymbolKind::Function:    item.kind = CompletionItemKind::Function; break;
        case HoshiSymbolKind::Struct:      item.kind = CompletionItemKind::Struct; break;
        case HoshiSymbolKind::Interface:   item.kind = CompletionItemKind::Interface; break;
        case HoshiSymbolKind::Variable:    item.kind = CompletionItemKind::Variable; break;
        case HoshiSymbolKind::TypeAlias:   item.kind = CompletionItemKind::TypeParameter; break;
        case HoshiSymbolKind::Enum:        item.kind = CompletionItemKind::Enum; break;
        case HoshiSymbolKind::Module:
        case HoshiSymbolKind::ModuleAlias:  item.kind = CompletionItemKind::Module; break;
        case HoshiSymbolKind::Method:      item.kind = CompletionItemKind::Method; break;
        case HoshiSymbolKind::Field:       item.kind = CompletionItemKind::Field; break;
        case HoshiSymbolKind::Constructor: item.kind = CompletionItemKind::Constructor; break;
        case HoshiSymbolKind::Finalizer:   item.kind = CompletionItemKind::Method; break;
        case HoshiSymbolKind::EnumMember:  item.kind = CompletionItemKind::EnumMember; break;
        case HoshiSymbolKind::Import:
        case HoshiSymbolKind::Export:      item.kind = CompletionItemKind::Reference; break;
        case HoshiSymbolKind::Concept_:    item.kind = CompletionItemKind::Interface; break;
        default:                           item.kind = CompletionItemKind::Text; break;
    }

    if (!sym.typeInfo.empty())
        item.documentation = "Type: " + yoi::wstring2string(sym.typeInfo);

    return item;
}

std::vector<CompletionItem> CompletionProvider::keywordCompletions(const std::string &prefix) {
    static const std::vector<std::pair<const char *, const char *>> keywords = {
        {"func", "Function definition"},
        {"struct", "Struct definition"},
        {"interface", "Interface definition"},
        {"impl", "Implementation block"},
        {"let", "Variable declaration"},
        {"if", "If statement"},
        {"elif", "Else-if branch"},
        {"else", "Else branch"},
        {"while", "While loop"},
        {"for", "For loop"},
        {"forEach", "For-each loop"},
        {"return", "Return statement"},
        {"break", "Break statement"},
        {"continue", "Continue statement"},
        {"import", "Import declaration"},
        {"export", "Export declaration"},
        {"use", "Use (module alias) statement"},
        {"alias", "Type alias"},
        {"enum", "Enumeration definition"},
        {"datastruct", "Data struct definition"},
        {"new", "New expression"},
        {"try", "Try block"},
        {"catch", "Catch block"},
        {"finally", "Finally block"},
        {"throw", "Throw statement"},
        {"type_id", "Type ID expression"},
        {"dyn_cast", "Dynamic cast expression"},
        {"yield", "Yield statement"},
        {"concept", "Concept definition"},
        {"constructor", "Constructor"},
        {"finalizer", "Finalizer"},
        {"true", "Boolean true literal"},
        {"false", "Boolean false literal"},
        {"null", "Null literal"},
        {"as", "Type cast / export alias"},
        {"from", "Import source specifier"},
        {"in", "Membership check"},
        {"no_ffi", "Function attribute: no FFI"},
        {"static", "Function attribute: static"},
        {"intrinsic", "Function attribute: intrinsic"},
        {"generator", "Function attribute: generator"},
        {"always_inline", "Function attribute: always inline"},
        {"datafield", "Struct field modifier"},
        {"callable", "Callable interface"},
        {"weak", "Weak modifier"},
        {"satisfy", "Concept satisfy clause"},
    };

    std::vector<CompletionItem> result;
    for (auto &[kw, desc] : keywords) {
        if (prefix.empty() || std::string(kw).rfind(prefix, 0) == 0) {
            CompletionItem item;
            item.label = kw;
            item.kind = CompletionItemKind::Keyword;
            item.detail = desc;
            item.insertText = kw;
            item.sortText = kw;
            result.push_back(item);
        }
    }
    return result;
}

std::vector<CompletionItem> CompletionProvider::useModuleCompletions(
        Document *doc, const std::string &prefix) {
    std::vector<CompletionItem> result;
    std::set<std::string> moduleNames = {"builtin"};

    for (const auto &root : moduleSearchRoots(doc)) {
        std::error_code ec;
        if (!fs::is_directory(root, ec)) continue;
        for (const auto &entry : fs::directory_iterator(root, ec)) {
            if (ec) break;
            const fs::path path = entry.path();
            if (entry.is_regular_file(ec) && path.extension() == ".hoshi") {
                moduleNames.insert(path.stem().string());
            } else if (entry.is_directory(ec) && fs::is_regular_file(path / "index.hoshi", ec)) {
                moduleNames.insert(path.filename().string());
            }
        }
    }

    for (const auto &name : moduleNames) {
        if (!prefix.empty() && name.rfind(prefix, 0) != 0) continue;
        CompletionItem item;
        item.label = name;
        item.kind = CompletionItemKind::Module;
        item.detail = "use " + name + " \"" + name + "\"";
        item.insertText = name + " \"" + name + "\"";
        item.sortText = name;
        result.push_back(std::move(item));
    }
    return result;
}

std::vector<CompletionItem> CompletionProvider::usePathCompletions(
        Document *doc, const std::string &pathPrefix) {
    std::vector<CompletionItem> result;
    const size_t slash = pathPrefix.find_last_of("/\\");
    const std::string directoryPart = slash == std::string::npos ? "" : pathPrefix.substr(0, slash + 1);
    const std::string entryPrefix = slash == std::string::npos ? pathPrefix : pathPrefix.substr(slash + 1);
    std::set<std::string> entries;

    for (const auto &root : moduleSearchRoots(doc)) {
        const fs::path directory = root / fs::path(directoryPart);
        std::error_code ec;
        if (!fs::is_directory(directory, ec)) continue;
        for (const auto &entry : fs::directory_iterator(directory, ec)) {
            if (ec) break;
            const std::string name = entry.path().filename().string();
            if (name.empty() || name[0] == '.' ||
                (!entryPrefix.empty() && name.rfind(entryPrefix, 0) != 0)) {
                continue;
            }
            if (entry.is_directory(ec)) {
                entries.insert(name + "/");
            } else if (entry.is_regular_file(ec) && entry.path().extension() == ".hoshi") {
                entries.insert(name);
            }
        }
    }

    for (const auto &entry : entries) {
        CompletionItem item;
        item.label = directoryPart + entry;
        item.kind = entry.back() == '/' ? CompletionItemKind::Folder : CompletionItemKind::File;
        item.detail = entry.back() == '/' ? "directory" : "Hoshi module";
        item.insertText = entry;
        item.sortText = entry;
        result.push_back(std::move(item));
    }
    return result;
}

std::vector<CompletionItem> CompletionProvider::symbolCompletions(Document *doc,
                                                                    const std::string &prefix,
                                                                    int cursorLine) {
    std::vector<CompletionItem> result;

    if (!doc || !doc->parseSucceeded) return result;

    for (auto &sym : doc->symbols) {
        if (cursorLine >= 0 &&
            !isSymbolVisibleAt(doc->symbols, sym, static_cast<yoi::indexT>(cursorLine))) {
            continue;
        }
        std::string name = yoi::wstring2string(sym.name);
        if (name.rfind(prefix, 0) != 0) continue;

        CompletionItem item = symbolToCompletionItem(sym);
        item.label = name;
        item.insertText = name;
        item.sortText = name;
        result.push_back(item);
    }

    return result;
}

std::vector<CompletionItem> CompletionProvider::resolveModuleCompletionsFromText(
        Document *doc, const std::string &parent, const std::string &prefix) {
    std::vector<CompletionItem> result;
    if (!doc || !doc->projectIndex) return result;

    // Search the raw text for "use <parent> \"...\"" or "import ... from \"...\"" patterns
    std::string text = yoi::wstring2string(doc->text);
    std::string importPath;

    // Pattern 1: use <parent> "<path>"
    std::string usePattern = "use " + parent + " \"";
    size_t pos = text.find(usePattern);
    if (pos != std::string::npos) {
        size_t pathStart = pos + usePattern.size();
        size_t pathEnd = text.find('"', pathStart);
        if (pathEnd != std::string::npos) {
            importPath = text.substr(pathStart, pathEnd - pathStart);
        }
    }

    // Pattern 2: import ... from "<path>" (any import whose path might contain the module)
    if (importPath.empty()) {
        std::string importKw = "import ";
        size_t ipos = 0;
        while ((ipos = text.find(importKw, ipos)) != std::string::npos) {
            size_t fromPos = text.find("from \"", ipos);
            if (fromPos != std::string::npos) {
                size_t pathStart = fromPos + 6;  // strlen("from \"")
                size_t pathEnd = text.find('"', pathStart);
                if (pathEnd != std::string::npos) {
                    std::string maybePath = text.substr(pathStart, pathEnd - pathStart);
                    // Check if the module path ends with the parent name
                    if (maybePath == parent ||
                        maybePath.rfind("/" + parent) == maybePath.size() - parent.size() - 1 ||
                        maybePath.rfind("\\" + parent) == maybePath.size() - parent.size() - 1) {
                        importPath = maybePath;
                        break;
                    }
                }
            }
            ipos = fromPos != std::string::npos ? fromPos + 1 : ipos + 1;
        }
    }

    if (importPath.empty()) {
        // Not a module alias — try to find the variable's type from its declaration.
        // Look for "let <parent> = <Module>.<Type>(...)" or "let <parent>: <Type>"
        std::string letPattern = "let " + parent + " = ";
        size_t letPos = text.find(letPattern);
        if (letPos == std::string::npos) {
            letPattern = "let " + parent + ": ";
            letPos = text.find(letPattern);
        }
        if (letPos != std::string::npos) {
            std::string afterLet = text.substr(letPos + letPattern.size());
            // Extract the type: for "str.Str(" → module="str", type="Str"
            size_t dotPos = afterLet.find('.');
            std::string typeName, moduleName;
            if (dotPos != std::string::npos && dotPos < afterLet.find('(')) {
                // Module.Type pattern — extract module and type
                moduleName = afterLet.substr(0, dotPos);
                size_t typeStart = dotPos + 1;
                size_t typeEnd = typeStart;
                while (typeEnd < afterLet.size() && (std::isalnum(afterLet[typeEnd]) || afterLet[typeEnd] == '_'))
                    typeEnd++;
                typeName = afterLet.substr(typeStart, typeEnd - typeStart);
            } else {
                size_t typeEnd = 0;
                while (typeEnd < afterLet.size() && (std::isalnum(afterLet[typeEnd]) || afterLet[typeEnd] == '_'))
                    typeEnd++;
                typeName = afterLet.substr(0, typeEnd);
            }
            if (!typeName.empty() && doc->projectIndex) {
                // Index the module and resolve the type
                if (!moduleName.empty()) {
                    std::string fp = doc->uri;
                    if (fp.rfind("file://", 0) == 0) fp = fp.substr(7);
                    std::string useP = "use " + moduleName + " \"";
                    size_t upos = text.find(useP);
                    if (upos != std::string::npos) {
                        size_t ps = upos + useP.size();
                        size_t pe = text.find('"', ps);
                        if (pe != std::string::npos) {
                            std::string resolvedPath = doc->projectIndex->resolveModule(
                                text.substr(ps, pe - ps), fp);
                            if (!resolvedPath.empty()) {
                                doc->projectIndex->indexAndGet(resolvedPath);
                            }
                        }
                    }
                }

                // If lowercase (function call), look up the function's return type
                std::string lookupType = typeName;
                if (std::islower(typeName[0]) && !moduleName.empty()) {
                    // First try the visitor's IR module for precise return type
                    std::string irRetType = getReturnTypeFromIR(doc, typeName);
                    if (!irRetType.empty()) {
                        lookupType = irRetType;
                    } else {
                        // Fall back to symbol-based lookup
                        yoi::wstr wFunc = yoi::string2wstring(typeName);
                        yoi::vec<Symbol> allSyms;
                        doc->projectIndex->getAllSymbols(allSyms);
                        for (auto &sym : allSyms) {
                            if (sym.name == wFunc && sym.kind == HoshiSymbolKind::Function && !sym.typeInfo.empty()) {
                                std::string retType = yoi::wstring2string(sym.typeInfo);
                                size_t lastDot = retType.rfind('.');
                                if (lastDot != std::string::npos) retType = retType.substr(lastDot + 1);
                                size_t tmpl = retType.find('<');
                                if (tmpl != std::string::npos) retType = retType.substr(0, tmpl);
                                while (!retType.empty() && std::isspace(retType.back())) retType.pop_back();
                                if (!retType.empty()) { lookupType = retType; break; }
                            }
                        }
                    }
                }

                // Look up the type name for member completion
                if (std::isupper(lookupType[0])) {
                    auto completions = completionsForTypeNameFromModules(doc, lookupType, prefix);
                    if (!completions.empty()) return completions;
                }
            }
        }
        return result;
    }

    // Resolve the module path relative to the document's directory
    std::string filePath = doc->uri;
    if (filePath.rfind("file://", 0) == 0) {
        filePath = filePath.substr(7);
    }
    std::string resolvedPath = doc->projectIndex->resolveModule(importPath, filePath);
    if (resolvedPath.empty()) return result;

    // Get symbols from the resolved module
    const auto &modSymbols = doc->projectIndex->indexAndGet(resolvedPath);

    // Build completion items
    for (const auto &sym : modSymbols) {
        if (sym.isLocal) continue;
        std::string name = yoi::wstring2string(sym.name);
        if (!prefix.empty() && name.rfind(prefix, 0) != 0) continue;

        CompletionItem item = symbolToCompletionItem(sym);
        item.label = name;
        item.insertText = name;
        item.sortText = name;
        item.documentation = std::string("From ") + resolvedPath;
        result.push_back(item);
    }

    return result;
}

std::vector<CompletionItem> CompletionProvider::completionsForType(
        Document *doc, const yoi::wstr &typeName, const std::string &prefix) {
    std::vector<CompletionItem> result;
    bool found = false;

    // Search local symbols for a struct/interface with this name
    for (auto &sym : doc->symbols) {
        if (sym.name == typeName &&
            (sym.kind == HoshiSymbolKind::Struct || sym.kind == HoshiSymbolKind::Interface)) {
            found = true;
            // Include children (declared inside the struct)
            for (auto &child : sym.children) {
                std::string name = yoi::wstring2string(child.name);
                if (name.rfind(prefix, 0) != 0) continue;
                CompletionItem item = symbolToCompletionItem(child);
                item.label = name;
                item.insertText = name;
                item.documentation = std::string("From ") + yoi::wstring2string(sym.name);
                result.push_back(item);
            }
        }
        // Also include symbols whose parentName matches (methods from impl blocks)
        if (sym.parentName == typeName &&
            (sym.kind == HoshiSymbolKind::Method || sym.kind == HoshiSymbolKind::Field ||
             sym.kind == HoshiSymbolKind::Constructor || sym.kind == HoshiSymbolKind::Finalizer)) {
            std::string name = yoi::wstring2string(sym.name);
            if (name.rfind(prefix, 0) != 0) continue;
            CompletionItem item = symbolToCompletionItem(sym);
            item.label = name;
            item.insertText = name;
            item.documentation = std::string("From ") + yoi::wstring2string(typeName);
            result.push_back(item);
        }
    }
    if (found && !result.empty()) return result;

    // Search cross-module symbols
    yoi::vec<Symbol> allCross;
    for (auto &cross : doc->crossModuleSymbols) allCross.push_back(cross);
    // Also get all from ProjectIndex
    if (doc->projectIndex) {
        doc->projectIndex->getAllSymbols(allCross);
    }

    for (auto &cross : allCross) {
        if (cross.name == typeName &&
            (cross.kind == HoshiSymbolKind::Struct || cross.kind == HoshiSymbolKind::Interface)) {
            found = true;
            for (auto &child : cross.children) {
                std::string name = yoi::wstring2string(child.name);
                if (name.rfind(prefix, 0) != 0) continue;
                CompletionItem item = symbolToCompletionItem(child);
                item.label = name;
                item.insertText = name;
                item.documentation = std::string("From ") + yoi::wstring2string(typeName);
                result.push_back(item);
            }
        }
        if (cross.parentName == typeName &&
            (cross.kind == HoshiSymbolKind::Method || cross.kind == HoshiSymbolKind::Field ||
             cross.kind == HoshiSymbolKind::Constructor || cross.kind == HoshiSymbolKind::Finalizer)) {
            std::string name = yoi::wstring2string(cross.name);
            if (name.rfind(prefix, 0) != 0) continue;
            CompletionItem item = symbolToCompletionItem(cross);
            item.label = name;
            item.insertText = name;
            item.documentation = std::string("From ") + yoi::wstring2string(typeName);
            result.push_back(item);
        }
    }

    return result;
}

std::vector<CompletionItem> CompletionProvider::completionsForTypeNameFromModules(
        Document *doc, const std::string &typeName, const std::string &prefix) {
    std::vector<CompletionItem> result;
    if (!doc || !doc->projectIndex) return result;
    yoi::wstr wTypeName = yoi::string2wstring(typeName);

    // Always ensure builtin is indexed (contains core types like Result, TypeInfo, etc.)
    doc->projectIndex->indexAndGet("builtin");

    // Check cross-module symbols already loaded
    for (auto &cross : doc->crossModuleSymbols) {
        if (cross.name == wTypeName && !cross.children.empty() &&
            (cross.kind == HoshiSymbolKind::Struct || cross.kind == HoshiSymbolKind::Interface)) {
            for (auto &child : cross.children) {
                std::string name = yoi::wstring2string(child.name);
                if (name.rfind(prefix, 0) != 0) continue;
                CompletionItem item = symbolToCompletionItem(child);
                item.label = name;
                item.insertText = name;
                item.documentation = std::string("From ") + typeName;
                result.push_back(item);
            }
            return result;
        }
    }

    // Also check all loaded modules in the ProjectIndex
    yoi::vec<Symbol> allSymbols;
    doc->projectIndex->getAllSymbols(allSymbols);
    for (auto &sym : allSymbols) {
        if (sym.name == wTypeName && !sym.children.empty() &&
            (sym.kind == HoshiSymbolKind::Struct || sym.kind == HoshiSymbolKind::Interface)) {
            for (auto &child : sym.children) {
                std::string name = yoi::wstring2string(child.name);
                if (name.rfind(prefix, 0) != 0) continue;
                CompletionItem item = symbolToCompletionItem(child);
                item.label = name;
                item.insertText = name;
                item.documentation = std::string("From ") + typeName;
                result.push_back(item);
            }
            return result;
        }
    }

    return result;
}

} // namespace lsp
