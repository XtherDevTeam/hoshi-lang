//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_VARUTIL_HPP
#define XSCRIPT2_VARUTIL_HPP

#include <string>
#include <Config.hpp>

namespace Hoshi {
    extern XCharacter GlobalNamePrefix;
    extern XCharacter LocalNamePrefix;

    XString NewVarName(XString prefix);

    XString NewLabelName(XString prefix);
}

#endif