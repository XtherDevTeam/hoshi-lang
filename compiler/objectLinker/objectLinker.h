//
// Created by XIaokang00010 on 2025/7/31.
//

#ifndef OBJECTLINKER_H
#define OBJECTLINKER_H

#include "compiler/ir/IR.h"
#include <memory>
#include <share/def.hpp>

namespace yoi {
    class ObjectLinker {
        yoi::wstr linkerPath;
        yoi::wstr objectPath;
        yoi::wstr elysiaRuntimePath;
        std::shared_ptr<IRBuildConfig> config;
    public:
        ObjectLinker(const yoi::wstr &objectPath, const std::shared_ptr<IRBuildConfig> &config);

        yoi::wstr getLinkerPath() const;

        ObjectLinker &setLinkerPath(const yoi::wstr &linkerPath);

        yoi::wstr getObjectPath() const;

        ObjectLinker &setObjectPath(const yoi::wstr &objectPath);

        yoi::wstr getElysiaRuntimePath() const;

        ObjectLinker &setElysiaRuntimePath(const yoi::wstr &elysiaRuntimePath);

        std::shared_ptr<IRBuildConfig> getConfig() const;

        ObjectLinker &setConfig(const std::shared_ptr<IRBuildConfig> &config);

        virtual ObjectLinker &searchAndSetupLinker() = 0;

        virtual ObjectLinker &link(const yoi::wstr &outputPath) = 0;

        virtual ~ObjectLinker() = default;
    };
} // namespace yoi
#endif // OBJECTLINKER_H