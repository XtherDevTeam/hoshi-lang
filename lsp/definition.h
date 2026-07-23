//
// Go-to-Definition Provider for hoshi-lang LSP
//

#ifndef HOSHI_LANG_LSP_DEFINITION_H
#define HOSHI_LANG_LSP_DEFINITION_H

#include "protocol.h"
#include "document.h"

namespace lsp {

class DefinitionProvider {
public:
    DefinitionResult provide(Document *doc, const Position &pos, const std::string &uri);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_DEFINITION_H
