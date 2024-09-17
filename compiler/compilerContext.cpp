//
// Created by XIaokang00010 on 2024/9/6.
//

#include "compilerContext.h"

namespace yoi {
    void compilerContext::compileModule(const yoi::wstr &filepath) {

    }

    const std::shared_ptr<IRObjectFile> &compilerContext::getIRObjectFile() const {
        return irObjectFile;
    }
} // yoi