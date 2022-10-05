//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_IRPROGRAM_HPP
#define XSCRIPT2_IRPROGRAM_HPP

#include <IR/IRFunction.hpp>
#include <IR/ProgramContext.hpp>

namespace Hoshi {
    class IRProgram {
        /**
         * @brief Name of the program
         */
        const XString Name;
        /**
         * @brief IRFunctions in the program
         */
        const XArray<IRFunction> Functions;

        /**
         * @brief construct an IRProgram
         * @param Name Name of the program
         * @param Functions IRFunctions in the program
         * @param Context Context of the program
         */
        IRProgram(XString Name, const XArray<IRFunction> &&Functions, const ProgramContext &&Context);

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
             * @brief IRFunctions in the program
             */
            XArray<IRFunction> Functions;
            /**
             * @brief Context of the program
             */
            ProgramContext Context;
        public:
            /**
             * @brief construct an IRProgram Builder
             */
            Builder();

            /**
             * @brief set the name of the IRProgram
             * @param Name name of the IRProgram
             * @return self
             */
            Builder &SetName(XString Name);

            /**
             * @brief add IRFunction to the IRProgram
             * @param Function the Function
             * @return self
             */
            Builder &AddFunction(IRFunction &&Function);

            /**
             * @brief Get the context of program
             * @return Context
             */
            ProgramContext &GetContext();

            /**
             * @brief create an IRProgram
             * @return IRProgram
             */
            IRProgram build();
        };

        /**
         * @brief Get the name of the program
         * @return Name of the program
         */
        XString GetName() const;

        /**
         * @brief Get blocks of the program
         * @return Blocks of the program
         */
        XArray<IRFunction> GetFunctions() const;

        friend class IRProgram::Builder;
    };
}

#endif