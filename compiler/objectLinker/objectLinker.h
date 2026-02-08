//
// Created by XIaokang00010 on 2025/7/31.
//

#ifndef OBJECTLINKER_H
#define OBJECTLINKER_H

#include "compiler/ir/IR.h"
#include <memory>
#include <share/def.hpp>
#include <filesystem>

namespace yoi {
    class ObjectLinker {
        yoi::wstr linkerPath;
        yoi::vec<yoi::wstr> objectPaths;
        yoi::wstr elysiaRuntimePath;
        std::shared_ptr<IRBuildConfig> config;
    public:
        ObjectLinker(const yoi::vec<yoi::wstr> &objectPaths, const std::shared_ptr<IRBuildConfig> &config);

        yoi::wstr getLinkerPath() const;

        ObjectLinker &setLinkerPath(const yoi::wstr &linkerPath);

        yoi::vec<yoi::wstr> getObjectPaths() const;

        ObjectLinker &setObjectPaths(const yoi::vec<yoi::wstr> &objectPaths);

        yoi::wstr getElysiaRuntimePath() const;

        ObjectLinker &setElysiaRuntimePath(const yoi::wstr &elysiaRuntimePath);

        std::shared_ptr<IRBuildConfig> getConfig() const;

        ObjectLinker &setConfig(const std::shared_ptr<IRBuildConfig> &config);

        virtual ObjectLinker &searchAndSetupLinker() = 0;

        virtual ObjectLinker &link(const yoi::wstr &outputPath) = 0;

        virtual ~ObjectLinker() = default;

        static yoi::vec<yoi::wstr> defaultAdditionalLinkingFiles();
    };
} // namespace yoi
#endif // OBJECTLINKER_H