//
// Created for hoshi-lang LSP readiness refactoring.
//

#ifndef HOSHI_LANG_DIAGNOSTIC_ENGINE_H
#define HOSHI_LANG_DIAGNOSTIC_ENGINE_H

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "diagnostic.h"

namespace yoi {

class DiagnosticEngine {
public:
    enum class Mode {
        Immediate,  // Error severity throws immediately (current CLI behavior)
        Collect     // Accumulate all diagnostics; throw only on checkAbort()
    };

    DiagnosticEngine();
    explicit DiagnosticEngine(Mode mode);

    // ---- Core reporting ----

    /// Record a diagnostic. In Immediate mode with Error severity,
    /// throws std::runtime_error after recording.
    void report(const Diagnostic &diag);

    /// Convenience overload: constructs a Diagnostic from components.
    /// line and col are 0-based (as received from the lexer/parser).
    /// They are stored as 1-based in the Diagnostic.
    void report(yoi::indexT line, yoi::indexT col,
                const std::string &msg,
                DiagnosticSeverity severity = DiagnosticSeverity::Error,
                DiagnosticCategory category = DiagnosticCategory::Generic);

    // ---- Query API ----

    bool hasErrors() const;
    bool hasWarnings() const;
    size_t errorCount() const;
    size_t warningCount() const;
    const std::vector<Diagnostic> &getDiagnostics() const;
    void clear();

    // ---- Mode control ----

    void setMode(Mode mode);
    Mode getMode() const;

    // ---- File path ----

    /// Set the file path for diagnostics reported via the convenience overload.
    void setCurrentFilePath(const yoi::wstr &path);
    const yoi::wstr &getCurrentFilePath() const;

    // ---- Category severity overrides ----

    /// Map a legacy exception_categories label to a severity override.
    /// Labels: "INTERNAL", "UCRT_NOT_FOUND", "ELYSIA_RUNTIME_NOT_FOUND",
    ///         "NULLABLE_VALUE_SUPPLY_TO_RAW", "MODULE_NOT_MODIFIED"
    void setCategorySeverity(const std::string &label, DiagnosticSeverity severity);

    /// Get the effective severity for a legacy category label.
    DiagnosticSeverity getCategorySeverity(const std::string &label) const;

    // ---- Source line hint ----

    /// Read the source file and produce a line with a caret pointing to the error.
    /// line and col are 1-based.
    yoi::wstr getLineHint(const yoi::wstr &file, yoi::indexT line, yoi::indexT col) const;

private:
    Mode m_mode;
    yoi::wstr m_currentFilePath;
    std::vector<Diagnostic> m_diagnostics;
    mutable std::mutex m_mutex;
    std::map<std::string, DiagnosticSeverity> m_categorySeverityOverrides;

    void initDefaultCategorySeverity();
};

} // namespace yoi

#endif // HOSHI_LANG_DIAGNOSTIC_ENGINE_H
