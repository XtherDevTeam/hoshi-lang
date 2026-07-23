//
// Created for hoshi-lang LSP readiness refactoring.
//

#ifndef HOSHI_LANG_DIAGNOSTIC_H
#define HOSHI_LANG_DIAGNOSTIC_H

#include <cstdint>
#include <string>
#include <share/def.hpp>

namespace yoi {

enum class DiagnosticSeverity {
    Error,
    Warning,
    Note,
    Hint
};

enum class DiagnosticCategory {
    Generic,
    Syntax,
    Semantic,
    Internal,
    UCRTNotFound,
    ElysiaRuntimeNotFound,
    ModuleNotModified,
    NullableValue
};

struct Diagnostic {
    yoi::wstr sourceFile;
    yoi::indexT line;       // 1-based, for display and LSP
    yoi::indexT column;     // 1-based, for display and LSP
    std::string message;
    DiagnosticSeverity severity;
    DiagnosticCategory category;

    Diagnostic() = default;

    Diagnostic(yoi::wstr file, yoi::indexT line, yoi::indexT col,
               std::string msg,
               DiagnosticSeverity sev = DiagnosticSeverity::Error,
               DiagnosticCategory cat = DiagnosticCategory::Generic)
        : sourceFile(std::move(file))
        , line(line)
        , column(col)
        , message(std::move(msg))
        , severity(sev)
        , category(cat) {}
};

} // namespace yoi

#endif // HOSHI_LANG_DIAGNOSTIC_H
