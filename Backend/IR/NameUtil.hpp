//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_VARUTIL_HPP
#define XSCRIPT2_VARUTIL_HPP

#include <string>

namespace Hoshi {
    extern char GlobalNamePrefix;
    extern char LocalNamePrefix;
    std::string NewVarName(std::string prefix);
    std::string NewLabelName(std::string prefix);
}

#endif