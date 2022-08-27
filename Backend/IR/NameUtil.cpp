//
// Created by theflysong on 2022/8/27.
//

#include <IR/NameUtil.hpp>

namespace Hoshi {
    char GlobalNamePrefix = '@';
    char LocalNamePrefix = '%';

    int VarIndex = 0;
    int LabelIndex = 0;

    std::string NewVarName(std::string prefix) {
        int Idx = VarIndex ++;
        return prefix + "_var_" + std::to_string(VarIndex);
    }

    std::string NewLabelName(std::string prefix) {
        int Idx = LabelIndex ++;
        return prefix + "_label_" + std::to_string(VarIndex);
    }
}