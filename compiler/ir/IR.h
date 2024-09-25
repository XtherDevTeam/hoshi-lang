//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_IR_H
#define HOSHI_LANG_IR_H

#include "share/def.hpp"
#include <map>
#include <ranges>
#include <compiler/compilerContext.h>

namespace yoi {
    class IRValueType {
    public:
        enum class valueType : yoi::indexT {
            integerRaw = 0,
            decimalRaw,
            booleanRaw,
            characterObject,
            stringLiteral,
            structObject,
            null,
            integerObject,
            booleanObject,
            decimalObject,
            stringObject,
        } type;

        yoi::indexT typeIndex;

        IRValueType(valueType type);

        IRValueType(valueType type, yoi::indexT objectPrototypeIndex);

        bool isBasicType() const;
    };

    class IROperand {
    public:
        enum class operandType {
            unknown = 0,
            integer,
            decimal,
            boolean,
            character,
            stringLiteral,
            codeBlock,
            index,
            /* a local var operand can only be used in a load_local instruction for loading a local variable, not available for other instructions */
            localVar,
            /* same for global var */
            globalVar,
            /* same for extern var */
            externVar,
        } type;

        union operandValue {
            int64_t integer;
            double decimal;
            bool boolean;
            yoi::wchar character;
            yoi::indexT stringLiteralIndex;
            yoi::indexT symbolIndex;
            yoi::indexT codeBlockIndex;

            operandValue();

            operandValue(int64_t integer);

            operandValue(double decimal);

            operandValue(yoi::indexT indexV);

            operandValue(bool boolean);

            operandValue(yoi::wchar character);
        } value;

        std::shared_ptr<IRValueType> lvalueType;

        IROperand();

        IROperand(operandType type, operandValue value);

        IROperand(operandType type, std::shared_ptr<IRValueType> lvalueType);

        std::shared_ptr<IRValueType> getLvalueType();
    };

    class IR {
    public:
        enum class Opcode {
            unknown = 0,
            store,
            load_local,
            increment,
            decrement,
            negate,
            bitwiseNot,
            mul,
            mod,
            div,
            deref,
            multiply, basic_cast, add, sub, right_shift, less_than, less_equal, greater_than, greater_equal, equal,
            not_equal, left_shift, bitwise_and, bitwise_xor, bitwise_or, jump, jump_if_true, jump_if_false, load_member,
            load_global, load_extern, dummy_break, dummy_continue, ret,
            push_integer, push_decimal, push_boolean, basic_cast_int, basic_cast_deci, basic_cast_bool, push_string,
            store_global, store_local, store_member, store_extern
        } opcode;

        yoi::vec<IROperand> operands;

        IR() = default;

        IR(Opcode opcode, const yoi::vec<IROperand> &operands);

        yoi::wstr to_string();
    };

    class IRCodeBlock {
        yoi::vec<IR> codeBlock;

    public:
        IRCodeBlock() = default;

        void insert(const IR &ir);

        yoi::wstr to_string();

        yoi::vec<IR> &getIRArray();
    };

    class IRVariableTable {
        yoi::vec<std::shared_ptr<IRValueType>> variables;
        yoi::vec<std::map<yoi::wstr, yoi::indexT>> variableNameIndexMap;

    public:
        IRVariableTable() = default;

        yoi::indexT createScope();

        /**
         * @brief Look up a variable by name in the current scope.
         * If the variable is not found in the current scope, look it up in the parent scope.
         * @param name The name of the variable to look up.
         * @return The index of the variable in the variable table.
         * @throws std::runtime_error panics if the variable is not found in any scope.
         */
        yoi::indexT lookup(const yoi::wstr &name);

        std::shared_ptr<IRValueType> get(yoi::indexT index);

        std::shared_ptr<IRValueType> operator[](const yoi::wstr &name);

        yoi::indexT put(const yoi::wstr &name, const std::shared_ptr<IRValueType> &type);

        void popScope();
    };

    class IRFunctionDefinition {
    public:
        yoi::wstr name;
        yoi::vec<std::shared_ptr<IRValueType>> argumentTypes;
        std::shared_ptr<IRValueType> returnType;
        yoi::vec<std::shared_ptr<IRCodeBlock>> codeBlock;
        IRVariableTable variableTable;

        IRFunctionDefinition(const yoi::wstr &name, const yoi::vec <std::shared_ptr<IRValueType>> &argumentTypes, const std::shared_ptr<IRValueType> &returnType);

        IRVariableTable &getVariableTable();

        yoi::wstr to_string();
    };

