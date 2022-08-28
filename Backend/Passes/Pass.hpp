//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_PASS_HPP
#define XSCRIPT2_PASS_HPP

// #include <AST.hpp>
#include <IR/IRProgram.hpp>

namespace Hoshi {
    template<typename ResultType> class PassResult {
    protected:
        ResultType Result;
    public:
        /**
         * @brief Result of This Pass
         */
        virtual ResultType &GetResult(void) {
            return Result;
        }
    };

    // using ASTPassResult = PassResult<AST>;
    using XStringPassResult = PassResult<XString>;
    using IRProgramPassResult = PassResult<IRProgram::Builder>;

    template<typename SourceType, typename ResultType> class Pass {
    protected:
        /**
         * @brief Result of Last Pass
         */
        PassResult<SourceType> &LastPass;
        /**
         * @brief Result of This Pass
         */
        PassResult<ResultType> &Result;
    public:
        /**
         * @brief Construct a Pass
         * @param LastPass Result of Last Pass
         * @param Result Result of This Pass
         */
        Pass(PassResult<SourceType> &LastPass, PassResult<ResultType> &Result)
            : LastPass(LastPass), Result(Result) {
        }
        /**
         * @brief Run a Pass
         */
        virtual void Run(void) = 0;
        /**
         * @brief Get Pass Result
         * @return Pass Result;
         */
        virtual PassResult<ResultType> &GetResult(void) const {
            return Result;
        }
    };
}

#endif