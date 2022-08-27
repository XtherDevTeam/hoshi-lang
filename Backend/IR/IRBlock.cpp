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
    /**
     * @brief construct an IRBlock Builder
     */
    IRBlock::Builder::Builder(void)
        : Name("") {
    }
    /**
     * @brief set the name of the IRBlock
     * @param Name name of the IRBlock
     * @return self
     */
    IRBlock::Builder &IRBlock::Builder::SetName(std::string Name) {
        this->Name = Name;
        return *this;
    }
    /**
     * @brief add IR to the IRBlock
     * @param ir the IR
     * @return self
     */
    IRBlock::Builder &IRBlock::Builder::AddIR(IR ir) {
        IRCollection.push_back(ir);
        return *this;
    }
    /**
     * @brief create an IRBlock
     * @return IRBlock
     */
    IRBlock IRBlock::Builder::build(void) {
        if (Name == "")
            throw "The block need a name!";
        return IRBlock(this->Name, std::move(this->IRCollection));
    }
}