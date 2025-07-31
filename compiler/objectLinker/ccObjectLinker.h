//
// Created by XIaokang00010 on 2025/7/31.
//

#ifndef YOICH_CCOBJECTLINKER_HPP
#define YOICH_CCOBJECTLINKER_HPP

#include "objectLinker.h"
#include <string>

namespace yoi {
    class ccObjectLinker : public ObjectLinker {
    public:
        ccObjectLinker(const yoi::wstr &objectPath);

        ObjectLinker &searchAndSetupLinker() override;
        ObjectLinker &link(const yoi::wstr &outputPath) override;

    private:
        bool commandExists(const std::string& command);
    };

} // namespace yoi

#endif // YOICH_CCOBJECTLINKER_HPP