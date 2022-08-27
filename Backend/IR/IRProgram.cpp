//
// Created by theflysong on 2022/8/27.
//

#include <IR/IRProgram.hpp>

namespace Hoshi {
    /**
     * @brief construct an IRProgram
     * @param Name Name of the program
     * @param Blocks IRBlocks in the program
     */
    IRProgram::IRProgram(const std::string Name, const std::vector<IRBlock> &&Blocks)
        : Name(Name), Blocks(std::move(Blocks)) {
    }

    /**
     * @brief construct an IRProgram Builder
     */
    IRProgram::Builder::Builder(void)
        : Name("") {
    }
    /**
     * @brief set the name of the IRProgram
     * @param Name name of the IRProgram
     * @return self
     */
    IRProgram::Builder &IRProgram::Builder::SetName(std::string Name) {
        this->Name = Name;
        return *this;
    }
    /**
     * @brief add IRBlock to the IRProgram
     * @param Block the Block
     * @return self
     */
    IRProgram::Builder &IRProgram::Builder::AddBlock(IRBlock Block) {
        this->Blocks.push_back(Block);
        return *this;
    }
    /**
     * @brief create an IRProgram
     * @return IRProgram
     */
    IRProgram IRProgram::Builder::build(void) {
        return IRProgram(Name, std::move(Blocks));
    }
}