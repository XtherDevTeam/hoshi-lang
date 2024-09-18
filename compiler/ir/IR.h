//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_IR_H
#define HOSHI_LANG_IR_H

#include "share/def.hpp"
#include <map>
#include <ranges>

namespace yoi {
    class IRValueType {
    public:
        enum class valueType : yoi::indexT {
            integer = 0,
            decimal,
            boolean,
            character,
            stringLiteral,
            structType,
            lvalue,
            null,
        } type;

        yoi::indexT typeIndex;

        std::shared_ptr<IRValueType> lvalueType;

        IRValueType(valueType type);

        IRValueType(valueType type, yoi::indexT objectPrototypeIndex);

        IRValueType(valueType type, std::shared_ptr<IRValueType> lvalueType);

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
            tempVar,
            codeBlock,
            index,
            /* a local var operand can only be used in a load_local instruction for loading a local variable, not available for other instructions */
            localVar,
            /* same for global var */
            globalVar,

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
            load_global, load_extern, dummy_break, dummy_continue, ret
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
        std::vector<std::shared_ptr<IRValueType>> tempVars;
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
        std::shared_ptr<IRModule> currentModule;
        std::shared_ptr<IRFunctionDefinition> currentFunction;
        std::vector<std::shared_ptr<IRCodeBlock>> codeBlocks;
        yoi::indexT currentCodeBlockIndex;
        std::vector<std::shared_ptr<IRValueType>> tempVars;
    public:
        IRBuilder() = delete;

        IRBuilder(std::shared_ptr<IRModule> currentModule, std::shared_ptr<IRFunctionDefinition> currentFunction);

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

        yoi::IROperand createTempVar(const std::shared_ptr<IRValueType> &type);

        IRValueType getTempVar(yoi::indexT index);

        void insert(const IR &ir);

        yoi::IROperand createLocalVar(const yoi::wstr &varName, const std::shared_ptr<IRValueType> &type);

        IRValueType getLocalVar(yoi::indexT index);

        /**
         * @brief Extract the value type from an operand, returning their IRValueType definition if they are localVar or globalVar,
         * returning the IRValueType from tempVar, otherwise return the corresponding IRValueType for the operand type.
         * @param operand The operand to extract the value type from.
         * @return The value type of the operand.
         * @author Jerry Chau
         */
        std::shared_ptr<IRValueType> extractValueType(const IROperand &operand);

        /**
         * @brief Dereference an operand, returning the value type of the dereferenced operand. It will only work for tempVar with lvalue type.
         * @param operand The operand to dereference.
         * @return The value type of the dereferenced operand.
         * @author Jerry Chau
         */
        yoi::IROperand deref(const yoi::IROperand &operand);

        yoi::IROperand basicCast(const yoi::IROperand &operand, const std::shared_ptr<IRValueType> &type);

        yoi::IROperand arithmeticOp(IR::Opcode op, const yoi::IROperand &left, const yoi::IROperand &right);

        void jumpOp(yoi::indexT target);

        void jumpIfOp(IR::Opcode op, const yoi::IROperand &condition, yoi::indexT target);
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
