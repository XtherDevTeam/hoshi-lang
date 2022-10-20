//
// Created by theflysong on 2022/8/27.
//

#include <IR/IRProgram.hpp>

namespace Hoshi {
    /**
     * @brief construct an IRProgram
     * @param Name Name of the program
     * @param Functions IRFunctionss in the program
     */
    IRProgram::IRProgram(const XString Name, const XArray<IRFunction> &&Functions, const ProgramContext &&Context)
            : Name(Name), Functions(Functions), Context(Context) {
    }

    /**
     * @brief construct an IRProgram Builder
     */
    IRProgram::Builder::Builder()
            {
    }

    /**
     * @brief set the name of the IRProgram
     * @param Name name of the IRProgram
     * @return self
     */
    IRProgram::Builder &IRProgram::Builder::SetName(XString Name) {
        this->Name = Name;
        return *this;
    }

    /**
     * @brief add IRFunction to the IRProgram
     * @param function the function
     * @return self
     */
    IRProgram::Builder &IRProgram::Builder::AddFunction(IRFunction &&Function) {
        this->Functions.push_back(Function);
        return *this;
    }

    /**
     * @brief Get the context of program
     * @return Context
     */
    ProgramContext &IRProgram::Builder::GetContext() {
        return Context;
    }

    /**
     * @brief create an IRProgram
     * @return IRProgram
     */
    IRProgram IRProgram::Builder::build() {
        return IRProgram(Name, std::move(Functions), std::move(Context));
    }

    /**
     * @brief Get the name of the program
     * @return Name of the program
     */
    XString IRProgram::GetName() const {
        return Name;
    }

    /**
     * @brief Get functions of the program
     * @return Functions of the program
     */
    XArray<IRFunction> IRProgram::GetFunctions() const {
        return Functions;
    }
}