//
// Created by Jerry Chou on 10/2/2024.
//

#include "IROptimizer.hpp"
#include "compiler/ir/IR.h"
#include "share/def.hpp"

#include <cmath>
#include <memory>

namespace yoi {
    IROptimizer::SimulationStack::Item::ContributedInstructionSet::ContributedInstructionSet(yoi::indexT codeBlockIndex,
        const std::set<yoi::indexT> &instructions): codeBlockIndex(codeBlockIndex), instructions(instructions) {
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet::ContributedInstructionSet(yoi::indexT codeBlockIndex,
        const std::set<yoi::indexT> &instructions, bool optimizable)  : codeBlockIndex(codeBlockIndex), instructions(    instructions), optimizable(optimizable) {
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet &IROptimizer::SimulationStack::Item::
    ContributedInstructionSet::insert(yoi::indexT index) {
        instructions.insert(index);
        return *this;
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet IROptimizer::SimulationStack::Item::
    ContributedInstructionSet::operator+(const ContributedInstructionSet &other) const {
        ContributedInstructionSet result{*this};
        for (auto &i: other.instructions) {
            result.insert(i);
        }
        return result;
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator::
    Iterator(const std::set<yoi::indexT> &set): it(set.begin()) {
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator::Iterator(
        std::set<yoi::indexT>::const_iterator it): it(it) {
    }

    bool IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator::operator!=(
        const Iterator &other) const {
        return it != other.it;
    }

    yoi::indexT IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator::operator*() const {
        return *it;
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator &IROptimizer::SimulationStack::Item::
    ContributedInstructionSet::Iterator::operator++() {
        ++it;
        return *this;
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator IROptimizer::SimulationStack::Item::
    ContributedInstructionSet::begin() const {
        return Iterator(instructions.begin());
    }

    IROptimizer::SimulationStack::Item::ContributedInstructionSet::Iterator IROptimizer::SimulationStack::Item::
    ContributedInstructionSet::end() const {
        return Iterator(instructions.end());
    }

    IROptimizer::SimulationStack::Item::PossibleValue::PossibleValue(int64_t intValue): intValue(intValue) {
    }

    IROptimizer::SimulationStack::Item::PossibleValue::PossibleValue(double deciValue): deciValue(deciValue) {
    }

    IROptimizer::SimulationStack::Item::PossibleValue::PossibleValue(bool boolValue): boolValue(boolValue) {
    }

    IROptimizer::SimulationStack::Item::PossibleValue::PossibleValue(yoi::indexT stringConstIndex): stringConstIndex(
        stringConstIndex) {
    }

    IROptimizer::SimulationStack::Item::PossibleValue::PossibleValue(char charValue) : charValue(charValue) {
    }


    void IROptimizer::SimulationStack::push(const std::shared_ptr<IRValueType> &type,
                                            const Item::ContributedInstructionSet &contributedInstructions) {
        items.emplace_back(Item{type, false, {}, contributedInstructions});
    }

    void IROptimizer::SimulationStack::push(const std::shared_ptr<IRValueType> &type,
                                            const Item::ContributedInstructionSet &contributedInstructions,
                                            Item::PossibleValue value) {
        items.emplace_back(Item{type, true, value, contributedInstructions});
    }

    void IROptimizer::SimulationStack::push(const Item &item) {
        items.push_back(item);
    }

    void IROptimizer::SimulationStack::pop() {
        items.pop_back();
    }

    IROptimizer::SimulationStack::Item &IROptimizer::SimulationStack::peek(yoi::indexT index) {
        assert(index < items.size());
        return items[items.size() - 1 - index];
    }

    yoi::indexT IROptimizer::reduce(const SimulationStack::Item::ContributedInstructionSet &contributedInstructions,
        yoi::indexT currentIndex) {

        // std::cout << "IROptimizer::reduce() called" << std::endl;s
        if (not contributedInstructions.optimizable) {
            return currentIndex;
        }
        for (auto i : contributedInstructions) {
            targetFunction->codeBlock[contributedInstructions.codeBlockIndex]->getIRArray()[i] = {IR::Opcode::nop, {}};
            // If the current code block index is greater than the index of the instruction being processed,
            // decrement it to account for the removal of the instruction.
            /*
            if (currentIndex > i) {
                currentIndex--;
            }
            */
        }
        return currentIndex;
    }

    IROptimizer::SimulationStack::Item IROptimizer::add(const IROptimizer::SimulationStack::Item &a,
                                                        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue + b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {a.possibleValue.deciValue + b.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? true : false}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue + b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::add(): Unsupported type for add operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::sub(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue - b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {a.possibleValue.deciValue - b.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue - b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::sub(): Unsupported type for sub operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::mul(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue * b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {a.possibleValue.deciValue * b.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? b.possibleValue.boolValue : false}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue * b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::mul(): Unsupported type for mul operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::div(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue / b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {a.possibleValue.deciValue / b.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue / b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::div(): Unsupported type for div operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::mod(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue % b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {std::fmod(a.possibleValue.deciValue, b.possibleValue.deciValue)}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue % b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::mod(): Unsupported type for mod operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::negate(const IROptimizer::SimulationStack::Item &a) {
        if (a.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {-a.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {-a.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(-a.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::negate(): Unsupported type for negate operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::bitwiseNot(const IROptimizer::SimulationStack::Item &a) {
        if (a.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {~a.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {~int64_t(a.possibleValue.deciValue)}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(~a.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::bitwiseNot(): Unsupported type for bitwise not operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::bitwiseAnd(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue & b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {int64_t(a.possibleValue.deciValue) & int64_t(b.possibleValue.deciValue)}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? b.possibleValue.boolValue : false}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue & b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::bitwiseAnd(): Unsupported type for bitwise and operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::bitwiseOr(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue | b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {int64_t(a.possibleValue.deciValue) | int64_t(b.possibleValue.deciValue)}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? true : b.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue | b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::bitwiseOr(): Unsupported type for bitwise or operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::bitwiseXor(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue ^ b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {int64_t(a.possibleValue.deciValue) ^ int64_t(b.possibleValue.deciValue)}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? b.possibleValue.boolValue : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue ^ b.possibleValue.charValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::bitwiseXor(): Unsupported type for bitwise xor operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::bitwiseShiftLeft(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue << b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {int64_t(a.possibleValue.deciValue) << b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue << b.possibleValue.intValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::bitwiseShiftLeft(): Unsupported type for bitwise shift left operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::bitwiseShiftRight(const IROptimizer::SimulationStack::Item &a,
        const IROptimizer::SimulationStack::Item &b) {
        if (a.hasPossibleValue && b.hasPossibleValue) {
            switch (a.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {a.type, true, {a.possibleValue.intValue >> b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {a.type, true, {int64_t(a.possibleValue.deciValue) >> b.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {a.type, true, {a.possibleValue.boolValue ? false : true}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {
                        a.type, true, char{static_cast<char>(a.possibleValue.charValue >> b.possibleValue.intValue)}, {}
                    };
                }
                default:
                    panic(0, 0, "IROptimizer::bitwiseShiftRight(): Unsupported type for bitwise shift right operation");
                    return {};
            }
        } else {
            return {a.type, false, {}, a.contributedInstructions + b.contributedInstructions};
        }
    }

    yoi::indexT IROptimizer::generatePushOp(const SimulationStack::Item &item, yoi::indexT index) {
        auto &IRArr = targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray();
        switch (item.type->type) {
            case IRValueType::valueType::integerObject:
                IRArr.insert(IRArr.begin() + index + 1, IR{
                                 IR::Opcode::push_integer, {IROperand{IROperand::operandType::integer, {item.possibleValue.intValue}}}});
                break;
            case IRValueType::valueType::decimalObject:
                IRArr.insert(IRArr.begin() + index + 1, IR{
                                 IR::Opcode::push_decimal, {IROperand{IROperand::operandType::decimal, {item.possibleValue.deciValue}}}});
                break;
            case IRValueType::valueType::booleanObject:
                IRArr.insert(IRArr.begin() + index + 1, IR{
                                 IR::Opcode::push_boolean, {{IROperand::operandType::boolean, IROperand::operandValue{item.possibleValue.boolValue}}}});
                break;
            case IRValueType::valueType::characterObject:
                // TODO: Implement push_character
                break;
            default:
                panic(0, 0, "IROptimizer::generatePushOp: Unsupported value type for push constant operation");
                break;
        }
        // simulationStack.push()IRArr.insert(IRArr.begin() + index, IR
        // IR traverse will handle the simulation stack by executing the push operation
        // simulationStack.push({item.type, true, item.possibleValue, {currentCodeBlockIndex, {index}}});
        return index;
    }

    IROptimizer::IROptimizer(const std::shared_ptr<compilerContext> &compilerCtx,
                             const std::shared_ptr<IRModule> &irModule): compilerCtx(compilerCtx), irModule(irModule),
                                                                         targetFunction(nullptr) {
    }

    IROptimizer &IROptimizer::setTargetFunction(const std::shared_ptr<IRFunctionDefinition> &targetFunction) {
        this->targetFunction = targetFunction;
        currentCodeBlockIndex = 0;
        return *this;
    }

    IROptimizer::SimulationStack::Item IROptimizer::lessThan(const SimulationStack::Item &item,
        const SimulationStack::Item &right) {
        if (item.hasPossibleValue && right.hasPossibleValue) {
            switch (item.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.intValue < right.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.deciValue < right.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.boolValue < right.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.charValue < right.possibleValue.charValue}, {}};
                }
                default:
                    panic(0, 0, "IROptimizer::lessThan(): Unsupported type for less than operation");
                    return {};
            }
        } else {
            return {compilerCtx->getBoolObjectType(), false, {}, item.contributedInstructions + right.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::lessThanOrEqual(const SimulationStack::Item &item,
        const SimulationStack::Item &right) {
        if (item.hasPossibleValue && right.hasPossibleValue) {
            switch (item.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.intValue <= right.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.deciValue <= right.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.boolValue <= right.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.charValue <= right.possibleValue.charValue}, {}};
                }
                default:
                    panic(0, 0, "IROptimizer::lessThanOrEqual(): Unsupported type for less than or equal operation");
                    return {};
            }
        } else {
            return {compilerCtx->getBoolObjectType(), false, {}, item.contributedInstructions + right.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::greaterThan(const SimulationStack::Item &item,
        const SimulationStack::Item &right) {
        if (item.hasPossibleValue && right.hasPossibleValue) {
            switch (item.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.intValue > right.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.deciValue > right.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.boolValue > right.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.charValue > right.possibleValue.charValue}, {}};
                }
                default:
                    panic(0, 0, "IROptimizer::greaterThan(): Unsupported type for greater than operation");
                    return {};
            }
        } else {
            return {compilerCtx->getBoolObjectType(), false, {}, item.contributedInstructions + right.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::greaterThanOrEqual(const SimulationStack::Item &item,
        const SimulationStack::Item &right) {
        if (item.hasPossibleValue && right.hasPossibleValue) {
            switch (item.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.intValue >= right.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.deciValue >= right.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.boolValue >= right.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.charValue >= right.possibleValue.charValue}, {}};
                }
                default:
                    panic(0, 0, "IROptimizer::greaterThanOrEqual(): Unsupported type for greater than or equal operation");
                    return {};
            }
        } else {
            return {compilerCtx->getBoolObjectType(), false, {}, item.contributedInstructions + right.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::equal(const SimulationStack::Item &item,
        const SimulationStack::Item &right) {
        if (item.hasPossibleValue && right.hasPossibleValue) {
            switch (item.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.intValue == right.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.deciValue == right.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.boolValue == right.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.charValue == right.possibleValue.charValue}, {}};
                }
                default:
                    panic(0, 0, "IROptimizer::equal(): Unsupported type for equal operation");
                    return {};
            }
        } else {
            return {compilerCtx->getBoolObjectType(), false, {}, item.contributedInstructions + right.contributedInstructions};
        }
    }

    IROptimizer::SimulationStack::Item IROptimizer::notEqual(const SimulationStack::Item &item,
        const SimulationStack::Item &right) {
        if (item.hasPossibleValue && right.hasPossibleValue) {
            switch (item.type->type) {
                case IRValueType::valueType::integerObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.intValue!= right.possibleValue.intValue}, {}};
                }
                case IRValueType::valueType::decimalObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.deciValue != right.possibleValue.deciValue}, {}};
                }
                case IRValueType::valueType::booleanObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.boolValue != right.possibleValue.boolValue}, {}};
                }
                case IRValueType::valueType::characterObject: {
                    return {compilerCtx->getBoolObjectType(), true, {item.possibleValue.charValue != right.possibleValue.charValue}, {}};
                }
                default:
                    panic(0, 0, "IROptimizer::notEqual(): Unsupported type for not equal operation");
                    return {};
            }
        } else {
            return {compilerCtx->getBoolObjectType(), false, {}, item.contributedInstructions + right.contributedInstructions};
        }
    }

    IROptimizer &IROptimizer::reduceRedundantConstantExpr() {
        for (yoi::indexT insIndex = 0; insIndex < targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray().size();++insIndex) {
            auto &ins = targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray()[insIndex];
            // std::cout << "ins " << insIndex << " " << wstring2string(ins.to_string()) << std::endl;
            // std::cout << currentCodeBlockIndex << " " << insIndex << " " <<  wstring2string(ins.to_string()) << std::endl;
            switch (ins.opcode) {
                case IR::Opcode::push_boolean: {
                    simulationStack.push(compilerCtx->getBoolObjectType(), {currentCodeBlockIndex,{insIndex}}, ins.operands[0].value.boolean);
                    break;
                }
                case IR::Opcode::push_integer: {
                    simulationStack.push(compilerCtx->getIntObjectType(), {currentCodeBlockIndex, {insIndex}}, ins.operands[0].value.integer);
                    break;
                }
                case IR::Opcode::push_decimal: {
                    simulationStack.push(compilerCtx->getDeciObjectType(), {currentCodeBlockIndex, {insIndex}}, ins.operands[0].value.decimal);
                    break;
                }
                case IR::Opcode::push_string: {
                    simulationStack.push(compilerCtx->getStrObjectType(), {currentCodeBlockIndex, {insIndex}}, ins.operands[0].value.stringLiteralIndex);
                    break;
                }
                case IR::Opcode::basic_cast_bool: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    // judge whether this is evaluable
                    if (value.hasPossibleValue) {
                        switch (value.type->type) {
                            case IRValueType::valueType::integerObject:
                                value.possibleValue.boolValue = value.possibleValue.intValue != 0;
                            break;
                            case IRValueType::valueType::decimalObject:
                                value.possibleValue.boolValue = value.possibleValue.deciValue != 0.0;
                            break;
                            case IRValueType::valueType::characterObject:
                                value.possibleValue.boolValue = value.possibleValue.charValue != 0;
                            break;
                            default:
                                break;
                        }
                        value.type = compilerCtx->getBoolObjectType();
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(value, insIndex);
                    } else {
                        simulationStack.push(compilerCtx->getBoolObjectType(),
                                             value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
                    }
                    break;
                }
                case IR::Opcode::basic_cast_int: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    if (value.hasPossibleValue) {
                        switch (value.type->type) {
                            case IRValueType::valueType::decimalObject:
                                value.possibleValue.intValue = static_cast<int64_t>(value.possibleValue.deciValue);
                            break;
                            case IRValueType::valueType::booleanObject:
                                value.possibleValue.intValue = value.possibleValue.boolValue ? 1 : 0;
                            break;
                            case IRValueType::valueType::characterObject:
                                value.possibleValue.intValue = static_cast<int64_t>(value.possibleValue.charValue);
                            break;
                            default:
                            break;
                        }
                        value.type = compilerCtx->getIntObjectType();
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(value, insIndex);
                    } else {
                        simulationStack.push(compilerCtx->getIntObjectType(),
                                             value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
                    }
                    break;
                }
                case IR::Opcode::basic_cast_deci: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    // std::cout << "simulate basic_cast_deci " << value.hasPossibleValue << std::endl;
                    if (value.hasPossibleValue) {
                        switch (value.type->type) {
                            case IRValueType::valueType::integerObject:
                                value.possibleValue.deciValue = static_cast<double>(value.possibleValue.intValue);
                            break;
                            case IRValueType::valueType::booleanObject:
                                value.possibleValue.deciValue = value.possibleValue.boolValue ? 1.0 : 0.0;
                            break;
                            case IRValueType::valueType::characterObject:
                                value.possibleValue.deciValue = static_cast<double>(value.possibleValue.charValue);
                            break;
                            default:
                            break;
                        }
                        value.type = compilerCtx->getDeciObjectType();
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(value, insIndex);
                    } else {
                        simulationStack.push(compilerCtx->getDeciObjectType(),
                                             value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
                    }
                    break;
                }
                case IR::Opcode::add: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = add(left, right);
                    // std::cout << "simulate add " << right.hasPossibleValue << " " << left.hasPossibleValue << " " << result.hasPossibleValue << std::endl;
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::sub: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = sub(left, right);
                    // std::cout << "simulate sub " << right.hasPossibleValue << " " << left.hasPossibleValue << " " << result.hasPossibleValue << std::endl;
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::mul: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = mul(left, right);
                    // std::cout << "simulate mul " << right.hasPossibleValue << " " << left.hasPossibleValue << " " << result.hasPossibleValue << std::endl;
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::div: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = div(left, right);
                    // std::cout << "simulate div " << right.hasPossibleValue << " " << left.hasPossibleValue << " " << result.hasPossibleValue << std::endl;
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::mod: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = mod(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::negate: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    // simulate
                    auto result = negate(value);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::bitwise_and: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = bitwiseAnd(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::bitwise_or: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = bitwiseOr(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::bitwise_xor: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = bitwiseXor(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::bitwise_not: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    // simulate
                    auto result = bitwiseNot(value);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::left_shift: {
                    auto value = simulationStack.peek(0);
                    auto shift = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = bitwiseShiftLeft(value, shift);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        insIndex = reduce(shift.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions + shift.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::right_shift: {
                    auto value = simulationStack.peek(0);
                    auto shift = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = bitwiseShiftRight(value, shift);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(value.contributedInstructions, insIndex);
                        insIndex = reduce(shift.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions + shift.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::less_than: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = lessThan(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::greater_than: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = greaterThan(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::less_equal: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = greaterThanOrEqual(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::greater_equal: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = greaterThanOrEqual(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::equal: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = equal(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::not_equal: {
                    auto right = simulationStack.peek(0);
                    auto left = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    // simulate
                    auto result = notEqual(left, right);
                    if (result.hasPossibleValue) {
                        insIndex = reduce(left.contributedInstructions, insIndex);
                        insIndex = reduce(right.contributedInstructions, insIndex);
                        ins = IR{IR::Opcode::nop, {}};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions);
                    }
                    break;
                }
                case IR::Opcode::load_local: {
                    if (auto it = variablesExtraInfo.find(ins.operands[0].value.symbolIndex); it != variablesExtraInfo.end()) {
                        // if exists, use the extra information
                        if (it->second.hasPossibleValue && it->second.possibleValue.contributedInstructions.codeBlockIndex == currentCodeBlockIndex) {
                            ins = IR{IR::Opcode::nop, {}};
                            insIndex = generatePushOp(it->second.possibleValue, insIndex);
                        } else {
                            // if we can't guess the value, we can't optimize it
                            // find the local variable definition
                            auto type = targetFunction->getVariableTable().get(ins.operands[0].value.symbolIndex);
                            simulationStack.push(type, {currentCodeBlockIndex, {insIndex}});
                            it->second.isReadAfterStore = true;
                        }
                    } else {
                        // well there does be a possibility that the variable will be not initialized, and this is parameter passing
                        // in this case, we can't optimize it
                        auto type = targetFunction->getVariableTable().get(ins.operands[0].value.symbolIndex);
                        simulationStack.push(type, {currentCodeBlockIndex, {insIndex}});
                        variablesExtraInfo[ins.operands[0].value.symbolIndex] = {false, true, {}};
                    }
                    break;
                }
                case IR::Opcode::store_local: {
                    // store the value to the variable
                    // if this is the twice or more times, we can ignore the previous store command and use the new value directly

                    // check the definition type and value type here
                    auto definitionType = targetFunction->getVariableTable().get(ins.operands[0].value.symbolIndex);
                    auto value = simulationStack.peek(0);

                    if(*definitionType != *value.type) {
                        // type mismatch, panic
                        panic(0, 0, "IROptimizer::reduceRedundantConstantExpr(): store_local: type mismatch");
                    }

                    simulationStack.pop();
                    if (auto it = variablesExtraInfo.find(ins.operands[0].value.symbolIndex); it != variablesExtraInfo.end() && it->second.hasPossibleValue && !it->second.isReadAfterStore && it->second.possibleValue.contributedInstructions.codeBlockIndex == currentCodeBlockIndex) {
                        // reduce previous redundant store
                        insIndex = reduce(it->second.possibleValue.contributedInstructions, insIndex);
                        variablesExtraInfo[ins.operands[0].value.symbolIndex] = {value.hasPossibleValue, false, value};
                    } else {
                        value.contributedInstructions = value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}};
                        variablesExtraInfo[ins.operands[0].value.symbolIndex] = {value.hasPossibleValue, false, value};
                    }
                    break;
                }
                case IR::Opcode::load_global: {
                    // as for global variables, we can't optimize it
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto type = compilerCtx->getImportedModule(moduleIndex)->globalVariables[ins.operands[0].value.symbolIndex];
                    simulationStack.push(type, {currentCodeBlockIndex, {insIndex}});
                    break;
                }
                case IR::Opcode::store_global: {
                    // check the definition type and value type here
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto definitionType = compilerCtx->getImportedModule(moduleIndex)->globalVariables[ins.operands[1].value.symbolIndex];
                    auto value = simulationStack.peek(0);

                    if(*definitionType != *value.type) {
                        // type mismatch, panic
                        panic(0, 0, "IROptimizer::reduceRedundantConstantExpr(): store_global: type mismatch");
                    }

                    simulationStack.pop();
                    break;
                }
                case IR::Opcode::load_member: {
                    // we can't optimize it
                    auto value = simulationStack.peek(0);
                    auto type = value.type->typeIndex;
                    auto targetModule = compilerCtx->getImportedModule(value.type->typeAffiliateModule);
                    auto structDef = targetModule->structTable[type];
                    auto memberIndex = ins.operands[0].value.symbolIndex;
                    auto memberDef = structDef->fieldTypes[memberIndex];
                    simulationStack.pop();
                    simulationStack.push(memberDef, value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}});
                    break;
                }
                case IR::Opcode::store_member: {
                    // we can't optimize it
                    auto value = simulationStack.peek(1);
                    auto type = simulationStack.peek(0).type->typeIndex;
                    auto structDef = irModule->structTable[type];
                    auto memberIndex = ins.operands[0].value.symbolIndex;
                    auto memberDef = structDef->fieldTypes[memberIndex];

                    if(*memberDef != *value.type) {
                        // type mismatch, panic
                        panic(0, 0, "IROptimizer::reduceRedundantConstantExpr(): store_member: type mismatch");
                    }

                    simulationStack.pop();
                    simulationStack.pop();
                    break;
                }
                case IR::Opcode::invoke: {
                    // we can't optimize it
                    // in case of which this got optimized in tempVar reduction, we set optimizable flag to false
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto function = compilerCtx->getImportedModule(moduleIndex)->functionTable[ins.operands[1].value.symbolIndex];
                    auto returnType = function->returnType;
                    auto argTypes = function->argumentTypes;
                    auto argCount = function->argumentTypes.size();
                    SimulationStack::Item::ContributedInstructionSet contributedInstructions = {currentCodeBlockIndex, {insIndex}, false};
                    for (int i = 0; i < argCount; i++) {
                        contributedInstructions = contributedInstructions + simulationStack.peek(0).contributedInstructions;
                        simulationStack.pop();
                    }
                    simulationStack.push(returnType, contributedInstructions);
                    break;
                }
                case IR::Opcode::invoke_imported: {
                    auto function = compilerCtx->getIRFFITable()->importedLibraries[ins.operands[0].value.symbolIndex].importedFunctionTable[ins.operands[1].value.symbolIndex];
                    auto returnType = function->returnType;
                    auto argTypes = function->argumentTypes;
                    auto argCount = function->argumentTypes.size();
                    for (int i = 0; i < argCount; i++) {
                        simulationStack.pop();
                    }
                    simulationStack.push(returnType, {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::invoke_virtual: {
                    auto argCount = ins.operands[2].value.symbolIndex;
                    for (int i = 0; i < argCount - 1; i++) {
                        simulationStack.pop();
                    }
                    auto returnType = compilerCtx->getImportedModule(simulationStack.peek(0).type->typeAffiliateModule)->interfaceTable[simulationStack.peek(0).type->typeIndex]->methodMap[ins.operands[1].value.symbolIndex]->returnType;
                    simulationStack.pop();
                    simulationStack.push(returnType, {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::new_struct: {
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto structDef = compilerCtx->getImportedModule(moduleIndex)->structTable[ins.operands[1].value.symbolIndex];
                    simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::structObject, moduleIndex, ins.operands[1].value.symbolIndex}), {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::new_interface: {
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto interfaceDef = compilerCtx->getImportedModule(moduleIndex)->interfaceTable[ins.operands[1].value.symbolIndex];
                    simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::interfaceObject, moduleIndex, ins.operands[1].value.symbolIndex}), {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::construct_interface_impl: {
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto interfaceImplDef = compilerCtx->getImportedModule(moduleIndex)->interfaceImplementationTable[ins.operands[1].value.symbolIndex];
                    auto returnType = managedPtr(IRValueType{IRValueType::valueType::interfaceObject, moduleIndex, ins.operands[1].value.symbolIndex});
                    auto argCount = interfaceImplDef->virtualMethodIndexMap.size();
                    for (int i = 0; i < argCount + 1; i++) {
                        simulationStack.pop();
                    }
                    simulationStack.push(returnType, {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::jump_if_true: {
                    auto condition = simulationStack.peek(0);
                    simulationStack.pop();
                    if (condition.hasPossibleValue) {
                        if (condition.possibleValue.boolValue) {
                            // if the condition is true, we can jump to the target block directly
                            // reduce redundant condition
                            insIndex = reduce(condition.contributedInstructions, insIndex);
                            ins.opcode = IR::Opcode::jump;
                        } else {
                            // if the condition is false, we can ignore the jump instruction
                            insIndex = reduce(condition.contributedInstructions, insIndex);
                            insIndex = reduce(SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}}, insIndex);
                            insIndex -= 1; // reduce the index by 1, to make sure we don't skip the next instruction, since we have reduced the instruction before
                        }
                    } else {
                        // if we can't guess the value, we can't optimize it, thus we do nothing
                    }
                    break;
                }
                case IR::Opcode::jump_if_false: {
                    auto condition = simulationStack.peek(0);
                    simulationStack.pop();
                    if (condition.hasPossibleValue) {
                        if (!condition.possibleValue.boolValue) {
                            // if the condition is false, we can jump to the target block directly
                            // reduce redundant condition
                            insIndex = reduce(condition.contributedInstructions, insIndex);
                            ins.opcode = IR::Opcode::jump;
                        } else {
                            // if the condition is true, we can ignore the jump instruction
                            insIndex = reduce(condition.contributedInstructions, insIndex);
                            insIndex = reduce(SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}}, insIndex);
                            insIndex -= 1; // reduce the index by 1, to make sure we don't skip the next instruction, since we have reduced the instruction before
                        }
                    } else {
                        // if we can't guess the value, we can't optimize it, thus we do nothing
                    }
                    break;
                }
                case IR::Opcode::ret: {
                    // just pop the return value
                    simulationStack.pop();
                    break;
                }
                case IR::Opcode::new_array_int: 
                case IR::Opcode::new_array_bool:
                case IR::Opcode::new_array_char:
                case IR::Opcode::new_array_deci:
                case IR::Opcode::new_array_str: {
                    // we can't optimize it
                    // dims in operands
                    std::shared_ptr<IRValueType> baseType;
                    switch (ins.opcode) {
                        case IR::Opcode::new_array_int:
                            baseType = compilerCtx->getIntObjectType();
                            break;
                        case IR::Opcode::new_array_bool:
                            baseType = compilerCtx->getBoolObjectType();
                            break;
                        case IR::Opcode::new_array_char:
                            baseType = compilerCtx->getCharObjectType();
                            break;
                        case IR::Opcode::new_array_deci:
                            baseType = compilerCtx->getDeciObjectType();
                            break;
                        case IR::Opcode::new_array_str:
                            baseType = compilerCtx->getStrObjectType();
                            break;
                        default:
                            break;
                    }

                    yoi::indexT size = 1;
                    yoi::vec<yoi::indexT> dims;
                    for (auto &dim : ins.operands) {
                        size *= dim.value.symbolIndex;
                        dims.push_back(dim.value.symbolIndex);
                    }
                    for (yoi::indexT i = 0; i < size; i++) {
                        simulationStack.pop();
                    }
                    simulationStack.push(managedPtr(baseType->getArrayType(dims)), {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::load_element: {
                    // we can't optimize it
                    auto index = simulationStack.peek(0);
                    auto array = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    simulationStack.push(managedPtr(array.type->getElementType()), array.contributedInstructions + index.contributedInstructions);
                    break;
                }
                case IR::Opcode::store_element: {
                    // we can't optimize it
                    auto index = simulationStack.peek(0);
                    auto value = simulationStack.peek(1);
                    auto array = simulationStack.peek(2);
                    simulationStack.pop();
                    simulationStack.pop();
                    simulationStack.pop();
                    break;
                }
                default: {
                    // pass
                    break;
                }
            }
        }
        return *this;
    }

    IROptimizer & IROptimizer::reduceRedundantTempVar() {
        for (auto &i : simulationStack.items) {
            reduce(i.contributedInstructions, 0);
        }
        return *this;
    }

    IROptimizer & IROptimizer::reduceRedundantNop() {
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            auto &codeBlock = targetFunction->codeBlock[i]->getIRArray();
            std::erase_if(codeBlock, [](const IR &ins) { return ins.opcode == IR::Opcode::nop; });
        }
        return *this;
    }

    IROptimizer & IROptimizer::reduceRedundantJump() {
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            auto &codeBlock = targetFunction->codeBlock[i]->getIRArray();
            if (codeBlock.empty()) {
                continue;
            }
            // if the above instruction of current jump is a jump, we can ignore the current jump
            for (auto it = codeBlock.begin() + 1; it != codeBlock.end();) {
                if ((it - 1)->opcode == IR::Opcode::jump && it->opcode == IR::Opcode::jump) {
                    it = codeBlock.erase(it);
                } else {
                    it++;
                }
            }
        }
        return *this;
    }

    IROptimizer & IROptimizer::reduceRedundantCodeAfterRet() {
        auto it = std::find_if(targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray().begin(), targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray().end(),
            [&](const IR &ins) { return ins.opcode == IR::Opcode::ret; });
        if (it != targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray().end()) {
            // if the last instruction is a ret, we can ignore all the instructions after it
            targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray().erase(it + 1, targetFunction->codeBlock[currentCodeBlockIndex]->getIRArray().end());
        }
        return *this;
    }

    IROptimizer& IROptimizer::controlFlowOptimization() {
        std::map<yoi::indexT, std::vector<indexT>> G; // graph
        std::map<yoi::indexT, std::vector<indexT>> reverseG; // record the predecessors of each block
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            G[i] = {};
            if (not reverseG.contains(i)) reverseG[i] = {};
            for (auto &ins : targetFunction->codeBlock[i]->getIRArray()) {
                switch (ins.opcode) {
                    case IR::Opcode::jump:
                    case IR::Opcode::jump_if_true:
                    case IR::Opcode::jump_if_false:
                        G[i].push_back(ins.operands[0].value.codeBlockIndex);
                        reverseG[ins.operands[0].value.codeBlockIndex].push_back(i);
                        break;
                    default:
                        break;
                }
            }
        }
        // remove the blocks that have no predecessors
        for (auto it = reverseG.begin(); it != reverseG.end(); it++) {
            if (it->second.empty() && it->first != 0) {
                targetFunction->codeBlock[it->first]->getIRArray() = {};
                // do not erase it cuz we need to keep the index of the block
            }
        }
        // identify the out block
        for (auto it = G.begin(); it != G.end(); it++) {
            if (it->second.empty()) {
                auto &targetBlock = targetFunction->codeBlock[it->first];
                // if the block has no successor, it's the out block
                if (targetBlock->getIRArray().empty()) {
                    if (reverseG[it->first].empty()) {
                        // no predecessor, and no successor, and no instructions, it's empty block, do nothing
                    } else {
                        // has predecessor, but no successor, it's the out block but with empty instructions
                        warning(0, 0, "IROptimizer::controlFlowOptimization(): function " + wstring2string(targetFunction->name) + " has no return instruction in out block");
                    }
                    continue;
                }
                if (targetBlock->getIRArray().back().opcode != IR::Opcode::ret && targetBlock->getIRArray().back().opcode != IR::Opcode::ret_none) {
                    // there's no return instruction, add a ret instruction at the end of the block if it returns none
                    if (targetFunction->returnType->type == IRValueType::valueType::none) {
                        targetBlock->getIRArray().push_back(IR{IR::Opcode::ret_none, {}});
                    } else {
                        warning(0, 0, "IROptimizer::controlFlowOptimization(): function " + wstring2string(targetFunction->name) + " has no return instruction in out block");
                    }
                }
            }
        }
        return *this;
    }

    IROptimizer & IROptimizer::doOptimizationForCurrentFunction() {
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            currentCodeBlockIndex = i;
            this->reduceRedundantConstantExpr().reduceRedundantTempVar().reduceRedundantCodeAfterRet();
            // clear the information we gathered as when jump back happens, the information is no longer valid
            // variablesExtraInfo.clear();
        }
        this->reduceRedundantNop().reduceRedundantJump().controlFlowOptimization().reduceEmptyCodeBlock();
        return *this;
    }
    IROptimizer &IROptimizer::reduceEmptyCodeBlock() {
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            if (targetFunction->codeBlock[i]->getIRArray().empty()) {
                targetFunction->codeBlock.erase(targetFunction->codeBlock.begin() + i);
                i--;
            }
        }
        return *this;
    }
} // namespace yoi
