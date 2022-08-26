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
    public:
        /**
         * @brief construct an IRBlock
         * @param Name Name of the block
         * @param IRCollection IR in the block
         */
        IRBlock(const std::string Name, const std::vector<IR> &&IRCollection);
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
    };
}

#endif