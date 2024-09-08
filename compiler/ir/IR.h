//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_IR_H
#define HOSHI_LANG_IR_H

#include "share/def.hpp"

namespace yoi {

    class IROperand {
    public:
        enum class operandType {
            unknown = 0,
            integer,
            decimal,
            boolean,
            character,
            stringLiteral,
            objectReference,
            code_block,
        } type;

        union operandValue {
            int64_t integer;
            double decimal;
            bool boolean;
            char character;
            yoi::indexT stringLiteralIndex;
            yoi::indexT symbolIndex;
            yoi::indexT codeBlockIndex;

            operandValue();

            operandValue(int64_t integer);

            operandValue(double decimal);

            operandValue(yoi::indexT indexV);
        } value;

        IROperand();

        IROperand(operandType type, operandValue value);
    };

    class IR {
    public:
        enum class Opcode {
            unknown = 0,
        } opcode;

        yoi::vec<IROperand> operands;

        IR() = default;

        IR(Opcode opcode, const yoi::vec<IROperand> &operands);

        yoi::wstr to_string();
    };

    class IRCodeBlock {
        std::vector<IR> codeBlock;

    public:
        IRCodeBlock() = default;

        void insert(const IR &ir);

        yoi::wstr to_string();
    };

    class IRBuilder {
        std::vector<IRCodeBlock> codeBlocks;
        std::stack<yoi::indexT> codeBlockStack;
    public:
        IRBuilder() = default;

        yoi::indexT createCodeBlock();

        IRCodeBlock &getCurrentCodeBlock();

        IRCodeBlock &getCodeBlock(yoi::indexT index);

        void popCodeBlock();

        std::vector<IRCodeBlock> finish();
    };

    class IRValueType {
        enum class valueType : yoi::indexT {
            integer = 0,
            decimal,
            boolean,
            character,
            stringLiteral,
            objectReference
        } type;

        yoi::indexT objectPrototypeIndex;

        IRValueType(valueType type);

        IRValueType(valueType type, yoi::indexT objectPrototypeIndex);
    };

    class IRFunctionDefinition {
    public:
        std::string name;
        yoi::vec<IRValueType> argumentTypes;
        yoi::vec<IRCodeBlock> codeBlock;

        IRFunctionDefinition(const std::string &name, const yoi::vec <IRCodeBlock> &codeBlock, const yoi::vec <IRValueType> &argumentTypes);

        yoi::wstr to_string();
    };

    class IRStructDefinition {
    public:
        std::string name;
        yoi::vec<IRValueType> fieldTypes;
        yoi::vec<IRFunctionDefinition> methodDefinitions;

        IRStructDefinition(const std::string &name, const yoi::vec<IRValueType> &fieldTypes, const yoi::vec<IRFunctionDefinition> &methodDefinitions);

        yoi::wstr to_string();
    };

    class IRStringLiteralPool {
    public:
        yoi::indexPool<yoi::wstr> pool;

        yoi::indexT addStringLiteral(const yoi::wstr &str);

        yoi::wstr &getStringLiteral(yoi::indexT index);
    };

    class IRModule {
    public:
        yoi::indexTable<yoi::wstr, IRFunctionDefinition> functionTable;
        yoi::indexTable<yoi::wstr, IRStructDefinition> structTable;
        yoi::indexTable<yoi::wstr, IRValueType> globalVariables;
        IRStringLiteralPool stringLiteralPool;
    };
} // yoi

#endif //HOSHI_LANG_IR_H
