//
// Created by theflysong on 2022/8/27.
//

#include <IR/NameUtil.hpp>

namespace Hoshi {
    XCharacter GlobalNamePrefix = '@';
    XCharacter LocalNamePrefix = '%';

    int VarIndex = 0;
    int LabelIndex = 0;

    XString NewVarName(XString prefix) {
        int Idx = VarIndex ++;
        return prefix + L"_var_" + std::to_wstring(VarIndex);
    }

    XString NewLabelName(XString prefix) {
        int Idx = LabelIndex ++;
        return prefix + L"_label_" + std::to_wstring(VarIndex);
    }
}