//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_IR_HPP
#define XSCRIPT2_IR_HPP

#include <Oprend.hpp>

namespace Hoshi {
    enum class Opcode : int {
        NOP
    };

    class IR {
    protected:
        /**
         * @brief Code of the IR
         */
        const Opcode Code;
        /**
         * @brief First Source Oprend
         */
        const Oprend SourceOprendA;
        /**
         * @brief Second Source Oprend
         */
        const Oprend SourceOprendB;
        /**
         * @brief Destination Oprend
         */
        const Oprend DestinationOprend;
    public:
        /**
         * @brief Construct an IR
         * @param SourceOprendA First Source Oprend
         * @param SourceOprendB Second Source Oprend
         * @param DestinationOprend Destination Oprend
         */
        IR(const Opcode Code, const Oprend SourceOprendA, const Oprend SourceOprendB, const Oprend DestinationOprend);
        /**
         * @brief Get the code of the IR
         * @return Code of the IR
         */
        const Opcode GetCode(void) const;
        /**
         * @brief Get the first source oprend of the IR
         * @return First Source Oprend of the IR
         */
        const Oprend GetSourceOprendA(void) const;
        /**
         * @brief Get the second source oprend of the IR
         * @return Second Source Oprend of the IR
         */
        const Oprend GetSourceOprendB(void) const;
        /**
         * @brief Get the destination oprend of the IR
         * @return Destination Oprend of the IR
         */
        const Oprend GetDestinationOprend(void) const;
    };
}

#endif