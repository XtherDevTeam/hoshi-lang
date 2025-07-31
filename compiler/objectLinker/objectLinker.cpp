//
// Created by XIaokang00010 on 2025/7/31.
//

#include "objectLinker.h"

namespace yoi {
    ObjectLinker::ObjectLinker(const yoi::wstr &objectPath) : objectPath(objectPath) {

    }
    yoi::wstr ObjectLinker::getLinkerPath() const {
        return linkerPath;
    }
    ObjectLinker &ObjectLinker::setLinkerPath(const yoi::wstr &linkerPath) {
        this->linkerPath = linkerPath;
        return *this;
    }
    ObjectLinker &ObjectLinker::setObjectPath(const yoi::wstr &objectPath) {
        this->objectPath = objectPath;
        return *this;
    }
    yoi::wstr ObjectLinker::getObjectPath() const {
        return objectPath;
    }
    yoi::wstr ObjectLinker::getElysiaRuntimePath() const {
        return elysiaRuntimePath;
    }
    ObjectLinker &ObjectLinker::setElysiaRuntimePath(const yoi::wstr &elysiaRuntimePath) {
        this->elysiaRuntimePath = elysiaRuntimePath;
        return *this;
    }
} // namespace yoi