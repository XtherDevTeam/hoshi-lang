//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_COMPILERCONTEXT_H
#define HOSHI_LANG_COMPILERCONTEXT_H

#include "share/def.hpp"
#include "compiler/ir/IR.h"
#include <map>

namespace yoi {
    class moduleContext;

    class compilerContext {
        yoi::indexTable<yoi::wstr, std::shared_ptr<yoi::moduleContext>> modules;
        std::map<yoi::indexT, bool> isModuleImported;
        std::shared_ptr<IRObjectFile> irObjectFile;

    public:
        compilerContext(const compilerContext& context) = default;

        void compileModule(const yoi::wstr &filepath);

        const std::shared_ptr<IRObjectFile>& getIRObjectFile() const;
    };

} // yoi

#endif //HOSHI_LANG_COMPILERCONTEXT_H
