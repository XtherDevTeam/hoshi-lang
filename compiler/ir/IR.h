//
// Created by XIaokang00010 on 2024/9/6.
//

#ifndef HOSHI_LANG_IR_H
#define HOSHI_LANG_IR_H

#include "share/def.hpp"
#include <compiler/compilerContext.h>
#include <compiler/frontend/ast.hpp>
#include <map>
#include <memory>
#include <set>

namespace yoi {
    struct IRBuildConfig {
        enum class BuildType : yoi::indexT { library = 0, executable } buildType;
        enum class BuildMode : yoi::indexT { debug = 0, release } buildMode;
        enum class UseObjectLinker : yoi::indexT { cc = 0, cl, none } useObjectLinker;
        yoi::wstr buildPlatform;
        yoi::wstr buildArch;
        bool preserveIntermediateFiles;
        yoi::vec<yoi::wstr> searchPaths;
        yoi::vec<yoi::wstr> additionalLinkingFiles;
        std::map<yoi::wstr, yoi::wstr> marcos;

        struct Builder {
            BuildType buildType{BuildType::executable};
            BuildMode buildMode{BuildMode::debug};
            UseObjectLinker useObjectLinker{UseObjectLinker::cc};
            yoi::wstr buildPlatform;
            yoi::wstr buildArch;
            bool preserveIntermediateFiles{false};
            yoi::vec<yoi::wstr> searchPaths{L""};
            yoi::vec<yoi::wstr> additionalLinkingFiles;
            std::map<yoi::wstr, yoi::wstr> marcos;

            Builder() = default;

            Builder &setBuildType(BuildType buildType);

            Builder &setBuildMode(BuildMode buildMode);

            Builder &setUseObjectLinker(UseObjectLinker useObjectLinker);

            Builder &setBuildPlatform(const yoi::wstr &buildPlatform);

            Builder &setBuildArch(const yoi::wstr &buildArch);

            Builder &setPreserveIntermediateFiles(bool preserveIntermediateFiles);

            Builder &addSearchPath(const yoi::wstr &searchPath);

            Builder &setSearchPaths(const yoi::vec<yoi::wstr> &searchPaths);

            Builder &setMarco(const yoi::wstr &name, const yoi::wstr &value);

            std::shared_ptr<IRBuildConfig> yield();
        };
    };

    struct IRDebugInfo {
        yoi::wstr sourceFile;
        yoi::indexT line;
        yoi::indexT column;
    };

    class IRValueType {
      public:
        enum class ValueAttr : yoi::indexT {
            Nullable,
            Raw,
            PermanentInCurrentScope,
            Borrow,
            NoBorrow
        };
        enum class valueType : yoi::indexT {
            integerRaw = 0,
            decimalRaw,
            booleanRaw,
            shortRaw,
            unsignedRaw,
            characterObject,
            stringLiteral,
            structObject,
            null,
            integerObject,
            booleanObject,
            decimalObject,
            shortObject,
            unsignedObject,
            stringObject,
            virtualMethod,
            pointerObject, // a placeholder for void* in llvmCodegen for unified interface this pointer
            pointer,
            interfaceObject,
            none,
            charRaw,
            incompleteTemplateType,
            foreignInt32Type,
            foreignFloatType,
        } type;

        yoi::indexT typeAffiliateModule;
        yoi::indexT typeIndex;

        yoi::vec<yoi::indexT> dimensions;

        std::set<ValueAttr> attributes;

        IRValueType();

        IRValueType(valueType type);

        IRValueType(valueType type, yoi::indexT typeAffiliateModule, yoi::indexT objectPrototypeIndex);

        IRValueType(valueType type,
                    yoi::indexT typeAffiliateModule,
                    yoi::indexT objectPrototypeIndex,
                    const std::set<ValueAttr> &attributes);

        IRValueType(valueType type, const yoi::vec<yoi::indexT> &dimensions);

        IRValueType(valueType type,
                    yoi::indexT typeAffiliateModule,
                    yoi::indexT objectPrototypeIndex,
                    const yoi::vec<yoi::indexT> &dimensions);

        bool isBasicType() const;

