//
// Created by XIaokang00010 on 2025/7/31.
//

#include "objectLinker.h"

namespace yoi {
    ObjectLinker::ObjectLinker(const yoi::vec<yoi::wstr> &objectPaths, const std::shared_ptr<IRBuildConfig> &config) : objectPaths(objectPaths), config(config) {

    }
    yoi::wstr ObjectLinker::getLinkerPath() const {
        return linkerPath;
    }
    ObjectLinker &ObjectLinker::setLinkerPath(const yoi::wstr &linkerPath) {
        this->linkerPath = linkerPath;
        return *this;
    }
    ObjectLinker &ObjectLinker::setObjectPaths(const yoi::vec<yoi::wstr> &objectPaths) {
        this->objectPaths = objectPaths;
        return *this;
    }
    yoi::vec<yoi::wstr> ObjectLinker::getObjectPaths() const {
        return objectPaths;
    }
    yoi::wstr ObjectLinker::getElysiaRuntimePath() const {
        return elysiaRuntimePath;
    }
    ObjectLinker &ObjectLinker::setElysiaRuntimePath(const yoi::wstr &elysiaRuntimePath) {
        this->elysiaRuntimePath = elysiaRuntimePath;
        return *this;
    }
    std::shared_ptr<IRBuildConfig> ObjectLinker::getConfig() const {
        return config;
    }
    ObjectLinker &ObjectLinker::setConfig(const std::shared_ptr<IRBuildConfig> &config) {
        this->config = config;
        return *this;
    }
    
    yoi::vec<yoi::wstr> ObjectLinker::defaultAdditionalLinkingFiles() {
        std::filesystem::path dir = whereIsHoshiLang();
        yoi::vec<yoi::wstr> files;
        for (auto &path : std::filesystem::recursive_directory_iterator(dir)) {
            if (path.path().extension() == L".lib" || path.path().extension() == L".a" ||
                path.path().extension() == L".so" || path.path().extension() == L".dylib") {
                files.push_back(path.path().wstring());
            }
        }
        return files;
    }
} // namespace yoi