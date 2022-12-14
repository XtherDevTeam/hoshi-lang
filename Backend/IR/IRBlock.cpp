//
// Created by theflysong on 2022/8/26.
//

#include <IR/IRBlock.hpp>
#include <HoshiException.hpp>

namespace Hoshi {
    /**
     * @brief construct an IRBlock
     * @param Name Name of the block
     * @param IRCollection IR in the block
     * @param Arguments Arguments in the block
     */
    IRBlock::IRBlock(const XString Name, const XArray<IR> &&IRCollection, const XArray<Argument> &&Arguments)
        : Name(Name), IRCollection(std::move(IRCollection)), Arguments(std::move(Arguments)) {
    }
    /**
     * @brief Get the name of the block
     * @return Name of the block
     */
    const XString IRBlock::GetName(void) const {
        return Name;
    }
    /**
     * @brief Get IR of the block
     * @return IR of the block
     */
    const XArray<IR> IRBlock::GetIRCollection(void) const {
        return IRCollection;
    }
    /**
     * @brief Get Arguments of the block
     * @return Arguments of the block
     */
    const XArray<Argument> IRBlock::GetArguments(void) const {
        return Arguments;
    }
    /**
     * @brief construct an IRBlock Builder
     */
    IRBlock::Builder::Builder(void)
        : Name(L"") {
    }
    /**
     * @brief set the name of the IRBlock
     * @param Name name of the IRBlock
     * @return self
     */
    IRBlock::Builder &IRBlock::Builder::SetName(XString Name) {
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
     * @brief add Argument to the IRBlock
     * @param Arg the Argument
     * @return self
     */
    IRBlock::Builder &IRBlock::Builder::AddArgument(Argument Arg) {
        Arguments.push_back(Arg);
        return *this;
    }
    /**
     * @brief create an IRBlock
     * @return IRBlock
     */
    IRBlock IRBlock::Builder::build(void) const {
        if (Name == L"")
            throw HoshiException(L"The block need a name!");
        return IRBlock(Name, std::move(IRCollection), std::move(Arguments));
    }
}