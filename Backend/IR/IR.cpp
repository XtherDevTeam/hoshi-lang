//
// Created by theflysong on 2022/8/26.
//

#include <IR/IR.hpp>

namespace Hoshi {
    /**
     * @brief Construct an IR
     * @param SourceOprendA First Source Oprend
     * @param SourceOprendB Second Source Oprend
     * @param DestinationOprend Destination Oprend
     */
    IR::IR(const Opcode Code, const Oprend SourceOprendA, const Oprend SourceOprendB, const Oprend DestinationOprend)
        : Code(Code), SourceOprendA(SourceOprendA), SourceOprendB(SourceOprendB), DestinationOprend(DestinationOprend) {
    }
    /**
     * @brief Get the code of the IR
     * @return Code of the IR
     */
    const Opcode IR::GetCode(void) const {
        return Code;
    }
    /**
     * @brief Get the first source oprend of the IR
     * @return First Source Oprend of the IR
     */
    const Oprend IR::GetSourceOprendA(void) const {
        return SourceOprendA;
    }
    /**
     * @brief Get the second source oprend of the IR
     * @return Second Source Oprend of the IR
     */
    const Oprend IR::GetSourceOprendB(void) const {
        return SourceOprendB;
    }
    /**
     * @brief Get the destination oprend of the IR
     * @return Destination Oprend of the IR
     */
    const Oprend IR::GetDestinationOprend(void) const {
        return DestinationOprend;
    }
}