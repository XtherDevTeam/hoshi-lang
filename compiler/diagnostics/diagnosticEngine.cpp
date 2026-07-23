//
// Created for hoshi-lang LSP readiness refactoring.
//

#include "diagnosticEngine.h"
#include <fstream>
#include <share/def.hpp>
#include <stdexcept>

namespace yoi {

DiagnosticEngine::DiagnosticEngine()
    : m_mode(Mode::Immediate) {
    initDefaultCategorySeverity();
}

DiagnosticEngine::DiagnosticEngine(Mode mode)
    : m_mode(mode) {
    initDefaultCategorySeverity();
}

void DiagnosticEngine::initDefaultCategorySeverity() {
    // Map legacy exception_categories labels to diagnostic severities.
    // These match the defaults in share/def.cpp's exception_categories map.
    m_categorySeverityOverrides["INTERNAL"]                    = DiagnosticSeverity::Error;
    m_categorySeverityOverrides["UCRT_NOT_FOUND"]              = DiagnosticSeverity::Warning;
    m_categorySeverityOverrides["ELYSIA_RUNTIME_NOT_FOUND"]    = DiagnosticSeverity::Warning;
    m_categorySeverityOverrides["NULLABLE_VALUE_SUPPLY_TO_RAW"] = DiagnosticSeverity::Warning; // effectively suppressed by caller
    m_categorySeverityOverrides["MODULE_NOT_MODIFIED"]          = DiagnosticSeverity::Warning; // effectively suppressed by caller
}

void DiagnosticEngine::report(const Diagnostic &diag) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_diagnostics.push_back(diag);
    }

    // In Immediate mode, Error severity aborts compilation immediately.
    // This preserves the existing "stop on first error" parser behavior.
    if (m_mode == Mode::Immediate && diag.severity == DiagnosticSeverity::Error) {
        // Format the error message the same way the old panic() did for CLI output.
        auto message = diag.message;
        if (!diag.sourceFile.empty()) {
            message += " near " + yoi::wstring2string(diag.sourceFile)
                    + ":" + std::to_string(diag.line)
                    + ":" + std::to_string(diag.column);
            auto hint = getLineHint(diag.sourceFile, diag.line, diag.column);
            if (!hint.empty()) {
                message += "\n" + yoi::wstring2string(hint);
            }
        }
        throw std::runtime_error(message);
    }
}

void DiagnosticEngine::report(yoi::indexT line, yoi::indexT col,
                               const std::string &msg,
                               DiagnosticSeverity severity,
                               DiagnosticCategory category) {
    // Convert from 0-based (lexer/parser internal) to 1-based (display/LSP).
    Diagnostic diag(m_currentFilePath, line + 1, col + 1, msg, severity, category);
    report(diag);
}

bool DiagnosticEngine::hasErrors() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &d : m_diagnostics) {
        if (d.severity == DiagnosticSeverity::Error)
            return true;
    }
    return false;
}

bool DiagnosticEngine::hasWarnings() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &d : m_diagnostics) {
        if (d.severity == DiagnosticSeverity::Warning)
            return true;
    }
    return false;
}

size_t DiagnosticEngine::errorCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t count = 0;
    for (const auto &d : m_diagnostics) {
        if (d.severity == DiagnosticSeverity::Error)
            ++count;
    }
    return count;
}

size_t DiagnosticEngine::warningCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t count = 0;
    for (const auto &d : m_diagnostics) {
        if (d.severity == DiagnosticSeverity::Warning)
            ++count;
    }
    return count;
}

const std::vector<Diagnostic> &DiagnosticEngine::getDiagnostics() const {
    // This returns a const reference. Callers must not hold it across
    // concurrent report() calls, or they must do their own locking.
    return m_diagnostics;
}

void DiagnosticEngine::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_diagnostics.clear();
}

void DiagnosticEngine::setMode(Mode mode) {
    m_mode = mode;
}

DiagnosticEngine::Mode DiagnosticEngine::getMode() const {
    return m_mode;
}

void DiagnosticEngine::setCurrentFilePath(const yoi::wstr &path) {
    m_currentFilePath = path;
}

const yoi::wstr &DiagnosticEngine::getCurrentFilePath() const {
    return m_currentFilePath;
}

void DiagnosticEngine::setCategorySeverity(const std::string &label, DiagnosticSeverity severity) {
    m_categorySeverityOverrides[label] = severity;
}

DiagnosticSeverity DiagnosticEngine::getCategorySeverity(const std::string &label) const {
    auto it = m_categorySeverityOverrides.find(label);
    if (it != m_categorySeverityOverrides.end()) {
        return it->second;
    }
    // Default fallback: for unknown categories, Warning severity.
    return DiagnosticSeverity::Warning;
}

yoi::wstr DiagnosticEngine::getLineHint(const yoi::wstr &file, yoi::indexT line, yoi::indexT col) const {
    // This mirrors the original get_line_hint_for_error() in share/def.cpp.
    std::fstream fileStream(yoi::wstring2string(file), std::ios::in);
    if (!fileStream.is_open()) {
        return L"";
    }
    std::string lineStr;
    for (yoi::indexT i = 0; i < line; i++) {
        if (!std::getline(fileStream, lineStr)) {
            return L"";
        }
    }
    yoi::wstr result = yoi::string2wstring(lineStr) + L"\n";
    // col is 1-based; place the caret at the right column
    if (col > 0) {
        result += std::wstring(col - 1, L' ') + L"^";
    }
    return result;
}

} // namespace yoi
