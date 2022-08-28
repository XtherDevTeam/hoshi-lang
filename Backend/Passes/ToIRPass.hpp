//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_TOIRPASS_HPP
#define XSCRIPT2_TOIRPASS_HPP

#include <Passes/Pass.hpp>

namespace Hoshi {
    class ToIRPassResult : public IRProgramPassResult {
    public:
        friend class ToIRPass;
    };

    class ToIRPass : public Pass<AST, IRProgram::Builder> {
        ToIRPassResult PassResult;
    public:
        /**
         * @brief Construct a Pass
         * @param LastPass Result of Last Pass
         */
        ToIRPass(ASTPassResult &ast);
        /**
         * @brief Run a Pass
         */
        virtual void Run(void) override;
    };
}

#endif