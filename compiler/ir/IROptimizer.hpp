//
// Created by Jerry Chou on 10/2/2024.
//

#ifndef IROPTIMIZER_HPP
#define IROPTIMIZER_HPP
#include <set>

#include "IR.h"
#include "share/def.hpp"

#include <compiler/compilerContext.h>
#include <compiler/frontend/lexer.hpp>

namespace yoi {

    class IRFunctionOptimizer;

    class AnalysisState;

    struct CallGraph {
        using FuncIdentifier = std::pair<indexT, indexT>; // (module index, function index)

        yoi::indexT entryModuleIndex{};

        std::map<FuncIdentifier, std::set<FuncIdentifier>> callGraph; // successors for each function
        std::map<FuncIdentifier, std::set<FuncIdentifier>> callerGraph; // predecessors for each function
        std::set<FuncIdentifier> entryPoints; // entry points of the program
        std::set<FuncIdentifier> unreachableFunctions; // functions that are not reachable from the entry points
        std::set<FuncIdentifier> functions; // all functions in the program

        CallGraph() = default;

        void addCall(FuncIdentifier caller, FuncIdentifier callee);

        void traverseGraph();
    };

    struct FunctionAnalysisInfo {
        bool isReturnValueNullable = false;
        bool isReturnValueRaw = true;

        // For checking if the analysis has reached a fixed point.
        bool operator!=(const FunctionAnalysisInfo &other) const;
    };

    class IRFunctionOptimizer {
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> irModule;
        std::shared_ptr<IRFunctionDefinition> targetFunction;
        yoi::indexT currentCodeBlockIndex;
        const std::map<CallGraph::FuncIdentifier, FunctionAnalysisInfo> &globalAnalysisResults;
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
                    uint64_t unsignedValue;
                    short shortValue;

                    PossibleValue() = default;

                    PossibleValue(int64_t intValue);

                    PossibleValue(double deciValue);

                    PossibleValue(bool boolValue);

                    PossibleValue(yoi::indexT stringConstIndex);

                    PossibleValue(char charValue);

                    PossibleValue(short shortValue);
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

        SimulationStack::Item add(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item sub(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item mul(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item div(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item mod(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item negate(const IRFunctionOptimizer::SimulationStack::Item& a);

        SimulationStack::Item bitwiseNot(const IRFunctionOptimizer::SimulationStack::Item& a);

        SimulationStack::Item bitwiseAnd(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseOr(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseXor(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseShiftLeft(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

        SimulationStack::Item bitwiseShiftRight(const IRFunctionOptimizer::SimulationStack::Item& a, const IRFunctionOptimizer::SimulationStack::Item &b);

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

        IRFunctionOptimizer(const std::shared_ptr<compilerContext> &compilerCtx, const std::shared_ptr<IRModule> &irModule, const std::map<CallGraph::FuncIdentifier, FunctionAnalysisInfo>& globalResults);

        IRFunctionOptimizer &setTargetFunction(const std::shared_ptr<IRFunctionDefinition> &targetFunction);

        IRFunctionOptimizer &reduceRedundantConstantExpr();

        IRFunctionOptimizer &reduceRedundantTempVar();

        IRFunctionOptimizer &reduceRedundantNop();

        IRFunctionOptimizer &reduceRedundantJump();

        IRFunctionOptimizer &reduceRedundantCodeAfterRet();

        IRFunctionOptimizer &controlFlowOptimization();

        IRFunctionOptimizer &doOptimizationForCurrentFunction();

        IRFunctionOptimizer &reduceEmptyCodeBlock();

        void handleInstruction(const IR &ins, yoi::indexT insIndex, yoi::indexT currentCodeBlockIndex);

        AnalysisState analyzeBlock(indexT blockIndex, const AnalysisState &inState);

        void transformBlock(indexT blockIndex, const AnalysisState &inState);        

        bool performNullableCheck();
        bool performRawCheck();
        bool performParamBorrowCheck();

      private:
        AnalysisState analyzeBlockForNullable(indexT blockIndex, const AnalysisState &inState);
        AnalysisState analyzeBlockForRaw(indexT blockIndex, const AnalysisState &inState);
        AnalysisState mergeStatesForNullable(const AnalysisState &s1, const AnalysisState &s2);
        AnalysisState mergeStatesForRaw(const AnalysisState &s1, const AnalysisState &s2);
    };

    struct AnalysisState {
        IRFunctionOptimizer::SimulationStack stack;
        std::map<indexT, IRFunctionOptimizer::VariablesExtraInfo> variableStates;

        // A simple comparison for the worklist algorithm to detect changes.
        bool operator!=(const AnalysisState &other) const;
    };


    AnalysisState mergeStates(const AnalysisState &s1, const AnalysisState &s2);

    class IROptimizer {
        std::shared_ptr<compilerContext> compilerCtx;
        CallGraph callGraph;
        yoi::indexT entryModuleIndex;
        std::map<CallGraph::FuncIdentifier, FunctionAnalysisInfo> functionAnalysisResults;

        bool performStructNullablePass();
        
    public:
        IROptimizer(const std::shared_ptr<compilerContext> &compilerCtx, yoi::indexT entryModuleIndex);

        void buildCallGraph();

        void optimize();
    };
        
} // yoi

#endif //IROPTIMIZER_HPP
