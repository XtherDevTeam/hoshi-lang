//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_IRPROGRAM_HPP
#define XSCRIPT2_IRPROGRAM_HPP

#include <IR/IRBlock.hpp>
#include <IR/ProgramContext.hpp>

namespace Hoshi {
    class IRProgram {
        /**
         * @brief Name of the program
         */
        const XString Name;
        /**
         * @brief IRBlocks in the program
         */
        const XArray<IRBlock> Blocks;
        /**
         * @brief construct an IRProgram
         * @param Name Name of the program
         * @param Blocks IRBlocks in the program
         * @param Context Context of the program
         */
        IRProgram(const XString Name, const XArray<IRBlock> &&Blocks, const ProgramContext &&Context);
    public:
        /**
         * @brief Context of the program
         */
        const ProgramContext Context;
        class Builder {
            /**
             * @brief Name of the program
             */
            XString Name;
            /**
             * @brief IRBlocks in the program
             */
            XArray<IRBlock> Blocks;
            /**
             * @brief Context of the program
             */
            ProgramContext Context;
        public:
            /**
             * @brief construct an IRProgram Builder
             */
            Builder(void);
            /**
             * @brief set the name of the IRProgram
             * @param Name name of the IRProgram
             * @return self
             */
            Builder &SetName(XString Name);
            /**
             * @brief add IRBlock to the IRProgram
             * @param Block the Block
             * @return self
             */
            Builder &AddBlock(IRBlock Block);
            /**
             * @brief Get the context of program
             * @return Context
             */
            ProgramContext &GetContext(void);
            /**
             * @brief create an IRProgram
             * @return IRProgram
             */
            IRProgram build(void);
        };
        /**
         * @brief Get the name of the program
         * @return Name of the program
         */
        const XString GetName(void) const;
        /**
         * @brief Get blocks of the program
         * @return Blocks of the program
         */
        const XArray<IRBlock> GetBlocks(void) const;
        friend class IRProgram::Builder;
    };
}

#endif