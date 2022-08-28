//
// Created by chou on 8/28/22.
//

#ifndef HOSHI_LANG_TOASTPASS_HPP
#define HOSHI_LANG_TOASTPASS_HPP

#include "Pass.hpp"

namespace Hoshi {

    class ToASTPassResult : public ASTPassResult {
    public:
        friend class ToASTPass;
    };

    class ToASTPass  : public Pass<XString, AST> {
        ToASTPassResult PassResult;
    public:
        /**
         * @brief Construct a Pass
         * @param LastPass Result of Last Pass
         */
        ToASTPass(XStringPassResult LastPass);
        /**
         * @brief Run a Pass
         */
        virtual void Run() override;
    };

} // Hoshi

#endif //HOSHI_LANG_TOASTPASS_HPP
