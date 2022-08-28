//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_XSTRINGINPUTPASSRESULT_HPP
#define XSCRIPT2_XSTRINGINPUTPASSRESULT_HPP

#include <Passes/Pass.hpp>

namespace Hoshi {
    class XStringInputPassResult : public XStringPassResult {
    public:
        XStringInputPassResult(XString Input);
    };
}

#endif