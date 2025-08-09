//
// Created by Jerry Chou on 10/2/2024.
//

#ifndef IROPTIMIZER_HPP
#define IROPTIMIZER_HPP
#include <set>

#include "IR.h"

#include <compiler/compilerContext.h>
#include <compiler/frontend/lexer.hpp>

namespace yoi {

    class IROptimizer;

    class AnalysisState;

    class IROptimizer {
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> irModule;
        std::shared_ptr<IRFunctionDefinition> targetFunction;
        yoi::indexT currentCodeBlockIndex;
    public:

        struct SimulationStack {
            struct Item {
                struct ContributedInstructionSet {
                    yoi::indexT codeBlockIndex;
                    std::set<yoi::indexT> instructions;
                    bool optimizable{true};

                    ContributedInstructionSet() = default;

                    ContributedInstructionSet(yoi::indexT codeBlockIndex, const std::set<yoi::indexT> &instructions);

                    ContributedInstructionSet(yoi::indexT codeBlockIndex, const std::set<yoi::indexT> &instructions, bool optimizable);

                    ContributedInstructionSet &insert(yoi::indexT index);

                    ContributedInstructionSet operator+(const ContributedInstructionSet &other) const;

                    struct Iterator {
                        std::set<yoi::indexT>::const_iterator it;

                        Iterator(const std::set<yoi::indexT> &set);

                        Iterator(std::set<yoi::indexT>::const_iterator it);

                        bool operator!=(const Iterator &other) const;

                        yoi::indexT operator*() const;

                        Iterator &operator++();
                    };

                    Iterator begin() const;

                    Iterator end() const;
                };
                std::shared_ptr<IRValueType> type;
                bool hasPossibleValue;
                union PossibleValue {
                    int64_t intValue;
                    double deciValue;
                    bool boolValue;
                    char charValue;
                    yoi::indexT stringConstIndex;

                    PossibleValue() = default;

                    PossibleValue(int64_t intValue);

                    PossibleValue(double deciValue);

                    PossibleValue(bool boolValue);

                    PossibleValue(yoi::indexT stringConstIndex);

                    PossibleValue(char charValue);
                } possibleValue;

                ContributedInstructionSet contributedInstructions;
            };
            std::vector<Item> items;

            SimulationStack() = default;

            /**
             * Creates a new simulation stack item with the given type, contributed instructions, and will not have a possible value.
             * @param type The IRValueType of the item.
             * @param contributedInstructions The set of instructions that contribute to the value of this item.
             */
            void push(const std::shared_ptr<IRValueType> &type, const Item::ContributedInstructionSet & contributedInstructions);

            /**
             * Creates a new simulation stack item with the given type, contributed instructions, and possible value.
             * @param type The IRValueType of the item.
             * @param contributedInstructions The set of instructions that contribute to the value of this item.
             * @param value The possible value of the item.
             */
            void push(const std::shared_ptr<IRValueType> &type, const Item::ContributedInstructionSet & contributedInstructions, Item::PossibleValue value);

            void push(const Item &item);

            void pop();

            Item &peek(yoi::indexT index);
        } simulationStack;

        /**
        * Reduce the given set of instructions by removing redundant instructions.
        * @param contributedInstructions The set of instructions to be reduced.
        * @param currentIndex The current index of the instruction being processed.
        * @return The index of the next instruction to be processed.
        */
        yoi::indexT reduce(const SimulationStack::Item::ContributedInstructionSet &contributedInstructions, yoi::indexT currentIndex);

        SimulationStack::Item add(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item sub(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item mul(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item div(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item mod(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item negate(const IROptimizer::SimulationStack::Item& a);

        SimulationStack::Item bitwiseNot(const IROptimizer::SimulationStack::Item& a);

        SimulationStack::Item bitwiseAnd(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseOr(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseXor(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseShiftLeft(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseShiftRight(const IROptimizer::SimulationStack::Item& a, const IROptimizer::SimulationStack::Item &b);

        SimulationStack::Item lessThan(const SimulationStack::Item & item, const SimulationStack::Item & right);

        SimulationStack::Item lessThanOrEqual(const SimulationStack::Item & item, const SimulationStack::Item & right);

        SimulationStack::Item greaterThan(const SimulationStack::Item & item, const SimulationStack::Item & right);

        SimulationStack::Item greaterThanOrEqual(const SimulationStack::Item & item, const SimulationStack::Item & right);

        SimulationStack::Item equal(const SimulationStack::Item & item, const SimulationStack::Item & right);

        SimulationStack::Item notEqual(const SimulationStack::Item & item, const SimulationStack::Item & right);

        struct VariablesExtraInfo {
            bool hasPossibleValue;
            bool isReadAfterStore;
            SimulationStack::Item possibleValue; // store the result of the latest store operation to this variable.
        };

        std::map<yoi::indexT, VariablesExtraInfo> variablesExtraInfo;

        /**
         * Generate push constant operation for the given value.
         * @param item The value to be pushed.
         * @param index The index of the instruction to be inserted.
         * @return The index of the next instruction to be inserted.
         */
        yoi::indexT generatePushOp(const SimulationStack::Item &item, yoi::indexT index);

        IROptimizer(const std::shared_ptr<compilerContext> &compilerCtx, const std::shared_ptr<IRModule> &irModule);

        IROptimizer &setTargetFunction(const std::shared_ptr<IRFunctionDefinition> &targetFunction);

        IROptimizer &reduceRedundantConstantExpr();

        IROptimizer &reduceRedundantTempVar();

        IROptimizer &reduceRedundantNop();

        IROptimizer &reduceRedundantJump();

        IROptimizer &reduceRedundantCodeAfterRet();

        IROptimizer &controlFlowOptimization();

        IROptimizer &doOptimizationForCurrentFunction();

        IROptimizer &reduceEmptyCodeBlock();

        AnalysisState analyzeBlock(indexT blockIndex, const AnalysisState &inState);

        void transformBlock(indexT blockIndex, const AnalysisState &inState);        
    };

    struct AnalysisState {
        IROptimizer::SimulationStack stack;
        std::map<indexT, IROptimizer::VariablesExtraInfo> variableStates;

        // A simple comparison for the worklist algorithm to detect changes.
        bool operator!=(const AnalysisState &other) const;
    };

    AnalysisState mergeStates(const AnalysisState &s1, const AnalysisState &s2);

} // yoi

#endif //IROPTIMIZER_HPP