        bool isBasicRawType() const;

        bool isForeignBasicType() const;

        bool is1ByteType() const;

        bool isArrayType() const;

        bool isDynamicArrayType() const;

        IRValueType getNormalizedForeignBasicType();

        IRValueType getElementType();

        IRValueType getArrayType(const yoi::vec<yoi::indexT> &dimensions);

        IRValueType getDynamicArrayType();

        IRValueType getBasicRawType() const;

        IRValueType getBasicObjectType() const;

        yoi::wstr to_string(bool showAttributes = false) const;

        bool operator==(const yoi::IRValueType &rhs) const;

        IRValueType & addAttribute(ValueAttr attr);

        IRValueType & removeAttribute(ValueAttr attr);

        bool hasAttribute(ValueAttr attr) const;
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
            shortInt,
            unsignedInt,
            /* a local var operand can only be used in a load_local instruction for loading a local variable, not
               available for other instructions */
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
            uint64_t unsignedV;
            short shortV;

            operandValue();

            operandValue(int64_t integer);

            operandValue(double decimal);

            operandValue(yoi::indexT indexV);

            operandValue(bool boolean);

            operandValue(yoi::wchar character);

            operandValue(short shortV);
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
            decrement,
            add,
            sub,
            right_shift,
            less_than,
            less_equal,
            greater_than,
            greater_equal,
            equal,
            direct_assign,
            not_equal,
            left_shift,
            bitwise_and,
            bitwise_xor,
            bitwise_or,
            jump,
            jump_if_true,
            jump_if_false,
            load_member,
            load_global,
            ret,
            ret_none,
            push_integer,
            push_decimal,
            push_boolean,
            push_character,
            push_null,
            push_short,
            push_unsigned,
            pop,
            basic_cast_int,
            basic_cast_deci,
            basic_cast_bool,
            basic_cast_char,
            basic_cast_short,
            basic_cast_unsigned,
            pointer_cast,
            push_string,
            store_global,
            store_local,
            store_member,
            invoke,
            new_struct,
            new_interface,
            construct_interface_impl,
            invoke_virtual,
            invoke_imported,
            store_element,
            load_element,
            new_array_int,
            new_array_deci,
            new_array_bool,
            new_array_char,
            new_array_str,
            new_array_short,
            new_array_unsigned,
            new_array_struct,
            new_array_interface,
            new_dynamic_array_int,
            new_dynamic_array_deci,
            new_dynamic_array_bool,
            new_dynamic_array_char,
            new_dynamic_array_str,
            new_dynamic_array_short,
            new_dynamic_array_unsigned,
            new_dynamic_array_struct,
            new_dynamic_array_interface,
            array_length,
            interfaceof,
            throws,
            push_exception_handler,
            pop_exception_handler,
            typeid_int,
            typeid_deci,
            typeid_bool,
            typeid_char,
            typeid_str,
            typeid_short,
            typeid_unsigned,
            typeid_struct,
            typeid_interface,
            typeid_object,
            typeid_object_non_stack,
            typeid_interface_impl,
            dyn_cast_int,
            dyn_cast_deci,
            dyn_cast_bool,
            dyn_cast_char,
            dyn_cast_str,
            dyn_cast_struct,
            dyn_cast_any,
            nop,
            FINAL,
        } opcode;

        static enum_range<Opcode> IROpCodeEnumRange;

        yoi::vec<IROperand> operands;

        IRDebugInfo debugInfo;

        IR() = default;

        IR(Opcode opcode, const yoi::vec<IROperand> &operands, IRDebugInfo debugInfo);

