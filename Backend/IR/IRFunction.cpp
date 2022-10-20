//
// Created by theflysong on 2022/10/5.
//

#include <IR/IRFunction.hpp>
#include <HoshiException.hpp>

namespace Hoshi {
    /**
     * @brief construct an IRFunction
     * @param Name Name of the function
     * @param Blocks IRBlocks in the function
     */
    IRFunction::IRFunction(const XString Name, const XArray<IRBlock> &&Blocks)
            : Name(Name), Blocks(Blocks) {
    }

    /**
     * @brief Get the name of the function
     * @return Name of the function
     */
    XString IRFunction::GetName() const {
        return Name;
    }

    /**
     * @brief Get blocks of the function
     * @return Blocks of the function
     */
    XArray<IRBlock> IRFunction::GetBlocks() const {
        return Blocks;
    }

    /**
     * @brief construct an IRFunction Builder
     */
    IRFunction::Builder::Builder()
            {
    }

    /**
     * @brief set the name of the IRFunction
     * @param Name name of the IRFunction
     * @return self
     */
    IRFunction::Builder &IRFunction::Builder::SetName(XString Name) {
        this->Name = Name;
        return *this;
    }

    /**
     * @brief add IRBlock to the IRFunction
     * @param Block the Block
     * @return self
     */
    IRFunction::Builder &IRFunction::Builder::AddBlock(IRBlock &&Block) {
        this->Blocks.push_back(Block);
        return *this;
    }

    /**
     * @brief create an IRFunction
     * @return IRFunction
     */
    IRFunction IRFunction::Builder::build() {
        if (Name.empty())
            throw HoshiException(L"The function need a name!");
        return IRFunction(Name, std::move(Blocks));
    }
}