    class IRStructDefinition {
    public:
        yoi::wstr name;
        yoi::vec<std::shared_ptr<IRValueType>> fieldTypes;
        yoi::vec<std::shared_ptr<IRFunctionDefinition>> methodDefinitions;

        struct nameInfo {
            enum class nameType {
                field,
                method
            } type;
            yoi::indexT index;
        };
        std::map<yoi::wstr, nameInfo> nameIndexMap;

        IRStructDefinition(const yoi::wstr &name, const std::map<yoi::wstr, nameInfo> &nameIndexMap, const yoi::vec<std::shared_ptr<IRValueType>> &fieldTypes, const yoi::vec<std::shared_ptr<IRFunctionDefinition>> &methodDefinitions);

        const nameInfo &lookupName(const yoi::wstr &name);

        yoi::wstr to_string();
    };

    class IRStringLiteralPool {
    public:
        yoi::indexPool<yoi::wstr> pool;

        yoi::indexT addStringLiteral(const yoi::wstr &str);

        yoi::wstr &getStringLiteral(yoi::indexT index);
    };

    class IRExternEntry {
    public:
        enum class externType {
            globalVar,
            function,
            structType
        } type;

        yoi::wstr name;
        yoi::indexT affiliateModule;
        yoi::indexT itemIndex;

        IRExternEntry() = default;

        IRExternEntry(externType type, const yoi::wstr &name, yoi::indexT affiliateModule, yoi::indexT itemIndex);

        externType getExternType() const;
    };

    class IRModule : std::enable_shared_from_this<IRModule> {
    public:
        yoi::indexT identifier;
        std::map<yoi::wstr, yoi::wstr> moduleImports;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> functionTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRStructDefinition>> structTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> globalVariables;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRExternEntry>> externTable;
        IRStringLiteralPool stringLiteralPool;
    };

    class IRBuilder {
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> currentModule;
        std::shared_ptr<IRFunctionDefinition> currentFunction;
        std::vector<std::shared_ptr<IRCodeBlock>> codeBlocks;
        std::vector<std::shared_ptr<yoi::IRValueType>> tempVarStack;
        yoi::indexT currentCodeBlockIndex;
    public:
        IRBuilder() = delete;

        IRBuilder(std::shared_ptr<compilerContext> compilerCtx,std::shared_ptr<IRModule> currentModule, std::shared_ptr<IRFunctionDefinition> currentFunction);

        yoi::indexT createCodeBlock();

        std::shared_ptr<IRFunctionDefinition> irFuncDefinition();

        IRCodeBlock &getCurrentCodeBlock();

        yoi::indexT getCurrentCodeBlockIndex();

        /**
         * Switch to the specified code block.
         * @param index the index of the code block to switch to
         * @return the index of the previous code block
         */
        yoi::indexT switchCodeBlock(yoi::indexT index);

        IRCodeBlock &getCodeBlock(yoi::indexT index);

        void yield();

        void insert(const IR &ir, yoi::indexT insertionPoint = 0xffffffff);

        yoi::IROperand createLocalVar(const yoi::wstr &varName, const std::shared_ptr<IRValueType> &type);

        IRValueType getLocalVar(yoi::indexT index);

        const std::shared_ptr<IRValueType> &getLhsFromTempVarStack();

        const std::shared_ptr<IRValueType> &getRhsFromTempVarStack();

        void IRBuilder::basicCast(const std::shared_ptr<IRValueType> &valType, yoi::indexT insertionPoint);

        void uniqueArithmeticOp(IR::Opcode op);

        void arithmeticOp(IR::Opcode op);

        void jumpOp(yoi::indexT target);

        void jumpIfOp(IR::Opcode op, yoi::indexT target);

        void pushOp(IR::Opcode op, const yoi::IROperand &constV);

        void loadOp(IR::Opcode op, const yoi::IROperand &operand);

        void loadMemberOp(const yoi::IROperand &memberIndex, const std::shared_ptr<IRValueType> &memberType);

        void storeOp(IR::Opcode op, const yoi::IROperand &operand);

        void storeMemberOp(const yoi::IROperand &memberIndex, const std::shared_ptr<IRValueType> &memberType);

        void retOp();

        yoi::indexT getCurrentInsertionPoint();
    };

    class IRObjectFile {
    public:
        /* saves all compiled modules, indexed by their path */
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRModule>> modules;
        /* one module that has renamed all functions and variables to their final names */
        std::shared_ptr<IRModule> compiledModule;
        yoi::indexT entryModule;
    };
} // yoi

#endif //HOSHI_LANG_IR_H
