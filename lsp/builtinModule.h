//
// Embedded builtin module content for LSP.
// Uses the same source file as the compiler (compiler/builtinModule.hoshi).
// The file contains R"( ... )" raw-string markers; when #included after "=",
// the C++ preprocessor treats it as a raw string literal assignment.
// The resulting const char* contains the hoshi source code directly.
//

#ifndef HOSHI_LANG_LSP_BUILTIN_MODULE_H
#define HOSHI_LANG_LSP_BUILTIN_MODULE_H

static const char *__lsp_builtin_module =
#include <compiler/builtinModule.hoshi>
    ;

#endif // HOSHI_LANG_LSP_BUILTIN_MODULE_H
