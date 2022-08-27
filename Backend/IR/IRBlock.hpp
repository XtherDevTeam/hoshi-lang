//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_IRBLOCK_HPP
#define XSCRIPT2_IRBLOCK_HPP

#include <IR/IR.hpp>
#include <vector>

namespace Hoshi {
    class IRBlock {
        /**
         * @brief Name of the block
         */
        const std::string Name;
        /**
         * @brief IR in the block
         */
        const std::vector<IR> IRCollection;
        /**
         * @brief construct an IRBlock
         * @param Name Name of the block
         * @param IRCollection IR in the block
         */
        IRBlock(const std::string Name, const std::vector<IR> &&IRCollection);
    public:
        class Builder {
            std::string Name;
            std::vector<IR> IRCollection;
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
            Builder &SetName(std::string Name);
            /**
             * @brief add IR to the IRBlock
             * @param ir the IR
             * @return self
             */
            Builder &AddIR(IR ir);
            /**
             * @brief create an IRBlock
             * @return IRBlock
             */
            IRBlock build(void);
        };
        /**
         * @brief Get the name of the block
         * @return Name of the block
         */
        const std::string GetName(void);
        /**
         * @brief Get IR of the block
         * @return IR of the block
         */
        const std::vector<IR> GetIRCollection(void);
        friend class IRBlock::Builder;
    };
}

#endif