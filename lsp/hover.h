//
// Hover Provider for hoshi-lang LSP
//

#ifndef HOSHI_LANG_LSP_HOVER_H
#define HOSHI_LANG_LSP_HOVER_H

#include "protocol.h"
#include "document.h"
#include <optional>

namespace lsp {

class HoverProvider {
public:
    std::optional<Hover> provide(Document *doc, const Position &pos);

private:
    std::optional<Hover> hoverOnSymbol(Document *doc, const Position &pos);
};

} // namespace lsp

#endif // HOSHI_LANG_LSP_HOVER_H
