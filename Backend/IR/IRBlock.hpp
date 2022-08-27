//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_IRBLOCK_HPP
#define XSCRIPT2_IRBLOCK_HPP

#include <IR/IR.hpp>
#include <vector>
#include <Config.hpp>

namespace Hoshi {
    class Argument {
        /**
         * @brief Type of the argument
         */
        XString TypeName;
        /**
         * @brief Name of the argument
         */
        XString Name;
    };

    class IRBlock {
        /**
         * @brief Name of the block
         */
        const XString Name;
        /**
         * @brief IR in the block
         */
        const XArray<IR> IRCollection;
        /**
         * @brief Arguments in the block
         */
        const XArray<Argument> Arguments;
        /**
         * @brief construct an IRBlock
         * @param Name Name of the block
         * @param IRCollection IR in the block
         * @param Arguments Arguments in the block
         */
        IRBlock(const XString Name, const XArray<IR> &&IRCollection, const XArray<Argument> &&Arguments);
    public:
        class Builder {
            /**
             * @brief Name of the block
             */
            XString Name;
            /**
             * @brief IR in the block
             */
            XArray<IR> IRCollection;
            /**
             * @brief Arguments in the block
             */
            XArray<Argument> Arguments;
        public:
            /**
             * @brief construct an IRBlock Builder
             */
            Builder(void);
            /**
             * @brief set the name of the IRBlock
             * @param Name name of the IRBlock
             * @return self
             */
            Builder &SetName(XString Name);
            /**
             * @brief add IR to the IRBlock
             * @param ir the IR
             * @return self
             */
            Builder &AddIR(IR ir);
            /**
             * @brief add Argument to the IRBlock
             * @param Arg the Argument
             * @return self
             */
            Builder &AddArgument(Argument Arg);
            /**
             * @brief create an IRBlock
             * @return IRBlock
             */
            IRBlock build(void) const;
        };
        /**
         * @brief Get the name of the block
         * @return Name of the block
         */
        const XString GetName(void) const;
        /**
         * @brief Get IR of the block
         * @return IR of the block
         */
        const XArray<IR> GetIRCollection(void) const;
        /**
         * @brief Get Arguments of the block
         * @return Arguments of the block
         */
        const XArray<Argument> GetArguments(void) const;
        friend class IRBlock::Builder;
    };
}

#endif