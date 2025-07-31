//
// Created by XIaokang00010 on 2025/7/31.
//

#ifndef OBJECTLINKER_H
#define OBJECTLINKER_H

#include <share/def.hpp>

namespace yoi {
    class ObjectLinker {
        yoi::wstr linkerPath;
        yoi::wstr objectPath;
        yoi::wstr elysiaRuntimePath;
    public:
        ObjectLinker(const yoi::wstr &objectPath);

        yoi::wstr getLinkerPath() const;

        ObjectLinker &setLinkerPath(const yoi::wstr &linkerPath);

        yoi::wstr getObjectPath() const;

        ObjectLinker &setObjectPath(const yoi::wstr &objectPath);

        yoi::wstr getElysiaRuntimePath() const;

        ObjectLinker &setElysiaRuntimePath(const yoi::wstr &elysiaRuntimePath);

        virtual ObjectLinker &searchAndSetupLinker() = 0;

        virtual ObjectLinker &link(const yoi::wstr &outputPath) = 0;

        virtual ~ObjectLinker() = default;
    };
} // namespace yoi
#endif // OBJECTLINKER_H