        yoi::wstr to_string() const;
    };

    class IRTypeAlias {
        public:
        yoi::wstr name;
        std::shared_ptr<IRValueType> type;
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

        yoi::vec<std::shared_ptr<IRValueType>> &getVariables();

        const std::map<yoi::indexT, yoi::wstr> &getReversedVariableNameMap() const;

        yoi::indexT scopeIndex(yoi::indexT varIndex);
    };

    class IRFunctionDefinition {
      public:
        enum class FunctionAttrs {
            AlwaysInline,
            BuiltinImplementation,
            NoFFI,
            Variadic,
            Static,
            Constructor,
            Unreachable,
            Preserve,
            NoRawAndNullOptimization,
        };

        yoi::wstr name;
        yoi::vec<std::shared_ptr<IRValueType>> argumentTypes;
        std::shared_ptr<IRValueType> returnType;
        yoi::vec<std::shared_ptr<IRCodeBlock>> codeBlock;
        IRVariableTable variableTable;
        yoi::vec<FunctionAttrs> attrs;
        IRDebugInfo debugInfo;

        IRFunctionDefinition(const yoi::wstr &name,
                             const yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> &argumentTypes,
                             const std::shared_ptr<IRValueType> &returnType,
                             const yoi::vec<std::shared_ptr<IRCodeBlock>> &codeBlock,
                             const yoi::vec<FunctionAttrs> &attrs,
                             const IRDebugInfo &debugInfo);

        IRVariableTable &getVariableTable();

        yoi::wstr to_string(yoi::indexT indent = 0);

        bool hasAttribute(const FunctionAttrs &attr);

        struct Builder {
            yoi::wstr name;
            yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> argumentTypes;
            std::shared_ptr<IRValueType> returnType;
            yoi::vec<FunctionAttrs> attrs;
            IRDebugInfo debugInfo;

            Builder() = default;

            Builder &setName(const yoi::wstr &name);

            Builder &addArgument(const yoi::wstr &argumentName, const std::shared_ptr<IRValueType> &argumentType);

            Builder &setReturnType(const std::shared_ptr<IRValueType> &returnType);

            Builder &setDebugInfo(const IRDebugInfo &debugInfo);

            Builder &addAttr(FunctionAttrs attr);

            std::shared_ptr<IRFunctionDefinition> yield();
        };
    };

    class IRTemplateBuilder {
      public:
        struct Argument {
            std::shared_ptr<IRValueType> templateType;
            std::pair<yoi::indexT, yoi::indexT> interfaceType;

            Argument(const std::shared_ptr<IRValueType> &templateType,
                     const std::pair<yoi::indexT, yoi::indexT> &interfaceType);

            Argument(const std::shared_ptr<IRValueType> &templateType);
        };

        yoi::indexTable<yoi::wstr, Argument> templateArguments;

        IRTemplateBuilder() = default;

        IRTemplateBuilder &addTemplateArgument(const yoi::wstr &templateName,
                                               const std::shared_ptr<IRValueType> &templateType,
                                               const std::pair<yoi::indexT, yoi::indexT> &interfaceType = {0, 0});
    };

    class IRFunctionTemplate {
      public:
        std::shared_ptr<IRFunctionDefinition> templateDefinition;
        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;

        IRFunctionTemplate(const std::shared_ptr<IRFunctionDefinition> &templateDefinition,
                           const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments);

        class Builder : public IRTemplateBuilder {
          public:
            std::shared_ptr<IRFunctionDefinition> templateDefinition;

            Builder() = default;

            Builder &setTemplateDefinition(const std::shared_ptr<IRFunctionDefinition> &templateDefinition);

            std::shared_ptr<IRFunctionTemplate> yield();
        };
    };

    class IRStructDefinition {
      public:
        yoi::wstr name;
        yoi::vec<std::shared_ptr<IRValueType>> fieldTypes;

        struct nameInfo {
            enum class nameType { field, method } type;
            yoi::indexT index;
        };
        std::map<yoi::wstr, nameInfo> nameIndexMap;

        IRStructDefinition(const yoi::wstr &name,
                           const std::map<yoi::wstr, nameInfo> &nameIndexMap,
                           const yoi::vec<std::shared_ptr<IRValueType>> &fieldTypes);

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

    class IRStructTemplate {
      public:
        std::shared_ptr<IRStructDefinition> templateDefinition;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionTemplate>> templateMethods;
        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;

        IRStructTemplate(const std::shared_ptr<IRStructDefinition> &templateDefinition,
                         const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionTemplate>> &templateMethods,
                         const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments);

        class Builder : public IRTemplateBuilder {
          public:
            std::shared_ptr<IRStructDefinition> templateDefinition;
            yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionTemplate>> templateMethods;

            Builder() = default;

            Builder &setTemplateDefinition(const std::shared_ptr<IRStructDefinition> &templateDefinition);

            Builder &setTemplateMethod(const yoi::wstr &methodName,
                                       const std::shared_ptr<IRFunctionTemplate> &methodTemplate);

            std::shared_ptr<IRStructTemplate> yield();
        };
    };

    class IRInterfaceImplementationDefinition {
      public:
        yoi::wstr name;
        std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex;
        yoi::indexT implInterfaceIndex;
        yoi::vec<std::shared_ptr<IRValueType>> virtualMethods;
        std::map<yoi::wstr, yoi::indexT> virtualMethodIndexMap;

        IRInterfaceImplementationDefinition(
            const yoi::wstr &name,
            std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex,
            yoi::indexT implInterfaceIndex,
            const yoi::vec<std::shared_ptr<IRValueType>> &virtualMethods,
            const std::map<yoi::wstr, yoi::indexT> &virtualMethodIndexMap);

        yoi::wstr to_string(yoi::indexT indent = 0);

        struct Builder {
            yoi::wstr name;
            std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex;
            yoi::indexT implInterfaceIndex;
            yoi::vec<std::shared_ptr<IRValueType>> virtualMethods;
            std::map<yoi::wstr, yoi::indexT> virtualMethodIndexMap;

            Builder() = default;

            Builder &setName(const yoi::wstr &name);

            Builder &setImplStructIndex(std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT> implStructIndex);

            Builder &setImplInterfaceIndex(yoi::indexT implInterfaceIndex);

            Builder &addVirtualMethod(const yoi::wstr &methodName, const std::shared_ptr<IRValueType> &methodType);

            std::shared_ptr<IRInterfaceImplementationDefinition> yield();
        };
    };

    class IRInterfaceInstanceDefinition {
      public:
        yoi::wstr name;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> methodMap;
        yoi::vec<std::tuple<IRValueType::valueType, yoi::indexT, yoi::indexT>> implementations;

        IRInterfaceInstanceDefinition(
            const yoi::wstr &name, const yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> &methodMap);

        yoi::wstr to_string(yoi::indexT indent = 0);

        struct Builder {
            yoi::wstr name;
            yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> methodMap;

            Builder() = default;

            Builder &setName(const yoi::wstr &name);

            Builder &addMethod(const yoi::wstr &methodName,
                               const std::shared_ptr<IRFunctionDefinition> &methodSignature);

            std::shared_ptr<IRInterfaceInstanceDefinition> yield();
        };
    };

    class IRInterfaceInstanceTemplate {
      public:
        std::shared_ptr<IRInterfaceInstanceDefinition> templateDefinition;
        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;

        class Builder : public IRTemplateBuilder {
          public:
            std::shared_ptr<IRInterfaceInstanceDefinition> templateDefinition;

            Builder() = default;

            Builder &setTemplateDefinition(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition);

            std::shared_ptr<IRInterfaceInstanceTemplate> yield();
        };

        IRInterfaceInstanceTemplate(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition,
                                    const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments);
    };

    class IRInterfaceImplementationTemplate {
      public:
        std::shared_ptr<IRInterfaceImplementationDefinition> templateDefinition;
        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;

        class Builder : public IRTemplateBuilder {
          public:
            std::shared_ptr<IRInterfaceImplementationDefinition> templateDefinition;
            yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;

            Builder() = default;

            Builder &
            setTemplateDefinition(const std::shared_ptr<IRInterfaceImplementationDefinition> &templateDefinition);

            std::shared_ptr<IRInterfaceImplementationTemplate> yield();
        };

        IRInterfaceImplementationTemplate(
            const std::shared_ptr<IRInterfaceImplementationDefinition> &templateDefinition,
            const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments);
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
            structType,
            interfaceType,
            interfaceImplType,
            importedFunction,
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
        yoi::wstr modulePath;
        std::map<yoi::wstr, yoi::indexT> moduleImports;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> functionTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRStructDefinition>> structTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> globalVariables;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRExternEntry>> externTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRInterfaceInstanceDefinition>> interfaceTable;
        yoi::indexTable<yoi::wstr, std::shared_ptr<IRInterfaceImplementationDefinition>> interfaceImplementationTable;

        std::map<yoi::wstr, yoi::funcDefStmt *> funcTemplateAsts;
        std::map<yoi::wstr, yoi::structDefStmt *> structTemplateAsts;
        std::map<yoi::wstr, yoi::interfaceDefStmt *> templateInterfaceAsts;
        std::map<yoi::wstr, yoi::implStmt *> templateImplAsts; // Maps struct template name to its impl block
        std::map<yoi::wstr, yoi::vec<yoi::implStmt *>> templateInterfaceImplAsts;
        std::map<yoi::wstr, yoi::typeAliasStmt *> typeAliasTemplateAsts;
        std::map<yoi::wstr, IRValueType> typeAliases;
        std::map<yoi::wstr, yoi::vec<yoi::indexT>> functionOverloadIndexies;

        IRStringLiteralPool stringLiteralPool;

        yoi::wstr to_string(yoi::indexT indent = 0);
    };

    class IRBuilder {
        struct LoopContext {
            yoi::indexT breakTarget;
            yoi::indexT continueTarget;
        };
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> currentModule;
        std::shared_ptr<IRFunctionDefinition> currentFunction;
        std::vector<std::shared_ptr<IRCodeBlock>> codeBlocks;
        std::vector<std::shared_ptr<yoi::IRValueType>> tempVarStack;
        yoi::indexT currentCodeBlockIndex;
        yoi::vec<std::pair<yoi::indexT, yoi::indexT>> codeBlockInsertionStates;
        yoi::vec<IR> tempStateCodeBlock;
        yoi::vec<std::shared_ptr<yoi::IRValueType>> tempStateTempVarStack;
        IRDebugInfo currentDebugInfo;
        yoi::vec<LoopContext> loopContext;

      public:
        IRBuilder() = delete;

        IRBuilder(std::shared_ptr<compilerContext> compilerCtx,
                  std::shared_ptr<IRModule> currentModule,
                  std::shared_ptr<IRFunctionDefinition> currentFunction);

        void setDebugInfo(const IRDebugInfo &debugInfo);

        const IRDebugInfo &getCurrentDebugInfo();

        void pushLoopContext(yoi::indexT breakTarget, yoi::indexT continueTarget);

        void popLoopContext();

        yoi::indexT saveState();

        void discardState();

        void restoreState();

        void restoreStateTemporarily(); // rollback to the state with current state saved

        void commitState(); // commit the overriden state and pour the saved state back

        void discardStateUntil(yoi::indexT stateIndex);

        void pushTempVar(const std::shared_ptr<IRValueType> &type);

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

        std::shared_ptr<IRValueType> &getLhsFromTempVarStack();

        std::shared_ptr<IRValueType> &getRhsFromTempVarStack();

        void basicCast(const std::shared_ptr<IRValueType> &valType, yoi::indexT insertionPoint, bool lhs = false);

        void popOp();

        void uniqueArithmeticOp(IR::Opcode op);

        void arithmeticOp(IR::Opcode op);

        bool hasTerminated();

        void jumpOp(yoi::indexT target);

        void jumpIfOp(IR::Opcode op, yoi::indexT target);

        void pushOp(IR::Opcode op, const yoi::IROperand &constV);

        void loadOp(IR::Opcode op,
                    const yoi::IROperand &source,
                    const std::shared_ptr<IRValueType> &expectedType,
                    yoi::indexT moduleIndex = -1);

        void loadMemberOp(const yoi::IROperand &memberIndex, const std::shared_ptr<IRValueType> &memberType);

        void storeOp(IR::Opcode op, const yoi::IROperand &operand, yoi::indexT moduleIndex = -1);

        void storeMemberOp(const yoi::IROperand &memberIndex);

        /**
         * @brief Invoke a function with the given arguments.
         * @param funcIndex The index of function in irModule->functionTable
         * @param funcArgsCount The number of arguments of invocation.
         * @param returnType The return type of the function. Need for push the return value type to tempVarStack.
         * @param externalInvocation If true, the function is invoked from an external module.
         */
        void invokeOp(yoi::indexT funcIndex,
                      yoi::indexT funcArgsCount,
                      const std::shared_ptr<IRValueType> &returnType,
                      bool externalInvocation = false,
                      yoi::indexT moduleIndex = -1);

        void invokeMethodOp(yoi::indexT funcIndex,
                            yoi::indexT methodArgsCount,
                            const std::shared_ptr<IRValueType> &returnType,
                            bool isStatic,
                            bool externalInvocation = false,
                            yoi::indexT moduleIndex = -1);

        void invokeVirtualOp(yoi::indexT funcIndex,
                             yoi::indexT interfaceIndex,
                             yoi::indexT methodArgsCount,
                             const std::shared_ptr<IRValueType> &returnType,
                             bool externalInvocation = false,
                             yoi::indexT moduleIndex = -1);

        void invokeImportedOp(yoi::indexT libIndex,
                              yoi::indexT funcIndex,
                              yoi::indexT funcArgsCount,
                              const std::shared_ptr<IRValueType> &returnType);

        void retOp(bool returnWithNone = false);

        void newStructOp(yoi::indexT structIndex, bool isExternal = false, yoi::indexT moduleIndex = -1);

        void newInterfaceOp(yoi::indexT interfaceIndex, bool isExternal = false, yoi::indexT moduleIndex = -1);

        void
        constructInterfaceImplOp(yoi::indexT interfaceImplIndex, bool isExternal = false, yoi::indexT moduleIndex = -1);

        void newArrayOp(const std::shared_ptr<IRValueType> &elementType, const yoi::vec<yoi::indexT> &dimensions);

        void newDynamicArrayOp(const std::shared_ptr<IRValueType> &elementType, yoi::indexT initializerSize = 0);

        void arrayLengthOp();

        void interfaceOfOp();

        void typeIdOp(const std::shared_ptr<IRValueType> &type);

        void typeIdOp();

        void dynCastOp(const std::shared_ptr<IRValueType> &type);

        void pointerCastOp();

        void breakOp();

        void continueOp();

        yoi::indexT getCurrentInsertionPoint();

        void popFromTempVarStack();
    };

    class IRObjectFile {
      public:
        /* one module that has renamed all functions and variables to their final names */
        std::shared_ptr<IRModule> compiledModule;

        yoi::indexT entryModule;
    };

    class IRFFITable {
      public:
        class ImportLibrary {
          public:
            yoi::wstr libraryPath;

            yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionDefinition>> importedFunctionTable;

            ImportLibrary(const yoi::wstr &libraryPath);
        };

        yoi::indexTable<yoi::wstr, std::tuple<yoi::indexT, yoi::indexT, yoi::vec<IRFunctionDefinition::FunctionAttrs>>>
            exportedFunctionTable;

        yoi::indexTable<yoi::wstr, std::shared_ptr<IRValueType>> foreignTypeTable;

        yoi::indexTable<yoi::wstr, ImportLibrary> importedLibraries;

        yoi::indexT addImportedFunction(const yoi::wstr &libraryName,
                                        const yoi::wstr &functionName,
                                        const std::shared_ptr<IRFunctionDefinition> &functionDefinition);

        void addForeignType(const yoi::wstr &foreignTypeName, const std::shared_ptr<IRValueType> &structType);

        /**
         * @brief Add an exported function to the FFI table.
         *
         * @param exportName The name of the exported function.
         * @param moduleIndex Module index of the function.
         * @param functionIndex Function index of the function.
         * @param attrs The attributes of the function.
         * @throws std::out_of_range If the export name already exists in the FFI table.
         */
        void addExportedFunction(const yoi::wstr &exportName,
                                 yoi::indexT moduleIndex,
                                 yoi::indexT functionIndex,
                                 const yoi::vec<IRFunctionDefinition::FunctionAttrs> &attrs);
    };
} // namespace yoi

#endif // HOSHI_LANG_IR_H