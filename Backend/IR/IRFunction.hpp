//
// Created by theflysong on 2022/10/5.
//

#ifndef XSCRIPT2_IRFUNCTION_HPP
#define XSCRIPT2_IRFUNCTION_HPP

#include <IR/IRBlock.hpp>

namespace Hoshi {
    class IRFunction {
        /**
         * @brief Name of the function
         */
        const XString Name;
        /**
         * @brief IRBlocks in the function
         */
        const XArray<IRBlock> Blocks;

        /**
         * @brief construct an IRFunction
         * @param Name Name of the function
         * @param Blocks IRBlocks in the function
         */
        IRFunction(XString Name, const XArray<IRBlock> &&Blocks);

    public:
        class Builder {
            /**
             * @brief Name of the function
             */
            XString Name;
            /**
             * @brief IRBlocks in the function
             */
            XArray<IRBlock> Blocks;
        public:
            /**
             * @brief construct an IRFunction Builder
             */
            Builder();

            /**
             * @brief set the name of the IRFunction
             * @param Name name of the IRFunction
             * @return self
             */
            Builder &SetName(XString Name);

            /**
             * @brief add IRBlock to the IRFunction
             * @param Block the Block
             * @return self
             */
            Builder &AddBlock(IRBlock &&Block);

            /**
             * @brief create an IRFunction
             * @return IRFunction
             */
            IRFunction build();
        };

        /**
         * @brief Get the name of the function
         * @return Name of the function
         */
        XString GetName() const;

        /**
         * @brief Get blocks of the function
         * @return Blocks of the function
         */
        XArray<IRBlock> GetBlocks() const;

        friend class IRFunction::Builder;
    };
}

#endif