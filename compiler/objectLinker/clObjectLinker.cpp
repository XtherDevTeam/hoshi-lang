// clObjectLinker.cpp
#include "clObjectLinker.h"
#include "share/def.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>      // For _wsystem (Windows specific)
#include <filesystem>   // For std::filesystem operations

namespace yoi {

    clObjectLinker::clObjectLinker(const yoi::wstr &objectPath, const std::shared_ptr<IRBuildConfig> &config)
        : ObjectLinker(objectPath, config) {
        // Constructor simply calls the base class constructor.
    }

    ObjectLinker &clObjectLinker::searchAndSetupLinker() {
        std::filesystem::path cl_exe_name = L"cl.exe";
        std::filesystem::path found_path;

        // 1. Check PATH environment variable
        // _wgetenv returns a pointer to a wide-character string (read-only)
        const char* path_env = std::getenv("PATH");
        if (path_env) {
            std::wstring path_env_str = yoi::string2wstring(path_env);
            size_t current_pos = 0;
            size_t delimiter_pos;

            while ((delimiter_pos = path_env_str.find(L';', current_pos)) != std::wstring::npos) {
                std::filesystem::path dir = path_env_str.substr(current_pos, delimiter_pos - current_pos);
                std::filesystem::path potential_cl_path = dir / cl_exe_name;
                if (std::filesystem::exists(potential_cl_path) && std::filesystem::is_regular_file(potential_cl_path)) {
                    found_path = potential_cl_path;
                    break;
                }
                current_pos = delimiter_pos + 1;
            }
            // Check the last path segment (or if no delimiters were found)
            if (found_path.empty()) {
                std::filesystem::path dir = path_env_str.substr(current_pos);
                std::filesystem::path potential_cl_path = dir / cl_exe_name;
                if (std::filesystem::exists(potential_cl_path) && std::filesystem::is_regular_file(potential_cl_path)) {
                    found_path = potential_cl_path;
                }
            }
        }

        if (!found_path.empty()) {
            setLinkerPath(found_path.wstring());
            return *this;
        }

        // 2. Search common Visual Studio installation paths
        // This search can be extensive and slow. Limiting to common patterns and x64 host/target.
        std::vector<std::filesystem::path> vs_install_bases = {
            L"C:/Program Files (x86)/Microsoft Visual Studio/", // For older VS or 32-bit components
            L"C:/Program Files/Microsoft Visual Studio/"        // For newer VS or 64-bit components
        };

        for (const auto& base_path : vs_install_bases) {
            if (!std::filesystem::exists(base_path) || !std::filesystem::is_directory(base_path)) {
                continue;
            }
            for (const auto& year_entry : std::filesystem::directory_iterator(base_path)) {
                if (!year_entry.is_directory()) continue;

                for (const auto& component_entry : std::filesystem::directory_iterator(year_entry.path())) {
                    if (!component_entry.is_directory()) continue;

                    std::filesystem::path vc_tools_msvc_path = component_entry.path() / L"VC" / L"Tools" / L"MSVC";

                    if (!std::filesystem::exists(vc_tools_msvc_path) || !std::filesystem::is_directory(vc_tools_msvc_path)) {
                        continue;
                    }

                    // Iterate through specific MSVC toolchain versions (e.g., "14.37.32822")
                    for (const auto& msvc_version_entry : std::filesystem::directory_iterator(vc_tools_msvc_path)) {
                        if (!msvc_version_entry.is_directory()) continue;

                        // Common bin paths relative to MSVC version folder (prefer x64 host/target)
                        std::vector<std::filesystem::path> bin_sub_paths = {
                            L"bin/Hostx64/x64",  // Preferred: 64-bit host, 64-bit target
                            L"bin/Hostx86/x64",  // 32-bit host, 64-bit target (e.g., when run from VS dev cmd x86)
                            L"bin/Hostx64/x86",  // 64-bit host, 32-bit target
                            L"bin/Hostx86/x86"   // 32-bit host, 32-bit target
                        };

                        for (const auto& bin_sub_path : bin_sub_paths) {
                            std::filesystem::path potential_cl_path = msvc_version_entry.path() / bin_sub_path / cl_exe_name;
                            if (std::filesystem::exists(potential_cl_path) && std::filesystem::is_regular_file(potential_cl_path)) {
                                found_path = potential_cl_path;
                                setLinkerPath(found_path.wstring());
                                std::wcout << L"clObjectLinker: Found cl.exe by searching VS installs: " << getLinkerPath() << std::endl;
                                return *this;
                            }
                        }
                    }
                }
            }
        }

        throw std::runtime_error("cl.exe linker not found. Please ensure Visual Studio Build Tools are installed and configured, or add cl.exe to your system PATH.");
    }

    ObjectLinker &clObjectLinker::link(const yoi::wstr &outputPath) {
        if (getLinkerPath().empty()) {
            throw std::runtime_error("Linker path not set. Call searchAndSetupLinker() first.");
        }
        if (getObjectPath().empty()) {
            throw std::runtime_error("Object path is empty.");
        }

        std::filesystem::path output_fs_path(outputPath);
        std::filesystem::path object_fs_path(getObjectPath());
        std::filesystem::path elysia_runtime_fs_path(getElysiaRuntimePath());

        std::wstring command = L"\"" + getLinkerPath() + L"\"";
        command += L" \"" + object_fs_path.wstring() + L"\"";
        command += L" /Fe:\"" + output_fs_path.wstring() + L"\"";

        command += L" /link";

        if (!elysia_runtime_fs_path.empty()) {
            command += L" /LIBPATH:\"" + elysia_runtime_fs_path.wstring() + L"\"";
            command += L" elysia_runtime.lib";
        } else {
            warning(0, 0, "Elysia runtime library not specified. Linking may fail if runtime functions are used.");
        }

        if (this->getConfig()->buildType == IRBuildConfig::BuildType::library) {
            command += L" /LD"; // Build a shared library
        }

#if defined(_WIN32)
        replace_all(command, std::wstring(L"\""), std::wstring(L"\\\""));
        command = L"powershell.exe -Command \"& " + command + L"\""; // fuck win32 command line
#endif

        int result = system(yoi::wstring2string(command).c_str());

        if (result != 0) {
            std::string error_msg = "Linking failed. cl.exe returned error code: " + std::to_string(result) + "\nCommand: " + yoi::wstring2string(command);
            throw std::runtime_error(error_msg);
        }

        std::wcout << L"clObjectLinker: Linking successful. Output executable: " << outputPath << std::endl;
        return *this;
    }

} // namespace yoi