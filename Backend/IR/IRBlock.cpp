//
// Created by theflysong on 2022/8/26.
//

#include <IR/IRBlock.hpp>

namespace Hoshi {
    /**
     * @brief construct an IRBlock
     * @param Name Name of the block
     * @param IRCollection IR in the block
     */
    IRBlock::IRBlock(const std::string Name, const std::vector<IR> &&IRCollection)
        : Name(Name), IRCollection(std::move(IRCollection)) {
    }
    /**
     * @brief Get the name of the block
     * @return Name of the block
     */
    const std::string IRBlock::GetName(void) {
        return Name;
    }
    /**
     * @brief Get IR of the block
     * @return IR of the block
     */
    const std::vector<IR> IRBlock::GetIRCollection(void) {
        return IRCollection;
    }
}