//
// Created by XIaokang00010 on 2025/7/31.
//

#include "ccObjectLinker.h"
#include "compiler/ir/IR.h"
#include <sstream>
#include <filesystem>

namespace yoi {

    ccObjectLinker::ccObjectLinker(const yoi::wstr &objectPath, const std::shared_ptr<IRBuildConfig> &config)
        : ObjectLinker(objectPath, config) {
        this->setLinkerPath(L"");
    }

    bool ccObjectLinker::commandExists(const std::string& commandName) {
        const char* path_env = std::getenv("PATH");
        if (!path_env) {
            return false; // PATH not set
        }

        std::string path_str(path_env);
        std::istringstream iss(path_str);
        std::string path_segment;

#ifdef _WIN32
        const char path_delimiter = ';';
        const std::string executable_extension = ".exe";
#else
        const char path_delimiter = ':';
        const std::string executable_extension; // Linux/macOS executables typically have no extension
#endif

        // Iterate through each segment in PATH
        while (std::getline(iss, path_segment, path_delimiter)) {
            std::filesystem::path full_path = std::filesystem::path(path_segment) / (commandName + executable_extension);

            // Check if the file exists and is a regular file (i.e., not a directory)
            if (std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path)) {
                // On non-Windows, also check for executable permissions
#ifndef _WIN32
                // This part requires <unistd.h> which std::filesystem doesn't directly provide for execute bit.
                // However, if std::filesystem::status() reports it as a regular file and `std::system` later finds it,
                // it's usually executable. For a stricter check, one might still use access(full_path.c_str(), X_OK).
                // For this context, assuming if it exists and is a file, it's usable by system().
#endif
                return true;
            }
        }
        return false;
    }


    ObjectLinker &ccObjectLinker::searchAndSetupLinker() {
        if (commandExists("cc")) {
            setLinkerPath(L"cc");
        } else if (commandExists("gcc")) {
            setLinkerPath(L"gcc");
        } else if (commandExists("clang")) {
            setLinkerPath(L"clang");
        }
        else {
            throw std::runtime_error("ccObjectLinker: 'cc', 'gcc', or 'clang' not found in system PATH. Cannot setup linker.");
        }
        return *this;
    }

    ObjectLinker &ccObjectLinker::link(const yoi::wstr &outputPath) {
        if (this->getLinkerPath().empty()) {
            throw std::runtime_error("ccObjectLinker: Linker path not set. Call searchAndSetupLinker() first.");
        }
        if (this->getObjectPath().empty()) {
            throw std::runtime_error("ccObjectLinker: Object file path not set.");
        }

        std::string command = yoi::wstring2string(this->getLinkerPath());
        command += " ";
        command += yoi::wstring2string(this->getObjectPath());
        command += " -o ";
        command += yoi::wstring2string(outputPath);

        // Add Elysia runtime path and library
        if (!this->getElysiaRuntimePath().empty()) {
            // Ensure the elysiaRuntimePath exists as a directory
            std::filesystem::path elysia_path_fs(this->getElysiaRuntimePath());
            if (!std::filesystem::exists(elysia_path_fs) || !std::filesystem::is_directory(elysia_path_fs)) {
                std::string error_msg = "ccObjectLinker: Elysia runtime path does not exist or is not a directory: " +
                                        yoi::wstring2string(this->getElysiaRuntimePath());
                throw std::runtime_error(error_msg);
            }

            command += " -L"; // Add library search path
            command += yoi::wstring2string(this->getElysiaRuntimePath());
            command += " -lelysia_runtime";
        }

        if (this->getConfig()->buildType == IRBuildConfig::BuildType::library) {
            command += " -shared"; // Build a shared library
        }

        int result = std::system(command.c_str());
        if (result != 0) {
            std::string error_msg = "ccObjectLinker: Linker command failed with exit code " + std::to_string(result) + ". Command: " + command;
            throw std::runtime_error(error_msg);
        }
        return *this;
    }

} // namespace yoi