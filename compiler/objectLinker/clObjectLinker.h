//
// Created by XIaokang00010 on 2025/7/31.
//

#ifndef CLOBJECTLINKER_H
#define CLOBJECTLINKER_H

#include "objectLinker.h" // Include the base class header

namespace yoi {
    class clObjectLinker : public ObjectLinker {
        yoi::vec<yoi::wstr> vsRuntimePath;
    public:
        explicit clObjectLinker(const yoi::wstr &objectPath, const std::shared_ptr<IRBuildConfig> &config);

        ObjectLinker &searchAndSetupLinker() override;
        ObjectLinker &link(const yoi::wstr &outputPath) override;
    };
} // namespace yoi

#endif // CLOBJECTLINKER_H