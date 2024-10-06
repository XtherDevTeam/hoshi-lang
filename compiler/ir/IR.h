//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_IR_H
#define HOSHI_LANG_IR_H

#include "share/def.hpp"
#include <map>
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
            none,
            charRaw,
        } type;

        yoi::indexT typeAffiliateModule;
        yoi::indexT typeIndex;

        IRValueType(valueType type);

        IRValueType(valueType type, yoi::indexT typeAffiliateModule, yoi::indexT objectPrototypeIndex);

        bool isBasicType() const;

        bool is1ByteType() const;

        yoi::wstr to_string() const;

        bool operator==(const yoi::IRValueType & rhs) const;
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
            FINAL
        } type;

        static enum_range<operandType> IROperandTypeEnumRange;

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

        yoi::wstr to_string() const;
    };

    class IR {
    public:

        enum class Opcode {
            unknown = 0,
            load_local,
            negate,
            bitwise_not,
            mul,
            mod,
            div,
            increment,
            decrement,add, sub, right_shift, less_than, less_equal, greater_than, greater_equal, equal,
            not_equal, left_shift, bitwise_and, bitwise_xor, bitwise_or, jump, jump_if_true, jump_if_false, load_member,
            load_global, load_extern, dummy_break, dummy_continue, ret, ret_void,
            push_integer, push_decimal, push_boolean, basic_cast_int, basic_cast_deci, basic_cast_bool, push_string,
            store_global, store_local, store_member, store_extern, invoke, invoke_extern, nop, FINAL, ret_none
        } opcode;

        static enum_range<Opcode> IROpCodeEnumRange;

        yoi::vec<IROperand> operands;

        IR() = default;

        IR(Opcode opcode, const yoi::vec<IROperand> &operands);

        yoi::wstr to_string() const;
    };

    class IRCodeBlock {
        yoi::vec<IR> codeBlock;

    public:
        IRCodeBlock() = default;

        void insert(const IR &ir);

        yoi::wstr to_string(yoi::indexT indent = 0);

        yoi::vec<IR> &getIRArray();
    };

    class IRVariableTable {
        yoi::vec<std::shared_ptr<IRValueType>> variables;
        yoi::vec<std::map<yoi::wstr, yoi::indexT>> variableNameIndexMap;
        std::map<yoi::indexT, yoi::indexT> variableScopeMap;
        std::map<yoi::indexT, yoi::wstr> reversedVariableNameMap;

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

        yoi::wstr to_string(yoi::indexT indent = 0);
    };

    class IRFunctionDefinition {
    public:
        yoi::wstr name;
        yoi::vec<std::shared_ptr<IRValueType>> argumentTypes;
        std::shared_ptr<IRValueType> returnType;
        yoi::vec<std::shared_ptr<IRCodeBlock>> codeBlock;
        IRVariableTable variableTable;

        IRFunctionDefinition(const yoi::wstr &name, const yoi::vec <std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> &argumentTypes, const std::shared_ptr<IRValueType> &returnType);

        IRVariableTable &getVariableTable();

        yoi::wstr to_string(yoi::indexT indent = 0);

        struct Builder {
            yoi::wstr name;
            yoi::vec <std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> argumentTypes;
            std::shared_ptr<IRValueType> returnType;

            Builder() = default;

            Builder &setName(const yoi::wstr &name);

            Builder &addArgument(const yoi::wstr &argumentName, const std::shared_ptr<IRValueType> &argumentType);

            Builder &setReturnType(const std::shared_ptr<IRValueType> &returnType);

            std::shared_ptr<IRFunctionDefinition> yield();
        };
    };

    class IRStructDefinition {
    public:
        yoi::wstr name;
        yoi::vec<std::shared_ptr<IRValueType>> fieldTypes;

        struct nameInfo {
            enum class nameType {
                field,
                method
            } type;
            yoi::indexT index;
        };
        std::map<yoi::wstr, nameInfo> nameIndexMap;

        IRStructDefinition(const yoi::wstr &name, const std::map<yoi::wstr, nameInfo> &nameIndexMap, const yoi::vec<std::shared_ptr<IRValueType>> &fieldTypes);

        const nameInfo &lookupName(const yoi::wstr &name);

        yoi::wstr to_string(yoi::indexT indent = 0);

        struct Builder {
            yoi::wstr name;
            std::map<yoi::wstr, nameInfo> nameIndexMap;
            yoi::vec<std::shared_ptr<IRValueType>> fieldTypes;

            Builder() = default;

            Builder &setName(const yoi::wstr &name);

            Builder &addField(const yoi::wstr &fieldName, const std::shared_ptr<IRValueType> &fieldType);

            Builder &addMethod(const yoi::wstr &methodName, yoi::indexT index);

            std::shared_ptr<IRStructDefinition> yield();
        };
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
        bool compiled;
        std::map<yoi::wstr, yoi::indexT> moduleImports;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> functionTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRStructDefinition>> structTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> globalVariables;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRExternEntry>> externTable;
        IRStringLiteralPool stringLiteralPool;

        yoi::wstr to_string(yoi::indexT indent = 0);
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

        void basicCast(const std::shared_ptr<IRValueType> &valType, yoi::indexT insertionPoint, bool lhs = false);

        void uniqueArithmeticOp(IR::Opcode op);

        void arithmeticOp(IR::Opcode op);

        void jumpOp(yoi::indexT target);

        void jumpIfOp(IR::Opcode op, yoi::indexT target);

        void pushOp(IR::Opcode op, const yoi::IROperand &constV);

        void loadOp(IR::Opcode op, const yoi::IROperand &source, const std::shared_ptr<IRValueType>& expectedType);

        void loadMemberOp(const yoi::IROperand &memberIndex, const std::shared_ptr<IRValueType> &memberType);

        void storeOp(IR::Opcode op, const yoi::IROperand &operand);

        void storeMemberOp(const yoi::IROperand &memberIndex);

        /**
         * @brief Invoke a function with the given arguments.
         * @param funcIndex The index of function in irModule->functionTable
         * @param funcArgsCount The number of arguments of invocation.
         * @param returnType The return type of the function. Need for push the return value type to tempVarStack.
         * @param externalInvocation If true, the function is invoked from an external module.
         */
        void invokeOp(yoi::indexT funcIndex, yoi::indexT funcArgsCount, const std::shared_ptr<IRValueType> &returnType, bool
                      externalInvocation = false);

        void invokeMethodOp(yoi::indexT funcIndex, yoi::indexT methodArgsCount, const std::shared_ptr<IRValueType> &returnType, bool externalInvocation = false);

        void retOp(bool returnWithNone = false);

        yoi::indexT getCurrentInsertionPoint();

        void popFromTempVarStack();
    };

    class IRObjectFile {
    public:
        /* one module that has renamed all functions and variables to their final names */
        std::shared_ptr<IRModule> compiledModule;

        yoi::indexT entryModule;
    };
} // yoi

#endif //HOSHI_LANG_IR_H
