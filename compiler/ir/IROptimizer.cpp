//
// Created by Jerry Chou on 10/2/2024.
//

#include "IROptimizer.hpp"
#include "compiler/ir/IR.h"
#include "share/def.hpp"

#include <cmath>
#include <cstdint>
#include <memory>
#include <queue>

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
        result.optimizable = result.optimizable && other.optimizable;
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
            targetFunction->codeBlock[contributedInstructions.codeBlockIndex]->getIRArray()[i] = {IR::Opcode::nop, {}, targetFunction->codeBlock[contributedInstructions.codeBlockIndex]->getIRArray()[i].debugInfo};
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
                                 IR::Opcode::push_integer, {IROperand{IROperand::operandType::integer, {item.possibleValue.intValue}}}, IRArr[index].debugInfo});
                break;
            case IRValueType::valueType::decimalObject:
                IRArr.insert(IRArr.begin() + index + 1, IR{
                                 IR::Opcode::push_decimal, {IROperand{IROperand::operandType::decimal, {item.possibleValue.deciValue}}}, IRArr[index].debugInfo});
                break;
            case IRValueType::valueType::booleanObject:
                IRArr.insert(IRArr.begin() + index + 1, IR{
                                 IR::Opcode::push_boolean, {{IROperand::operandType::boolean, IROperand::operandValue{item.possibleValue.boolValue}}}, IRArr[index].debugInfo});
                break;
            case IRValueType::valueType::stringObject:
                IRArr.insert(IRArr.begin() + index + 1, IR{
                                 IR::Opcode::push_string, {{IROperand::operandType::stringLiteral, IROperand::operandValue{item.possibleValue.stringConstIndex}}}, IRArr[index].debugInfo});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
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
                        ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        simulationStack.push(result.type, value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions + shift.contributedInstructions  + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, value.contributedInstructions + shift.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                        ins = ins = IR{IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp(result, insIndex);
                    } else {
                        // lost information, push back
                        simulationStack.push(result.type, left.contributedInstructions + right.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
                    }
                    break;
                }
                case IR::Opcode::load_local: {
                    if (auto it = variablesExtraInfo.find(ins.operands[0].value.symbolIndex); it != variablesExtraInfo.end()) {
                        // if exists, use the extra information
                        if (it->second.hasPossibleValue && it->second.possibleValue.contributedInstructions.codeBlockIndex == currentCodeBlockIndex) {
                            // inherit the possible value onto the stack
                            simulationStack.push(it->second.possibleValue.type, SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}});
                            it->second.isReadAfterStore = false;
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
                    auto returnType = simulationStack.peek(0).type;
                    simulationStack.pop();
                    simulationStack.pop();
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
                case yoi::IR::Opcode::new_array_struct:
                case yoi::IR::Opcode::new_array_interface: {
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto typeIndex = ins.operands[1].value.symbolIndex;
                    yoi::indexT size = 1;
                    yoi::vec<yoi::indexT> dims;

                    auto baseType = managedPtr(IRValueType{ins.opcode == yoi::IR::Opcode::new_array_struct ? IRValueType::valueType::structObject : IRValueType::valueType::interfaceObject, moduleIndex, typeIndex});

                    for (yoi::indexT i = 2; i < ins.operands.size(); i++) {
                        size *= ins.operands[i].value.symbolIndex;
                        dims.push_back(ins.operands[i].value.symbolIndex);
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
                    simulationStack.push(
                        managedPtr(array.type->getElementType()),
                        array.contributedInstructions + index.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}, false});
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
                case IR::Opcode::direct_assign: {
                    auto rhs = simulationStack.peek(0);
                    auto lhs = simulationStack.peek(1);
                    simulationStack.pop();
                    simulationStack.pop();
                    simulationStack.push(lhs.type, lhs.contributedInstructions + rhs.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::array_length: {
                    auto array = simulationStack.peek(0);
                    simulationStack.pop();
                    if (array.type->isArrayType()) {
                        yoi::indexT size = 1;
                        for (auto &dim : array.type->dimensions) {
                            size *= dim;
                        }
                        // reduce 
                        insIndex = reduce(array.contributedInstructions, insIndex);
                        // generate push op
                        ins = {IR::Opcode::nop, {}, ins.debugInfo};
                        insIndex = generatePushOp({compilerCtx->getIntObjectType(), true, static_cast<int64_t>(size)}, insIndex);
                        break;
                    } else if (array.type->isDynamicArrayType()) {
                        // not even optimizable
                        simulationStack.push(compilerCtx->getIntObjectType(), array.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}, false});
                        break;
                    }
                }
                case IR::Opcode::pop:{
                    auto rhs = simulationStack.peek(0);
                    if (rhs.contributedInstructions.optimizable) {
                        insIndex = reduce(rhs.contributedInstructions, insIndex);
                        ins = {IR::Opcode::nop, {}, ins.debugInfo};
                    }
                    simulationStack.pop();
                    break;
                }
                case IR::Opcode::typeid_int:
                case IR::Opcode::typeid_bool:
                case IR::Opcode::typeid_char:
                case IR::Opcode::typeid_deci:
                case IR::Opcode::typeid_str:
                case IR::Opcode::typeid_struct:
                case IR::Opcode::typeid_interface: {
                    simulationStack.push(compilerCtx->getIntObjectType(), {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::dyn_cast_int:
                case IR::Opcode::dyn_cast_bool:
                case IR::Opcode::dyn_cast_deci:
                case IR::Opcode::dyn_cast_char:
                case IR::Opcode::dyn_cast_str: {
                    std::shared_ptr<IRValueType> value_type;
                    switch (ins.opcode) {
                        case IR::Opcode::dyn_cast_int:
                            value_type = compilerCtx->getIntObjectType();
                            break;
                        case IR::Opcode::dyn_cast_bool:
                            value_type = compilerCtx->getBoolObjectType();
                            break;
                        case IR::Opcode::dyn_cast_char:
                            value_type = compilerCtx->getCharObjectType();
                            break;
                        case IR::Opcode::dyn_cast_deci:
                            value_type = compilerCtx->getDeciObjectType();
                            break;
                        case IR::Opcode::dyn_cast_str:
                            value_type = compilerCtx->getStrObjectType();
                            break;
                        default:
                            break;
                    }
                    simulationStack.pop();
                    simulationStack.push(value_type, {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::dyn_cast_struct: {
                    auto structType = managedPtr(IRValueType{IRValueType::valueType::structObject, ins.operands[0].value.symbolIndex, ins.operands[1].value.symbolIndex});
                    simulationStack.pop();
                    simulationStack.push(structType, {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::pointer_cast: {
                    simulationStack.pop();
                    simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::pointerObject}), {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::push_null: {
                    simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::pointerObject}), {currentCodeBlockIndex, {insIndex}, true});
                    break;
                }
                case IR::Opcode::new_dynamic_array_int: 
                case IR::Opcode::new_dynamic_array_bool:
                case IR::Opcode::new_dynamic_array_char:
                case IR::Opcode::new_dynamic_array_deci:
                case IR::Opcode::new_dynamic_array_str:
                case IR::Opcode::new_dynamic_array_struct:
                case IR::Opcode::new_dynamic_array_interface: {
                    std::shared_ptr<IRValueType> baseType;
                    switch (ins.opcode) {
                        case IR::Opcode::new_dynamic_array_int:
                            baseType = compilerCtx->getIntObjectType();
                            break;
                        case IR::Opcode::new_dynamic_array_bool:
                            baseType = compilerCtx->getBoolObjectType();
                            break;
                        case IR::Opcode::new_dynamic_array_char:
                            baseType = compilerCtx->getCharObjectType();
                            break;
                        case IR::Opcode::new_dynamic_array_deci:
                            baseType = compilerCtx->getDeciObjectType();
                            break;
                        case IR::Opcode::new_dynamic_array_str:
                            baseType = compilerCtx->getStrObjectType();
                            break;
                        case IR::Opcode::new_dynamic_array_interface:
                            baseType = managedPtr(IRValueType{
                                IRValueType::valueType::interfaceObject, 
                                ins.operands[0].value.symbolIndex,
                                ins.operands[1].value.symbolIndex
                            });
                            break;
                        case IR::Opcode::new_dynamic_array_struct:
                            baseType = managedPtr(IRValueType{
                                IRValueType::valueType::structObject, 
                                ins.operands[0].value.symbolIndex,
                                ins.operands[1].value.symbolIndex
                            });
                            break;
                        default:
                            break;
                    }

                    for (yoi::indexT i = 0; i < ins.operands.back().value.symbolIndex; i++) {
                        simulationStack.pop();
                    }
                    simulationStack.push(managedPtr(baseType->getDynamicArrayType()), {currentCodeBlockIndex, {insIndex}, false});
                    break;
                }
                case IR::Opcode::interfaceof: {
                    // pop two values and push one boolean value
                    auto interfaceType = simulationStack.peek(0).type;
                    auto objectType = simulationStack.peek(1).type;
                    simulationStack.pop();
                    simulationStack.pop();
                    simulationStack.push(
                        compilerCtx->getBoolObjectType(),
                        {currentCodeBlockIndex, {insIndex}, false}
                    );
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
                        panic(targetFunction->debugInfo.line, targetFunction->debugInfo.column, "IROptimizer::controlFlowOptimization(): function " + wstring2string(targetFunction->name) + " has no return instruction in out block");
                    }
                    continue;
                }
                if (targetBlock->getIRArray().back().opcode != IR::Opcode::ret && targetBlock->getIRArray().back().opcode != IR::Opcode::ret_none) {
                    // there's no return instruction, add a ret instruction at the end of the block if it returns none
                    if (targetFunction->returnType->type == IRValueType::valueType::none) {
                        targetBlock->getIRArray().push_back(IR{IR::Opcode::ret_none, {}, targetBlock->getIRArray().back().debugInfo});
                    } else {
                        panic(targetFunction->debugInfo.line, targetFunction->debugInfo.column, "IROptimizer::controlFlowOptimization(): function " + wstring2string(targetFunction->name) + " has no return instruction in out block");
                    }
                }
            }
        }
        return *this;
    }

    IROptimizer & IROptimizer::doOptimizationForCurrentFunction() {
        std::map<indexT, std::vector<indexT>> successors;
        std::map<indexT, std::vector<indexT>> predecessors;
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            if (successors.find(i) == successors.end()) successors[i] = {};
            for (auto &ins : targetFunction->codeBlock[i]->getIRArray()) {
                switch (ins.opcode) {
                    case IR::Opcode::jump: {
                        indexT target = ins.operands[0].value.codeBlockIndex;
                        successors[i].push_back(target);
                        predecessors[target].push_back(i);
                        break;
                    }
                    case IR::Opcode::jump_if_true:
                    case IR::Opcode::jump_if_false: {
                        indexT target = ins.operands[0].value.codeBlockIndex;
                        successors[i].push_back(target); // Branch target
                        predecessors[target].push_back(i);
                        if (i + 1 < targetFunction->codeBlock.size()) {
                            successors[i].push_back(i + 1); // Fallthrough
                            predecessors[i + 1].push_back(i);
                        }
                        break;
                    }
                    case IR::Opcode::ret:
                    case IR::Opcode::ret_none:
                        // This block is a graph sink.
                        break;
                    default:
                        break;
                }
            }
            // Add implicit fallthrough for non-terminating blocks
            if (!targetFunction->codeBlock[i]->getIRArray().empty()) {
                auto& lastIns = targetFunction->codeBlock[i]->getIRArray().back();
                bool isTerminator = (lastIns.opcode == IR::Opcode::jump ||
                                    lastIns.opcode == IR::Opcode::jump_if_false ||
                                    lastIns.opcode == IR::Opcode::jump_if_true ||
                                    lastIns.opcode == IR::Opcode::ret ||
                                    lastIns.opcode == IR::Opcode::ret_none);

                if (!isTerminator && (i + 1 < targetFunction->codeBlock.size())) {
                    successors[i].push_back(i + 1);
                    predecessors[i + 1].push_back(i);
                }
            }
        }

        // data flow analyse
        std::map<indexT, AnalysisState> blockInStates;
        std::map<indexT, AnalysisState> blockOutStates;
        std::queue<indexT> worklist;

        // start with the entry block.
        worklist.push(0);
        blockInStates[0] = AnalysisState{}; // Entry state is empty.

        while (!worklist.empty()) {
            indexT currentBlockIdx = worklist.front();
            worklist.pop();

            // merge predecessors' out-states to get the in-state for the current block.
            AnalysisState inState;
            if (predecessors.count(currentBlockIdx) > 0) {
                for (indexT predIdx : predecessors[currentBlockIdx]) {
                    // Get the last calculated out-state of the predecessor.
                    inState = mergeStates(inState, blockOutStates[predIdx]);
                }
            }
            blockInStates[currentBlockIdx] = inState;

            // analyze the current block to get its new out-state.
            AnalysisState newOutState = analyzeBlock(currentBlockIdx, inState);

            // if the out-state has changed, we need to re-process successors.
            if (blockOutStates.find(currentBlockIdx) == blockOutStates.end() || 
                blockOutStates[currentBlockIdx] != newOutState) {
                blockOutStates[currentBlockIdx] = newOutState;
                if (successors.count(currentBlockIdx) > 0) {
                    for (indexT succIdx : successors[currentBlockIdx]) {
                        worklist.push(succIdx);
                    }
                }
            }
        }

        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            if (blockInStates.count(i) > 0) { // Only transform reachable blocks
                transformBlock(i, blockInStates[i]);
            } else {
                // unreachable, clear it.
                targetFunction->codeBlock[i]->getIRArray().clear();
            }
        }
        
        this->reduceRedundantNop().reduceRedundantJump().controlFlowOptimization().reduceEmptyCodeBlock();
        
        this->performNullableCheck();
        this->performRawCheck();
        return *this;
    }
    
    IROptimizer &IROptimizer::reduceEmptyCodeBlock() {
        const auto originalSize = targetFunction->codeBlock.size();
        if (originalSize == 0) {
            return *this;
        }

        // identify which blocks to keep and create an old-to-new index mapping
        std::map<indexT, indexT> oldToNewMap;
        std::vector<bool> isBlockKept(originalSize);
        indexT newIndexCounter = 0;
        for (indexT oldIndex = 0; oldIndex < originalSize; ++oldIndex) {
            if (!targetFunction->codeBlock[oldIndex]->getIRArray().empty()) {
                isBlockKept[oldIndex] = true;
                oldToNewMap[oldIndex] = newIndexCounter++;
            } else {
                isBlockKept[oldIndex] = false;
            }
        }

        // ff no blocks were empty, no remapping is needed.
        if (newIndexCounter == originalSize) {
            return *this;
        }

        // create a fallthrough map for jumps that might target empty blocks
        // This map redirects an index to the next non-empty block's original index.
        std::vector<indexT> fallthroughTargetMap(originalSize);
        const indexT noFallthroughSentinel = originalSize;
        indexT nextNonEmptyBlockIndex = noFallthroughSentinel;

        // iterate backwards to find the next non-empty block for each position
        for (indexT oldIndex = originalSize; oldIndex-- > 0;) {
            if (isBlockKept[oldIndex]) {
                nextNonEmptyBlockIndex = oldIndex;
            }
            fallthroughTargetMap[oldIndex] = nextNonEmptyBlockIndex;
        }

        std::vector<std::shared_ptr<IRCodeBlock>> newCodeBlocks;
        newCodeBlocks.reserve(newIndexCounter);

        for (indexT oldIndex = 0; oldIndex < originalSize; ++oldIndex) {
            if (isBlockKept[oldIndex]) {
                auto& block = targetFunction->codeBlock[oldIndex];
                
                for (auto& ins : block->getIRArray()) {
                    switch (ins.opcode) {
                        case IR::Opcode::jump:
                        case IR::Opcode::jump_if_true:
                        case IR::Opcode::jump_if_false: {
                            indexT originalTarget = ins.operands[0].value.codeBlockIndex;

                            if (originalTarget >= originalSize) {
                                warning(0, 0, "IROptimizer::reduceEmptyCodeBlock(): Invalid jump target " +
                                        std::to_string(originalTarget) + " in block " + std::to_string(oldIndex) +
                                        " of function " + wstring2string(targetFunction->name) + ". Replacing with NOP.");
                                ins.opcode = IR::Opcode::nop;
                                ins.operands.clear();
                                continue;
                            }

                            indexT resolvedOldTarget = fallthroughTargetMap[originalTarget];
                            
                            if (resolvedOldTarget == noFallthroughSentinel) {
                                // This jump targets a region of empty blocks at the end of the function.
                                // This control flow path becomes undefined after removal.
                                warning(0, 0, "IROptimizer::reduceEmptyCodeBlock(): Jump in block " +
                                        std::to_string(oldIndex) + " of function " + wstring2string(targetFunction->name) +
                                        " targets an empty region at the function's end. This control flow path is being removed.");
                                ins.opcode = IR::Opcode::nop;
                                ins.operands.clear();
                            } else {
                                // We found a valid non-empty target block. Convert its old index to the new one.
                                indexT newTarget = oldToNewMap.at(resolvedOldTarget);
                                ins.operands[0].value.codeBlockIndex = newTarget;
                            }
                            break;
                        }
                        default:
                            // not a jump instruction, no action needed.
                            break;
                    }
                }
                newCodeBlocks.push_back(block);
            }
        }

        targetFunction->codeBlock = std::move(newCodeBlocks);

        return *this;
    }
    
    bool AnalysisState::operator!=(const AnalysisState &other) const {
        if (stack.items.size() != other.stack.items.size() || variableStates.size() != other.variableStates.size()) {
            return true;
        }

        for (size_t i = 0; i < stack.items.size(); ++i) {
            const auto &item1 = stack.items[i];
            const auto &item2 = other.stack.items[i];
            if (item1.hasPossibleValue != item2.hasPossibleValue)
                return true;
            if (item1.hasPossibleValue) {
                if (item1.possibleValue.intValue != item2.possibleValue.intValue)
                    return true;
            }
            // Also compare attributes for attribute passes
            if (*item1.type != *item2.type || item1.type->attributes != item2.type->attributes) {
                return true;
            }
        }

        for (const auto &[varIdx, info1] : variableStates) {
            auto it = other.variableStates.find(varIdx);
            if (it == other.variableStates.end())
                return true;
            const auto &info2 = it->second;
            if (info1.hasPossibleValue != info2.hasPossibleValue)
                return true;
            if (info1.hasPossibleValue) {
                if (info1.possibleValue.possibleValue.intValue != info2.possibleValue.possibleValue.intValue)
                    return true;
            }
            if ((info1.possibleValue.type && info2.possibleValue.type) && (*info1.possibleValue.type != *info2.possibleValue.type || info1.possibleValue.type->attributes != info2.possibleValue.type->attributes)) {
                return true;
            }
        }

        return false;
    }

    AnalysisState IROptimizer::analyzeBlock(indexT blockIndex, const AnalysisState &inState) {
        AnalysisState currentState = inState;
        simulationStack = currentState.stack;
        variablesExtraInfo = currentState.variableStates;
        currentCodeBlockIndex = blockIndex;

        for (yoi::indexT insIndex = 0; insIndex < targetFunction->codeBlock[blockIndex]->getIRArray().size(); insIndex++) {
            const auto &ins = targetFunction->codeBlock[blockIndex]->getIRArray()[insIndex];
            handleInstruction(ins, insIndex, currentCodeBlockIndex);
        }

        // Return the final state after all instructions are processed.
        return {simulationStack, variablesExtraInfo};
    }

    AnalysisState mergeStates(const AnalysisState &s1, const AnalysisState &s2) {
        if (s1.stack.items.empty() && s1.variableStates.empty())
            return s2;
        if (s2.stack.items.empty() && s2.variableStates.empty())
            return s1;

        // It's an error if stack sizes don't match at a merge point.
        // The IR is likely invalid.
        if (s1.stack.items.size() != s2.stack.items.size()) {
            panic(0, 0, "IROptimizer: Incompatible stack depths at merge point.");
        }

        AnalysisState mergedState;

        // Merge Stacks
        for (size_t i = 0; i < s1.stack.items.size(); ++i) {
            const auto &item1 = s1.stack.items[i];
            const auto &item2 = s2.stack.items[i];

            auto mergedItem = IROptimizer::SimulationStack::Item{
                item1.type, false, {}, item1.contributedInstructions + item2.contributedInstructions};

            if (item1.hasPossibleValue && item2.hasPossibleValue) {
                if (item1.type->type == item2.type->type &&
                    item1.type->isBasicType() &&
                    item1.possibleValue.intValue == item2.possibleValue.intValue) {
                    mergedItem.hasPossibleValue = true;
                    mergedItem.possibleValue = item1.possibleValue;
                }
            }
            mergedState.stack.push(mergedItem);
        }

        // Merge Variables
        // Create a set of all variable keys from both maps
        std::set<indexT> allVarKeys;
        for (const auto &[key, val] : s1.variableStates)
            allVarKeys.insert(key);
        for (const auto &[key, val] : s2.variableStates)
            allVarKeys.insert(key);

        for (const auto &key : allVarKeys) {
            auto it1 = s1.variableStates.find(key);
            auto it2 = s2.variableStates.find(key);

            if (it1 != s1.variableStates.end() && it2 != s2.variableStates.end()) {
                // Variable exists in both paths
                const auto &v1 = it1->second;
                const auto &v2 = it2->second;

                // Default to an unknown value
                auto mergedInfo = IROptimizer::VariablesExtraInfo{false, true, {}};

                if (v1.hasPossibleValue && v2.hasPossibleValue) {
                    // If known and identical, preserve value
                    if (v1.possibleValue.possibleValue.intValue == v2.possibleValue.possibleValue.intValue) {
                        mergedInfo.hasPossibleValue = true;
                        mergedInfo.possibleValue = v1.possibleValue;
                    }
                }
                mergedState.variableStates[key] = mergedInfo;

            } else {
                mergedState.variableStates[key] = {false, true, {}};
            }
        }

        return mergedState;
    }

    void IROptimizer::transformBlock(indexT blockIndex, const AnalysisState &inState) {
        simulationStack = inState.stack;
        variablesExtraInfo = inState.variableStates;
        currentCodeBlockIndex = blockIndex;

        reduceRedundantConstantExpr();
        reduceRedundantCodeAfterRet();
    }
    
    // ===================================================================================
    // ==                             Nullable Check Pass                             ==
    // ===================================================================================

    IROptimizer &IROptimizer::performNullableCheck() {
        std::map<indexT, std::vector<indexT>> successors;
        std::map<indexT, std::vector<indexT>> predecessors;
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            if (successors.find(i) == successors.end()) successors[i] = {};
            if (predecessors.find(i) == predecessors.end()) predecessors[i] = {};
            if (!targetFunction->codeBlock[i]->getIRArray().empty()) {
                auto& lastIns = targetFunction->codeBlock[i]->getIRArray().back();
                bool isTerminator = (lastIns.opcode == IR::Opcode::jump ||
                                    lastIns.opcode == IR::Opcode::jump_if_false ||
                                    lastIns.opcode == IR::Opcode::jump_if_true ||
                                    lastIns.opcode == IR::Opcode::ret ||
                                    lastIns.opcode == IR::Opcode::ret_none);
                if (!isTerminator && (i + 1 < targetFunction->codeBlock.size())) {
                    successors[i].push_back(i + 1);
                    predecessors[i + 1].push_back(i);
                }
            }
            for (auto &ins : targetFunction->codeBlock[i]->getIRArray()) {
                 switch (ins.opcode) {
                    case IR::Opcode::jump: {
                        indexT target = ins.operands[0].value.codeBlockIndex;
                        successors[i].push_back(target);
                        predecessors[target].push_back(i);
                        break;
                    }
                    case IR::Opcode::jump_if_true:
                    case IR::Opcode::jump_if_false: {
                        indexT target = ins.operands[0].value.codeBlockIndex;
                        successors[i].push_back(target);
                        if (i + 1 < targetFunction->codeBlock.size()) successors[i].push_back(i + 1);
                        predecessors[target].push_back(i);
                        if (i + 1 < targetFunction->codeBlock.size()) predecessors[i + 1].push_back(i);
                        break;
                    }
                    default: break;
                }
            }
        }
    
        std::map<indexT, AnalysisState> blockInStates;
        std::map<indexT, AnalysisState> blockOutStates;
        std::queue<indexT> worklist;
        
        AnalysisState entryState;
        // Rule 3: Function parameters are nullable
        for(yoi::indexT i = 0; i < targetFunction->argumentTypes.size(); ++i) {
            auto varType = std::make_shared<IRValueType>(*targetFunction->variableTable.get(i));
            varType->addAttribute(IRValueType::ValueAttr::Nullable);
            entryState.variableStates[i] = {false, true, {varType, false, {}}};
        }

        worklist.push(0);
        blockInStates[0] = entryState;

        while (!worklist.empty()) {
            indexT currentBlockIdx = worklist.front();
            worklist.pop();

            AnalysisState inState = currentBlockIdx == 0 ? blockInStates[0] : AnalysisState{};
            if (predecessors.count(currentBlockIdx) > 0) {
                for (indexT predIdx : predecessors[currentBlockIdx]) {
                    if (blockOutStates.count(predIdx) > 0)
                        inState = mergeStatesForNullable(inState, blockOutStates[predIdx]);
                }
            }
            blockInStates[currentBlockIdx] = inState;

            AnalysisState newOutState = analyzeBlockForNullable(currentBlockIdx, inState);

            if (blockOutStates.find(currentBlockIdx) == blockOutStates.end() || blockOutStates[currentBlockIdx] != newOutState) {
                blockOutStates[currentBlockIdx] = newOutState;
                if (successors.count(currentBlockIdx) > 0) {
                    for (indexT succIdx : successors[currentBlockIdx]) {
                        worklist.push(succIdx);
                    }
                }
            }
        }

        // Apply results
        for(const auto& [varIndex, varType] : targetFunction->variableTable.getReversedVariableNameMap()) {
            bool isNullable = false;
            for(const auto& [blockIndex, outState] : blockOutStates) {
                if(outState.variableStates.count(varIndex) && outState.variableStates.at(varIndex).possibleValue.type->hasAttribute(IRValueType::ValueAttr::Nullable)) {
                    isNullable = true;
                    break;
                }
            }
            if(isNullable) {
                targetFunction->variableTable.get(varIndex)->addAttribute(IRValueType::ValueAttr::Nullable);
            }
        }
        
        return *this;
    }
    
    AnalysisState IROptimizer::analyzeBlockForNullable(indexT blockIndex, const AnalysisState &inState) {
        simulationStack = inState.stack;
        variablesExtraInfo.clear();
        for(const auto& [idx, state] : inState.variableStates) {
            variablesExtraInfo[idx] = {false, true, {std::make_shared<IRValueType>(*state.possibleValue.type), false, {}}};
        }

        auto getVarType = [&](indexT varIndex) {
            if (!variablesExtraInfo.count(varIndex)) {
                auto originalType = targetFunction->variableTable.get(varIndex);
                variablesExtraInfo[varIndex] = {false, true, {std::make_shared<IRValueType>(*originalType), false, {}}};
            }
            return variablesExtraInfo.at(varIndex).possibleValue.type;
        };

        for (const auto &ins : targetFunction->codeBlock[blockIndex]->getIRArray()) {
            switch(ins.opcode) {
                // Instructions that push non-nullable values
                case IR::Opcode::push_integer: simulationStack.push(compilerCtx->getIntObjectType(), {}); break;
                case IR::Opcode::push_decimal: simulationStack.push(compilerCtx->getDeciObjectType(), {}); break;
                case IR::Opcode::push_boolean: simulationStack.push(compilerCtx->getBoolObjectType(), {}); break;
                case IR::Opcode::push_string: simulationStack.push(compilerCtx->getStrObjectType(), {}); break;
                // `push_null` is the source of nullability
                case IR::Opcode::push_null: {
                    auto type = std::make_shared<IRValueType>(IRValueType::valueType::null);
                    type->addAttribute(IRValueType::ValueAttr::Nullable);
                    simulationStack.push(type, {});
                    break;
                }
                // Data movement
                case IR::Opcode::store_local: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    auto varType = getVarType(ins.operands[0].value.symbolIndex);
                    varType->attributes = value.type->attributes; // Rule 1: Direct propagation
                    break;
                }
                case IR::Opcode::load_local: {
                    auto varType = getVarType(ins.operands[0].value.symbolIndex);
                    simulationStack.push(std::make_shared<IRValueType>(*varType), {});
                    break;
                }
                case IR::Opcode::load_member: {
                    auto structObj = simulationStack.peek(0);
                    simulationStack.pop(); 
                    auto structDef = compilerCtx->getImportedModule(structObj.type->typeAffiliateModule)->structTable[structObj.type->typeIndex];
                    auto memberIndex = ins.operands[0].value.symbolIndex;
                    auto memberType = std::make_shared<IRValueType>(*structDef->fieldTypes[memberIndex]);
                    memberType->addAttribute(IRValueType::ValueAttr::Nullable);
                    simulationStack.push(memberType, {});
                    break;
                }
                case IR::Opcode::store_member: {
                    simulationStack.pop();
                    simulationStack.pop();
                    break;
                }
                // Array rules
                case IR::Opcode::load_element: {
                    simulationStack.pop(); // index
                    auto array = simulationStack.peek(0);
                    simulationStack.pop();
                    auto elemType = std::make_shared<IRValueType>(array.type->getElementType());
                    if (array.type->isBasicType() && (array.type->isArrayType() || array.type->isDynamicArrayType())) {
                        elemType->removeAttribute(IRValueType::ValueAttr::Nullable); // Rule 2 special case
                    } else {
                        elemType->addAttribute(IRValueType::ValueAttr::Nullable);
                    }
                    simulationStack.push(elemType, {});
                    break;
                }
                case IR::Opcode::store_element: {
                    simulationStack.pop(); // value
                    simulationStack.pop(); // index
                    simulationStack.pop(); // array
                    // Rule 1 special case: No propagation
                    break;
                }
                // External control flow rules
                case IR::Opcode::invoke: {
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto funcIndex = ins.operands[1].value.symbolIndex;
                    auto func = compilerCtx->getImportedModule(moduleIndex)->functionTable[funcIndex];
                    for(size_t i = 0; i < func->argumentTypes.size(); ++i) simulationStack.pop();
                    auto returnType = std::make_shared<IRValueType>(*func->returnType);
                    if (returnType->type != IRValueType::valueType::none) {
                        returnType->addAttribute(IRValueType::ValueAttr::Nullable);
                    }
                    simulationStack.push(returnType, {});
                    break;
                }
                case IR::Opcode::invoke_virtual: {
                    auto argCount = ins.operands[2].value.symbolIndex;
                    for(size_t i = 0; i < argCount; ++i) if(!simulationStack.items.empty()) simulationStack.pop();
                    auto returnType = std::make_shared<IRValueType>(IRValueType::valueType::pointerObject); // Placeholder
                    returnType->addAttribute(IRValueType::ValueAttr::Nullable); // Rule 3
                    simulationStack.push(returnType, {});
                    break;
                }
                case IR::Opcode::invoke_imported: {
                    auto argCount = ins.operands[2].value.symbolIndex;
                    for(size_t i = 0; i < argCount; ++i) if(!simulationStack.items.empty()) simulationStack.pop();
                    auto returnType = std::make_shared<IRValueType>(IRValueType::valueType::pointerObject); // Placeholder
                    returnType->addAttribute(IRValueType::ValueAttr::Nullable); // Rule 3
                    simulationStack.push(returnType, {});
                    break;
                }
                // Binary Ops: Result is nullable if either operand is.
                case IR::Opcode::add: case IR::Opcode::sub: case IR::Opcode::mul: case IR::Opcode::div:
                case IR::Opcode::mod: case IR::Opcode::bitwise_and: case IR::Opcode::bitwise_or:
                case IR::Opcode::bitwise_xor: case IR::Opcode::left_shift: case IR::Opcode::right_shift: {
                    auto r = simulationStack.peek(0); simulationStack.pop();
                    auto l = simulationStack.peek(0); simulationStack.pop();
                    auto resultType = std::make_shared<IRValueType>(*l.type);
                    if (l.type->hasAttribute(IRValueType::ValueAttr::Nullable) || r.type->hasAttribute(IRValueType::ValueAttr::Nullable)) {
                        resultType->addAttribute(IRValueType::ValueAttr::Nullable);
                    }
                    simulationStack.push(resultType, {});
                    break;
                }
                // Comparison Ops: Result is a non-nullable boolean.
                case IR::Opcode::equal: case IR::Opcode::not_equal: case IR::Opcode::less_than:
                case IR::Opcode::less_equal: case IR::Opcode::greater_than: case IR::Opcode::greater_equal: {
                    simulationStack.pop(); simulationStack.pop();
                    simulationStack.push(std::make_shared<IRValueType>(*compilerCtx->getBoolObjectType()), {});
                    break;
                }
                // Unary Ops: Nullability propagates.
                case IR::Opcode::negate: case IR::Opcode::bitwise_not: {
                    auto val = simulationStack.peek(0); simulationStack.pop();
                    simulationStack.push(std::make_shared<IRValueType>(*val.type), {});
                    break;
                }
                // Casting: Nullability propagates.
                case IR::Opcode::basic_cast_int: case IR::Opcode::basic_cast_deci: case IR::Opcode::basic_cast_bool: {
                    auto val = simulationStack.peek(0); simulationStack.pop();
                    std::shared_ptr<IRValueType> targetType;
                    if (ins.opcode == IR::Opcode::basic_cast_int) targetType = compilerCtx->getIntObjectType();
                    else if (ins.opcode == IR::Opcode::basic_cast_deci) targetType = compilerCtx->getDeciObjectType();
                    else targetType = compilerCtx->getBoolObjectType();
                    
                    auto resultType = std::make_shared<IRValueType>(*targetType);
                    if (val.type->hasAttribute(IRValueType::ValueAttr::Nullable)) {
                        resultType->addAttribute(IRValueType::ValueAttr::Nullable);
                    }
                    simulationStack.push(resultType, {});
                    break;
                }
                // Creation: Results are never null.
                case IR::Opcode::new_struct:
                    simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::structObject, ins.operands[0].value.symbolIndex, ins.operands[1].value.symbolIndex}), {});
                    break;
                case IR::Opcode::new_interface:
                    simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::interfaceObject, ins.operands[0].value.symbolIndex, ins.operands[1].value.symbolIndex}), {});
                    break;
                case IR::Opcode::construct_interface_impl: {
                    simulationStack.pop(); // struct
                    auto interfaceShell = simulationStack.peek(0);
                    simulationStack.pop(); // interface
                    simulationStack.push(std::make_shared<IRValueType>(*interfaceShell.type), {});
                    break;
                }
                // Dynamic Casts: Results are always nullable.
                case IR::Opcode::dyn_cast_int: case IR::Opcode::dyn_cast_bool: case IR::Opcode::dyn_cast_deci:
                case IR::Opcode::dyn_cast_char: case IR::Opcode::dyn_cast_str: case IR::Opcode::dyn_cast_struct: {
                    simulationStack.pop();
                    std::shared_ptr<IRValueType> resultType;
                    switch(ins.opcode) {
                        case IR::Opcode::dyn_cast_int: resultType = compilerCtx->getIntObjectType(); break;
                        case IR::Opcode::dyn_cast_bool: resultType = compilerCtx->getBoolObjectType(); break;
                        case IR::Opcode::dyn_cast_deci: resultType = compilerCtx->getDeciObjectType(); break;
                        case IR::Opcode::dyn_cast_char: resultType = compilerCtx->getCharObjectType(); break;
                        case IR::Opcode::dyn_cast_str: resultType = compilerCtx->getStrObjectType(); break;
                        case IR::Opcode::dyn_cast_struct: resultType = managedPtr(IRValueType{IRValueType::valueType::structObject, ins.operands[0].value.symbolIndex, ins.operands[1].value.symbolIndex}); break;
                        default: break;
                    }
                    auto finalType = std::make_shared<IRValueType>(*resultType);
                    finalType->addAttribute(IRValueType::ValueAttr::Nullable);
                    simulationStack.push(finalType, {});
                    break;
                }
                case IR::Opcode::array_length: {
                    simulationStack.pop(); // array
                    simulationStack.push(std::make_shared<IRValueType>(*compilerCtx->getIntObjectType()), {});
                    break;
                }
                case IR::Opcode::typeid_int: case IR::Opcode::typeid_bool: case IR::Opcode::typeid_char:
                case IR::Opcode::typeid_deci: case IR::Opcode::typeid_str: case IR::Opcode::typeid_struct:
                case IR::Opcode::typeid_interface: {
                    simulationStack.push(std::make_shared<IRValueType>(*compilerCtx->getIntObjectType()), {});
                    break;
                }
                case IR::Opcode::interfaceof: {
                    simulationStack.pop(); simulationStack.pop();
                    simulationStack.push(std::make_shared<IRValueType>(*compilerCtx->getBoolObjectType()), {});
                    break;
                }
                case IR::Opcode::pointer_cast: {
                    auto val = simulationStack.peek(0); simulationStack.pop();
                    auto resultType = std::make_shared<IRValueType>(IRValueType::valueType::pointerObject);
                    if (val.type->hasAttribute(IRValueType::ValueAttr::Nullable)) {
                        resultType->addAttribute(IRValueType::ValueAttr::Nullable);
                    }
                    simulationStack.push(resultType, {});
                    break;
                } 
                // Instructions that just pop
                case IR::Opcode::pop: case IR::Opcode::ret: case IR::Opcode::jump_if_true: case IR::Opcode::jump_if_false: {
                    if (!simulationStack.items.empty()) simulationStack.pop();
                    break;
                }
                // Instructions with no stack effect
                case IR::Opcode::jump: case IR::Opcode::ret_none: case IR::Opcode::nop:
                    break;
                // Other instructions with stack effects
                case IR::Opcode::direct_assign: {
                    simulationStack.pop(); // rhs
                    auto lhs = simulationStack.peek(0); 
                    simulationStack.pop(); // lhs
                    simulationStack.push(std::make_shared<IRValueType>(*lhs.type), {});
                    break;
                }
                case IR::Opcode::load_global: {
                    auto type = compilerCtx->getImportedModule(ins.operands[0].value.symbolIndex)->globalVariables[ins.operands[1].value.symbolIndex];
                    auto newType = std::make_shared<IRValueType>(*type);
                    newType->addAttribute(IRValueType::ValueAttr::Nullable); // Globals are conservatively nullable
                    simulationStack.push(newType, {});
                    break;
                }
                case IR::Opcode::store_global: {
                    if (!simulationStack.items.empty()) simulationStack.pop();
                    break;
                }
                default: 
                    handleInstruction(ins, 0, currentCodeBlockIndex);
            }
        }
        
        AnalysisState outState;
        outState.stack = simulationStack;
        for(const auto& [idx, state] : variablesExtraInfo) {
            outState.variableStates[idx] = {false, true, state.possibleValue};
        }
        return outState;
    }


    AnalysisState IROptimizer::mergeStatesForNullable(const AnalysisState &s1, const AnalysisState &s2) {
        if (s1.variableStates.empty()) return s2;
        if (s2.variableStates.empty()) return s1;
        
        if (s1.stack.items.size() != s2.stack.items.size()) {
            panic(0, 0, "IROptimizer: Incompatible stack depths at merge point.");
        }

        AnalysisState mergedState;

        for (size_t i = 0; i < s1.stack.items.size(); ++i) {
            const auto &item1 = s1.stack.items[i];
            const auto &item2 = s2.stack.items[i];

            auto mergedItem = IROptimizer::SimulationStack::Item{
                item1.type, false, {}, {}};
            
            if (item1.type->hasAttribute(IRValueType::ValueAttr::Nullable) || item2.type->hasAttribute(IRValueType::ValueAttr::Nullable)) {
                mergedItem.type->addAttribute(IRValueType::ValueAttr::Nullable);
            }
            mergedState.stack.push(mergedItem);
        }

        // Merge Variables (Conservative: if nullable in ANY path, it's nullable)
        std::set<indexT> allVarKeys;
        for (const auto &[key, val] : s1.variableStates) allVarKeys.insert(key);
        for (const auto &[key, val] : s2.variableStates) allVarKeys.insert(key);

        for (const auto &key : allVarKeys) {
            auto it1 = s1.variableStates.find(key);
            auto it2 = s2.variableStates.find(key);
            
            if (it1 != s1.variableStates.end() && it2 != s2.variableStates.end()) {
                auto mergedType = std::make_shared<IRValueType>(*it1->second.possibleValue.type);
                if (it2->second.possibleValue.type->hasAttribute(IRValueType::ValueAttr::Nullable)) {
                    mergedType->addAttribute(IRValueType::ValueAttr::Nullable);
                }
                mergedState.variableStates[key] = {false, true, {mergedType, false, {}}};
            } else if (it1 != s1.variableStates.end()) {
                mergedState.variableStates[key] = it1->second;
            } else {
                mergedState.variableStates[key] = it2->second;
            }
        }
        return mergedState;
    }
    
    // ===================================================================================
    // ==                               Raw Check Pass                                ==
    // ===================================================================================

    IROptimizer &IROptimizer::performRawCheck() {
        std::map<indexT, std::vector<indexT>> successors;
        std::map<indexT, std::vector<indexT>> predecessors;
        for (auto i = 0; i < targetFunction->codeBlock.size(); i++) {
            if (successors.find(i) == successors.end()) successors[i] = {};
            if (predecessors.find(i) == predecessors.end()) predecessors[i] = {};
            if (!targetFunction->codeBlock[i]->getIRArray().empty()) {
                auto& lastIns = targetFunction->codeBlock[i]->getIRArray().back();
                bool isTerminator = (lastIns.opcode == IR::Opcode::jump ||
                                    lastIns.opcode == IR::Opcode::jump_if_false ||
                                    lastIns.opcode == IR::Opcode::jump_if_true ||
                                    lastIns.opcode == IR::Opcode::ret ||
                                    lastIns.opcode == IR::Opcode::ret_none);
                if (!isTerminator && (i + 1 < targetFunction->codeBlock.size())) {
                    successors[i].push_back(i + 1);
                    predecessors[i + 1].push_back(i);
                }
            }
            for (auto &ins : targetFunction->codeBlock[i]->getIRArray()) {
                 switch (ins.opcode) {
                    case IR::Opcode::jump: {
                        indexT target = ins.operands[0].value.codeBlockIndex;
                        successors[i].push_back(target);
                        predecessors[target].push_back(i);
                        break;
                    }
                    case IR::Opcode::jump_if_true:
                    case IR::Opcode::jump_if_false: {
                        indexT target = ins.operands[0].value.codeBlockIndex;
                        successors[i].push_back(target);
                        if (i + 1 < targetFunction->codeBlock.size()) successors[i].push_back(i + 1);
                        predecessors[target].push_back(i);
                        if (i + 1 < targetFunction->codeBlock.size()) predecessors[i + 1].push_back(i);
                        break;
                    }
                    default: break;
                }
            }
        }
    
        std::map<indexT, AnalysisState> blockInStates;
        std::map<indexT, AnalysisState> blockOutStates;
        std::queue<indexT> worklist;
        
        // Rule 5: Parameters are not raw
        AnalysisState entryState;
        for(yoi::indexT i = 0; i < targetFunction->argumentTypes.size(); ++i) {
            auto varType = std::make_shared<IRValueType>(*targetFunction->variableTable.get(i));
            varType->removeAttribute(IRValueType::ValueAttr::Raw);
            entryState.variableStates[i] = {false, true, {varType, false, {}}};
        }

        worklist.push(0);
        blockInStates[0] = entryState;

        while (!worklist.empty()) {
            indexT currentBlockIdx = worklist.front();
            worklist.pop();

            AnalysisState inState = currentBlockIdx == 0 ? blockInStates[0] : AnalysisState{};
            if (predecessors.count(currentBlockIdx) > 0) {
                for (indexT predIdx : predecessors[currentBlockIdx]) {
                     if (blockOutStates.count(predIdx) > 0)
                        inState = mergeStatesForRaw(inState, blockOutStates[predIdx]);
                }
            }
            blockInStates[currentBlockIdx] = inState;

            AnalysisState newOutState = analyzeBlockForRaw(currentBlockIdx, inState);

            if (blockOutStates.find(currentBlockIdx) == blockOutStates.end() || blockOutStates[currentBlockIdx] != newOutState) {
                blockOutStates[currentBlockIdx] = newOutState;
                if (successors.count(currentBlockIdx) > 0) {
                    for (indexT succIdx : successors[currentBlockIdx]) {
                        worklist.push(succIdx);
                    }
                }
            }
        }

        // Apply results
        for(const auto& [varIndex, varType] : targetFunction->variableTable.getReversedVariableNameMap()) {
            bool isRaw = true; // Assume raw unless proven otherwise
            if (blockOutStates.empty()) isRaw = false; // No reachable blocks
            
            for(const auto& [blockIndex, outState] : blockOutStates) {
                if(outState.variableStates.count(varIndex) && !outState.variableStates.at(varIndex).possibleValue.type->hasAttribute(IRValueType::ValueAttr::Raw)) {
                    isRaw = false;
                    break;
                }
            }
            if(isRaw) {
                 targetFunction->variableTable.get(varIndex)->addAttribute(IRValueType::ValueAttr::Raw);
            } else {
                 targetFunction->variableTable.get(varIndex)->removeAttribute(IRValueType::ValueAttr::Raw);
            }
        }

        return *this;
    }

    AnalysisState IROptimizer::analyzeBlockForRaw(indexT blockIndex, const AnalysisState &inState) {
        simulationStack = inState.stack;
        variablesExtraInfo.clear();
        for(const auto& [idx, state] : inState.variableStates) {
            variablesExtraInfo[idx] = {false, true, {std::make_shared<IRValueType>(*state.possibleValue.type), false, {}}};
        }
        
        auto getVarType = [&](indexT varIndex) {
            if (!variablesExtraInfo.count(varIndex)) {
                auto originalType = targetFunction->variableTable.get(varIndex);
                variablesExtraInfo[varIndex] = {false, true, {std::make_shared<IRValueType>(*originalType), false, {}}};
            }
            return variablesExtraInfo.at(varIndex).possibleValue.type;
        };

        for (const auto &ins : targetFunction->codeBlock[blockIndex]->getIRArray()) {
            switch(ins.opcode) {
                // Rule 1: Push instructions for basic types create Raw values.
                case IR::Opcode::push_integer: {
                    auto newType = std::make_shared<IRValueType>(*compilerCtx->getIntObjectType());
                    newType->addAttribute(IRValueType::ValueAttr::Raw);
                    simulationStack.push(newType, {});
                    break;
                }
                case IR::Opcode::push_decimal: {
                    auto newType = std::make_shared<IRValueType>(*compilerCtx->getDeciObjectType());
                    newType->addAttribute(IRValueType::ValueAttr::Raw);
                    simulationStack.push(newType, {});
                    break;
                }
                case IR::Opcode::push_boolean: {
                    auto newType = std::make_shared<IRValueType>(*compilerCtx->getBoolObjectType());
                    newType->addAttribute(IRValueType::ValueAttr::Raw);
                    simulationStack.push(newType, {});
                    break;
                }
                // Other push instructions create non-Raw values.
                case IR::Opcode::push_string: {
                    auto newType = std::make_shared<IRValueType>(*compilerCtx->getStrObjectType());
                    simulationStack.push(newType, {});
                    break;
                }
                case IR::Opcode::push_null: {
                    auto newType = std::make_shared<IRValueType>(IRValueType::valueType::null);
                    simulationStack.push(newType, {});
                    break;
                }
                // Rule 2: store_local propagates Raw attribute.
                case IR::Opcode::store_local: {
                    auto value = simulationStack.peek(0);
                    simulationStack.pop();
                    auto varType = getVarType(ins.operands[0].value.symbolIndex);
                    varType->attributes = value.type->attributes;
                    break;
                }
                // Rule 3: load_local propagates Raw attribute.
                case IR::Opcode::load_local: {
                    auto varType = getVarType(ins.operands[0].value.symbolIndex);
                    simulationStack.push(std::make_shared<IRValueType>(*varType), {});
                    break;
                }
                // Rule 4: direct_assign destroys Raw attribute.
                case IR::Opcode::direct_assign: {
                    simulationStack.pop(); // rhs
                    auto lhs = simulationStack.peek(0); 
                    simulationStack.pop(); // lhs
                    auto resultType = std::make_shared<IRValueType>(*lhs.type);
                    resultType->removeAttribute(IRValueType::ValueAttr::Raw); 
                    simulationStack.push(resultType, {});
                    break;
                }
                case IR::Opcode::load_element: {
                    simulationStack.pop(); // index
                    auto array = simulationStack.peek(0);
                    simulationStack.pop();
                    auto elemType = std::make_shared<IRValueType>(array.type->getElementType());
                    if (array.type->isBasicType() && (array.type->isArrayType() || array.type->isDynamicArrayType())) {
                        elemType->addAttribute(IRValueType::ValueAttr::Raw);
                    } else {
                        elemType->removeAttribute(IRValueType::ValueAttr::Raw);
                    }
                    simulationStack.push(elemType, {});
                    break;
                }
                case IR::Opcode::store_element: {
                    simulationStack.pop(); // value
                    simulationStack.pop(); // index
                    simulationStack.pop(); // array
                    break;
                }
                case IR::Opcode::invoke: {
                    auto moduleIndex = ins.operands[0].value.symbolIndex;
                    auto funcIndex = ins.operands[1].value.symbolIndex;
                    auto func = compilerCtx->getImportedModule(moduleIndex)->functionTable[funcIndex];
                    for(size_t i = 0; i < func->argumentTypes.size(); ++i) simulationStack.pop();
                    auto returnType = std::make_shared<IRValueType>(*func->returnType);
                    returnType->removeAttribute(IRValueType::ValueAttr::Raw);
                    simulationStack.push(returnType, {});
                    break;
                }
                case IR::Opcode::invoke_virtual:
                case IR::Opcode::invoke_imported: {
                    auto argCount = ins.operands.back().value.symbolIndex;
                    for(size_t i = 0; i < argCount; ++i) if(!simulationStack.items.empty()) simulationStack.pop();
                    auto placeholderType = std::make_shared<IRValueType>(IRValueType::valueType::pointerObject);
                    placeholderType->removeAttribute(IRValueType::ValueAttr::Raw);
                    simulationStack.push(placeholderType, {});
                    break;
                }
                case IR::Opcode::basic_cast_int:
                case IR::Opcode::basic_cast_deci:
                case IR::Opcode::basic_cast_bool: {
                    auto val = simulationStack.peek(0); simulationStack.pop();
                    std::shared_ptr<IRValueType> targetType;
                    if (ins.opcode == IR::Opcode::basic_cast_int) targetType = compilerCtx->getIntObjectType();
                    else if (ins.opcode == IR::Opcode::basic_cast_deci) targetType = compilerCtx->getDeciObjectType();
                    else targetType = compilerCtx->getBoolObjectType();
                    
                    auto resultType = std::make_shared<IRValueType>(*targetType);
                    if (val.type->hasAttribute(IRValueType::ValueAttr::Raw)) {
                        resultType->addAttribute(IRValueType::ValueAttr::Raw);
                    }
                    simulationStack.push(resultType, {});
                    break;
                }
                default: {
                    handleInstruction(ins, 0, blockIndex);
                    break;
                }
            }
        }
        AnalysisState outState;
        outState.stack = simulationStack;
        for(const auto& [idx, state] : variablesExtraInfo) {
            outState.variableStates[idx] = {false, true, state.possibleValue};
        }
        return outState;
    }

    AnalysisState IROptimizer::mergeStatesForRaw(const AnalysisState &s1, const AnalysisState &s2) {
        if (s1.variableStates.empty()) return s2;
        if (s2.variableStates.empty()) return s1;

        if (s1.stack.items.size() != s2.stack.items.size()) {
            panic(0, 0, "IROptimizer: Incompatible stack depths at merge point.");
        }

        AnalysisState mergedState;

        for (size_t i = 0; i < s1.stack.items.size(); ++i) {
            const auto &item1 = s1.stack.items[i];
            const auto &item2 = s2.stack.items[i];

            auto mergedItem = IROptimizer::SimulationStack::Item{
                item1.type, false, {}, {}};
            
            if (item1.type->hasAttribute(IRValueType::ValueAttr::Raw) && item2.type->hasAttribute(IRValueType::ValueAttr::Raw)) {
                mergedItem.type->addAttribute(IRValueType::ValueAttr::Raw);
            }
            mergedState.stack.push(mergedItem);
        }

        // Merge Variables (Optimistic: must be raw in ALL paths to stay raw)
        std::set<indexT> allVarKeys;
        for (const auto &[key, val] : s1.variableStates) allVarKeys.insert(key);
        for (const auto &[key, val] : s2.variableStates) allVarKeys.insert(key);

        for (const auto &key : allVarKeys) {
            auto it1 = s1.variableStates.find(key);
            auto it2 = s2.variableStates.find(key);
            
            bool isRawInS1 = (it1 != s1.variableStates.end() && it1->second.possibleValue.type->hasAttribute(IRValueType::ValueAttr::Raw));
            bool isRawInS2 = (it2 != s2.variableStates.end() && it2->second.possibleValue.type->hasAttribute(IRValueType::ValueAttr::Raw));

            auto baseType = (it1 != s1.variableStates.end()) ? it1->second.possibleValue.type : it2->second.possibleValue.type;
            auto mergedType = std::make_shared<IRValueType>(*baseType);

            if ((it1 != s1.variableStates.end() && isRawInS1) && (it2 != s2.variableStates.end() && isRawInS2)) {
                // printf("localVar#%lld is raw in both paths\n", key);
                mergedType->addAttribute(IRValueType::ValueAttr::Raw);
            }  else { // only in s2
                mergedType->removeAttribute(IRValueType::ValueAttr::Raw);
            }
            mergedState.variableStates[key] = {false, true, {mergedType, false, {}}};
        }
        return mergedState;
    }

    void IROptimizer::handleInstruction(const IR &ins, yoi::indexT insIndex, yoi::indexT currentCodeBlockIndex) {
        switch (ins.opcode) {
            case IR::Opcode::push_boolean: {
                simulationStack.push(compilerCtx->getBoolObjectType(),
                                     {currentCodeBlockIndex, {insIndex}},
                                     ins.operands[0].value.boolean);
                break;
            }
            case IR::Opcode::push_integer: {
                simulationStack.push(compilerCtx->getIntObjectType(),
                                     {currentCodeBlockIndex, {insIndex}},
                                     ins.operands[0].value.integer);
                break;
            }
            case IR::Opcode::push_decimal: {
                simulationStack.push(compilerCtx->getDeciObjectType(),
                                     {currentCodeBlockIndex, {insIndex}},
                                     ins.operands[0].value.decimal);
                break;
            }
            case IR::Opcode::push_string: {
                simulationStack.push(compilerCtx->getStrObjectType(),
                                     {currentCodeBlockIndex, {insIndex}},
                                     ins.operands[0].value.stringLiteralIndex);
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
                    simulationStack.push(value);
                } else {
                    simulationStack.push(compilerCtx->getBoolObjectType(),
                                         value.contributedInstructions +
                                             SimulationStack::Item::ContributedInstructionSet{
                                                 currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                    simulationStack.push(value);
                } else {
                    simulationStack.push(compilerCtx->getIntObjectType(),
                                         value.contributedInstructions +
                                             SimulationStack::Item::ContributedInstructionSet{
                                                 currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                    simulationStack.push(value);
                } else {
                    simulationStack.push(compilerCtx->getDeciObjectType(),
                                         value.contributedInstructions +
                                             SimulationStack::Item::ContributedInstructionSet{
                                                 currentCodeBlockIndex, std::set{yoi::indexT{insIndex}}});
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
                // std::cout << "simulate add " << right.hasPossibleValue << " " << left.hasPossibleValue << " " <<
                // result.hasPossibleValue << std::endl;
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::sub: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = sub(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::mul: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = mul(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::div: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = div(left, right);
                // std::cout << "simulate div " << right.hasPossibleValue << " " << left.hasPossibleValue << " " <<
                // result.hasPossibleValue << std::endl;
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::mod: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = mod(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::negate: {
                auto value = simulationStack.peek(0);
                simulationStack.pop();
                // simulate
                auto result = negate(value);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::bitwise_and: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = bitwiseAnd(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::bitwise_or: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = bitwiseOr(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::bitwise_xor: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = bitwiseXor(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::bitwise_not: {
                auto value = simulationStack.peek(0);
                simulationStack.pop();
                // simulate
                auto result = bitwiseNot(value);
                // lost information, push back
                simulationStack.push(result.type, value.contributedInstructions);
                break;
            }
            case IR::Opcode::left_shift: {
                auto value = simulationStack.peek(0);
                auto shift = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = bitwiseShiftLeft(value, shift);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::right_shift: {
                auto value = simulationStack.peek(0);
                auto shift = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = bitwiseShiftRight(value, shift);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::less_than: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = lessThan(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::greater_than: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = greaterThan(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::less_equal: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = greaterThanOrEqual(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::greater_equal: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = greaterThanOrEqual(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::equal: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = equal(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::not_equal: {
                auto right = simulationStack.peek(0);
                auto left = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                // simulate
                auto result = notEqual(left, right);
                simulationStack.push(result);
                break;
            }
            case IR::Opcode::load_local: {
                auto varIndex = ins.operands[0].value.symbolIndex;
                auto varType = targetFunction->getVariableTable().get(varIndex);

                if (auto it = variablesExtraInfo.find(varIndex);
                    it != variablesExtraInfo.end() && it->second.hasPossibleValue) {
                    simulationStack.push(it->second.possibleValue);
                } else {
                    simulationStack.push(varType, {currentCodeBlockIndex, {insIndex}});
                }

                if (auto it = variablesExtraInfo.find(varIndex); it != variablesExtraInfo.end()) {
                    it->second.isReadAfterStore = true;
                } else {
                    variablesExtraInfo[varIndex] = {false, true, {}};
                }
                break;
            }
            case IR::Opcode::store_local: {
                auto varIndex = ins.operands[0].value.symbolIndex;
                auto value = simulationStack.peek(0);
                simulationStack.pop();

                variablesExtraInfo[varIndex] = {value.hasPossibleValue, false, value};
                break;
            }
            case IR::Opcode::load_global: {
                // as for global variables, we can't optimize it
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto type =
                    compilerCtx->getImportedModule(moduleIndex)->globalVariables[ins.operands[0].value.symbolIndex];
                simulationStack.push(type, {currentCodeBlockIndex, {insIndex}});
                break;
            }
            case IR::Opcode::store_global: {
                // check the definition type and value type here
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto definitionType =
                    compilerCtx->getImportedModule(moduleIndex)->globalVariables[ins.operands[1].value.symbolIndex];
                auto value = simulationStack.peek(0);

                if (*definitionType != *value.type) {
                    // type mismatch, panic
                    panic(0, 0, "IROptimizer::analyzeBlock(): store_global: type mismatch");
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
                simulationStack.push(memberDef,
                                     value.contributedInstructions + SimulationStack::Item::ContributedInstructionSet{
                                                                         currentCodeBlockIndex, {insIndex}});
                break;
            }
            case IR::Opcode::store_member: {
                // we can't optimize it
                auto value = simulationStack.peek(1);
                auto type = simulationStack.peek(0).type->typeIndex;
                auto structDef = irModule->structTable[type];
                auto memberIndex = ins.operands[0].value.symbolIndex;
                auto memberDef = structDef->fieldTypes[memberIndex];

                if (*memberDef != *value.type) {
                    // type mismatch, panic
                    panic(0, 0, "IROptimizer::analyzeBlock(): store_member: type mismatch");
                }

                simulationStack.pop();
                simulationStack.pop();
                break;
            }
            case IR::Opcode::invoke: {
                // we can't optimize it
                // in case of which this got optimized in tempVar reduction, we set optimizable flag to false
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto function =
                    compilerCtx->getImportedModule(moduleIndex)->functionTable[ins.operands[1].value.symbolIndex];
                auto returnType = function->returnType;
                auto argTypes = function->argumentTypes;
                auto argCount = function->argumentTypes.size();
                SimulationStack::Item::ContributedInstructionSet contributedInstructions = {
                    currentCodeBlockIndex, {insIndex}, false};
                for (int i = 0; i < argCount; i++) {
                    contributedInstructions = contributedInstructions + simulationStack.peek(0).contributedInstructions;
                    simulationStack.pop();
                }
                simulationStack.push(returnType, contributedInstructions);
                break;
            }
            case IR::Opcode::invoke_imported: {
                auto function = compilerCtx->getIRFFITable()
                                    ->importedLibraries[ins.operands[0].value.symbolIndex]
                                    .importedFunctionTable[ins.operands[1].value.symbolIndex];
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
                auto returnType = compilerCtx->getImportedModule(simulationStack.peek(0).type->typeAffiliateModule)
                                      ->interfaceTable[simulationStack.peek(0).type->typeIndex]
                                      ->methodMap[ins.operands[1].value.symbolIndex]
                                      ->returnType;
                simulationStack.pop();
                simulationStack.push(returnType, {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::new_struct: {
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto structDef =
                    compilerCtx->getImportedModule(moduleIndex)->structTable[ins.operands[1].value.symbolIndex];
                simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::structObject,
                                                            moduleIndex,
                                                            ins.operands[1].value.symbolIndex}),
                                     {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::new_interface: {
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto interfaceDef =
                    compilerCtx->getImportedModule(moduleIndex)->interfaceTable[ins.operands[1].value.symbolIndex];
                simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::interfaceObject,
                                                            moduleIndex,
                                                            ins.operands[1].value.symbolIndex}),
                                     {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::construct_interface_impl: {
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto interfaceImplDef = compilerCtx->getImportedModule(moduleIndex)
                                            ->interfaceImplementationTable[ins.operands[1].value.symbolIndex];
                auto returnType = simulationStack.peek(0).type;
                simulationStack.pop();
                simulationStack.pop();
                simulationStack.push(returnType, {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::jump_if_true:
            case IR::Opcode::jump_if_false:
            case IR::Opcode::ret: {
                simulationStack.pop();
                break;
            }
            case IR::Opcode::new_array_int:
            case IR::Opcode::new_array_bool:
            case IR::Opcode::new_array_char:
            case IR::Opcode::new_array_deci:
            case IR::Opcode::new_array_str: {
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
                simulationStack.push(managedPtr(baseType->getArrayType(dims)),
                                     {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case yoi::IR::Opcode::new_array_struct:
            case yoi::IR::Opcode::new_array_interface: {
                auto moduleIndex = ins.operands[0].value.symbolIndex;
                auto typeIndex = ins.operands[1].value.symbolIndex;
                yoi::indexT size = 1;
                yoi::vec<yoi::indexT> dims;

                auto baseType = managedPtr(IRValueType{ins.opcode == yoi::IR::Opcode::new_array_struct
                                                           ? IRValueType::valueType::structObject
                                                           : IRValueType::valueType::interfaceObject,
                                                       moduleIndex,
                                                       typeIndex});

                for (yoi::indexT i = 2; i < ins.operands.size(); i++) {
                    size *= ins.operands[i].value.symbolIndex;
                    dims.push_back(ins.operands[i].value.symbolIndex);
                }
                for (yoi::indexT i = 0; i < size; i++) {
                    simulationStack.pop();
                }
                simulationStack.push(managedPtr(baseType->getArrayType(dims)),
                                     {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::new_dynamic_array_int:
            case IR::Opcode::new_dynamic_array_bool:
            case IR::Opcode::new_dynamic_array_char:
            case IR::Opcode::new_dynamic_array_deci:
            case IR::Opcode::new_dynamic_array_str:
            case IR::Opcode::new_dynamic_array_struct:
            case IR::Opcode::new_dynamic_array_interface: {
                std::shared_ptr<IRValueType> baseType;
                switch (ins.opcode) {
                    case IR::Opcode::new_dynamic_array_int:
                        baseType = compilerCtx->getIntObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_bool:
                        baseType = compilerCtx->getBoolObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_char:
                        baseType = compilerCtx->getCharObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_deci:
                        baseType = compilerCtx->getDeciObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_str:
                        baseType = compilerCtx->getStrObjectType();
                        break;
                    case IR::Opcode::new_dynamic_array_interface:
                        baseType = managedPtr(IRValueType{IRValueType::valueType::interfaceObject,
                                                          ins.operands[0].value.symbolIndex,
                                                          ins.operands[1].value.symbolIndex});
                        break;
                    case IR::Opcode::new_dynamic_array_struct:
                        baseType = managedPtr(IRValueType{IRValueType::valueType::structObject,
                                                          ins.operands[0].value.symbolIndex,
                                                          ins.operands[1].value.symbolIndex});
                        break;
                    default:
                        break;
                }

                for (yoi::indexT i = 0; i < ins.operands.back().value.symbolIndex; i++) {
                    simulationStack.pop();
                }
                simulationStack.push(managedPtr(baseType->getDynamicArrayType()),
                                     {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::load_element: {
                // we can't optimize it
                auto index = simulationStack.peek(0);
                auto array = simulationStack.peek(1);
                simulationStack.pop();
                simulationStack.pop();
                simulationStack.push(
                    managedPtr(array.type->getElementType()),
                    array.contributedInstructions + index.contributedInstructions +
                        SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}, false});
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
            case IR::Opcode::pop: {
                simulationStack.pop();
                break;
            }
            case IR::Opcode::direct_assign: {
                auto rhs = simulationStack.peek(0);
                auto lhs = simulationStack.peek(1);
                yoi_assert(*lhs.type == *rhs.type, 0, 0, "IROptimizer::analyzeBlock(): direct_assign: type mismatch");
                simulationStack.pop();
                simulationStack.pop();
                simulationStack.push(
                    lhs.type,
                    lhs.contributedInstructions + rhs.contributedInstructions +
                        SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::typeid_int:
            case IR::Opcode::typeid_bool:
            case IR::Opcode::typeid_char:
            case IR::Opcode::typeid_deci:
            case IR::Opcode::typeid_str:
            case IR::Opcode::typeid_struct:
            case IR::Opcode::typeid_interface: {
                simulationStack.push(compilerCtx->getIntObjectType(), {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::dyn_cast_int:
            case IR::Opcode::dyn_cast_bool:
            case IR::Opcode::dyn_cast_deci:
            case IR::Opcode::dyn_cast_char:
            case IR::Opcode::dyn_cast_str: {
                std::shared_ptr<IRValueType> value_type;
                switch (ins.opcode) {
                    case IR::Opcode::dyn_cast_int:
                        value_type = compilerCtx->getIntObjectType();
                        break;
                    case IR::Opcode::dyn_cast_bool:
                        value_type = compilerCtx->getBoolObjectType();
                        break;
                    case IR::Opcode::dyn_cast_char:
                        value_type = compilerCtx->getCharObjectType();
                        break;
                    case IR::Opcode::dyn_cast_deci:
                        value_type = compilerCtx->getDeciObjectType();
                        break;
                    case IR::Opcode::dyn_cast_str:
                        value_type = compilerCtx->getStrObjectType();
                        break;
                    default:
                        break;
                }
                simulationStack.pop();
                simulationStack.push(value_type, {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::dyn_cast_struct: {
                auto structType = managedPtr(IRValueType{IRValueType::valueType::structObject,
                                                         ins.operands[0].value.symbolIndex,
                                                         ins.operands[1].value.symbolIndex});
                simulationStack.pop();
                simulationStack.push(structType, {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::pointer_cast: {
                simulationStack.pop();
                simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::pointerObject}),
                                     {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            case IR::Opcode::push_null: {
                simulationStack.push(managedPtr(IRValueType{IRValueType::valueType::pointerObject}),
                                     {currentCodeBlockIndex, {insIndex}, true});
                break;
            }
            case IR::Opcode::array_length: {
                auto array = simulationStack.peek(0);
                simulationStack.pop();
                if (array.type->isArrayType()) {
                    yoi::indexT size = 1;
                    for (auto dim : array.type->dimensions) {
                        size *= dim;
                    }
                    simulationStack.push(
                        compilerCtx->getIntObjectType(),
                        array.contributedInstructions +
                            SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}},
                        static_cast<int64_t>(size));
                } else if (array.type->isDynamicArrayType()) {
                    simulationStack.push(
                        compilerCtx->getIntObjectType(),
                        array.contributedInstructions +
                            SimulationStack::Item::ContributedInstructionSet{currentCodeBlockIndex, {insIndex}, false});
                }
                break;
            }
            case IR::Opcode::interfaceof: {
                // pop two values and push one boolean value
                auto interfaceType = simulationStack.peek(0).type;
                auto objectType = simulationStack.peek(1).type;
                simulationStack.pop();
                simulationStack.pop();
                simulationStack.push(compilerCtx->getBoolObjectType(), {currentCodeBlockIndex, {insIndex}, false});
                break;
            }
            default: {
                // pass
                break;
            }
        }
    }
} // namespace yoi
