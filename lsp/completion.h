//
// Completion Provider for hoshi-lang LSP
//

#ifndef HOSHI_LANG_LSP_COMPLETION_H
#define HOSHI_LANG_LSP_COMPLETION_H

#include "protocol.h"
#include "document.h"

namespace lsp {

class CompletionProvider {
public:
    CompletionList provide(Document *doc, const Position &pos);

private:
    std::vector<CompletionItem> keywordCompletions(const std::string &prefix);
    std::vector<CompletionItem> symbolCompletions(Document *doc, const std::string &prefix);
    std::vector<CompletionItem> crossModuleCompletions(Document *doc, const std::string &prefix);
    std::vector<CompletionItem> memberCompletions(Document *doc, const std::string &parent, const std::string &prefix);
    std::vector<CompletionItem> resolveModuleCompletionsFromText(Document *doc, const std::string &parent, const std::string &prefix);
    std::vector<CompletionItem> completionsForType(Document *doc, const yoi::wstr &typeName, const std::string &prefix);
    std::vector<CompletionItem> completionsForTypeNameFromModules(Document *doc, const std::string &typeName, const std::string &prefix);
    CompletionItem symbolToCompletionItem(const Symbol &sym);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_COMPLETION_H
