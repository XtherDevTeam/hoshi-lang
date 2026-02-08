//
// Created by XIaokang00010 on 2025/7/31.
//

#include "ccObjectLinker.h"
#include "compiler/ir/IR.h"
#include "share/def.hpp"
#include <cstring>
#include <sstream>
#include <filesystem>
#include <string>

namespace yoi {

    ccObjectLinker::ccObjectLinker(const yoi::vec<yoi::wstr> &objectPaths, const std::shared_ptr<IRBuildConfig> &config)
        : ObjectLinker(objectPaths, config) {
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
        const std::string executable_extension;
#endif

        // iterate through each segment in PATH
        while (std::getline(iss, path_segment, path_delimiter)) {
            std::filesystem::path full_path = std::filesystem::path(path_segment) / (commandName + executable_extension);

            // check if the file exists and is a regular file (i.e., not a directory)
            if (std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path)) {
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
        if (this->getObjectPaths().empty()) {
            throw std::runtime_error("ccObjectLinker: Object file path not set.");
        }

        std::string command = "\"" + yoi::wstring2string(this->getLinkerPath()) + "\"";
        for (const auto &objectPath : this->getObjectPaths()) {
            command += " \"";
            command += yoi::wstring2string(objectPath) + "\"";
        }

        // add additional linking files
        if (strcmp(YOI_PLATFORM, "darwin") != 0)
            // if the platform is not darwin, we need to add -Wl,--start-group and -Wl,--end-group to link as groups
            command += " -Wl,--start-group";
        for (const auto &file : this->getConfig()->additionalLinkingFiles) {
            // link as groups
            command += " \"" + yoi::wstring2string(file) + "\"";
        }
        if (strcmp(YOI_PLATFORM, "darwin") != 0)
            command += " -Wl,--end-group";

        command += " -o \"";
        command += yoi::wstring2string(outputPath) + "\"";

        // add Elysia runtime path and library
        if (!this->getElysiaRuntimePath().empty()) {
            // ensure the elysiaRuntimePath exists as a directory
            std::filesystem::path elysia_path_fs(this->getElysiaRuntimePath());
            if (!std::filesystem::exists(elysia_path_fs) || !std::filesystem::is_directory(elysia_path_fs)) {
                std::string error_msg = "ccObjectLinker: Elysia runtime path does not exist or is not a directory: " +
                                        yoi::wstring2string(this->getElysiaRuntimePath());
                throw std::runtime_error(error_msg);
            }

            command += " -L\""; // add library search path
            command += yoi::wstring2string(this->getElysiaRuntimePath()) + "\"";
            command += " -lelysia_runtime";
        }

        if (this->getConfig()->buildType == IRBuildConfig::BuildType::library) {
            command += " -shared"; // build a shared library
        }

        // synchronize the release/debug
        if (this->getConfig()->buildMode == IRBuildConfig::BuildMode::debug) {
            command += " -g";
        }
#ifdef _WIN32
        command += " -mconsole"; // fuck argc, argv
        replace_all(command, std::string("\""), std::string("\\\""));
        command = "powershell.exe -Command \"&" + command + "\""; // fuck win32 command line
#endif

        int result = std::system(command.c_str());
        if (result != 0) {
            std::string error_msg = "ccObjectLinker: Linker command failed with exit code " + std::to_string(result) + ". Command: " + command;
            throw std::runtime_error(error_msg);
        }
        return *this;
    }

} // namespace yoi