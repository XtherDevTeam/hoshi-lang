//
// Created by XIaokang00010 on 2023/3/3.
//

#ifndef HOSHI_LANG_IR_HPP
#define HOSHI_LANG_IR_HPP

#include <share/def.hpp>

namespace hoshi {
    class irBuilder;

    class irContext;

    class irFuncConstantPool;

    class irConstantPool;

    class irStructType;

    class irStructTable;

    class irFuncType;

    class irTempVarTable;

    class irFunc;

    class irVarTable;

    class irInstruction;

    class irBasicBlock;

    class irModule;

    class irValueType;

    class irConstVal;

    class irConstantPool {
        indexPool<vdeci> deciPool;
    public:
        int64_t putDeci(vdeci d);
    };

    class irConstVal {
    public:
        enum class type {
            vInt,
            vDeci,
            vBool,
        } tp{};

        int64_t v{};

        void set(type t, int64_t val);

        void set(type t, bool val);

        bool getBool();

        int64_t getDeciIdx();

        int64_t getInt();

        bool operator==(const irConstVal &rhs) const;
    };

    /**
     * @brief struct type in Hoshi IR
     * @note Every object is pointers in hoshi-lang, functions are objects too, so we don't need to make an other table for functions
     */
    class irStructType {
        indexTable<wstr, irValueType> fields;
    public:
        int64_t setFieldType(const wstr &name, const irValueType &value);

        irValueType &getFieldType(int64_t i);

        int64_t getFieldIndex(const wstr &name);
    };

    /**
     * @brief struct table which is used to save struct types.
     */
    class irStructTable {
        indexTable<wstr, irStructType> structTypes;
    public:
        int64_t putStructType(const wstr &name, const irStructType &value);

        irStructType &getStructType(int64_t i);
    };

    class irFuncType {
        indexTable<wstr, irValueType> argsType;
        std::shared_ptr<irValueType> resultType;
    public:
        /**
         * @brief a function which is used to set the type of each param
         * @warning make sure add args in order
         * @param name the name of param
         * @param value the type of param
         */
        int64_t setArgsType(const wstr &name, const irValueType &value);

        void setResultType(const irValueType &value );

        irValueType& getParamType(int64_t i);

        irValueType& getResultType();
    };

    class irVarTable {
        indexTable<wstr, irValueType> fields;
    public:
        int64_t setFieldType(const wstr &name, const irValueType &value);

        irValueType &getFieldType(int64_t i);

        int64_t getFieldIndex(const wstr &name);
    };

    class irTempVarTable {
        vec<irValueType> fields;
    public:
        int64_t put(const irValueType &v);

        irValueType &get(int64_t v);
    };

     class irFuncConstantPool {
         indexPool<irConstVal> fields;
     public:
         int64_t put(const irConstVal &v);

         irConstVal &get(int64_t v);
     };

    class irFunc {
        std::shared_ptr<irFuncType> funcType;
        indexTable<wstr, irBasicBlock> basicBlocks;
        int64_t entryBlock;
    public:
        irVarTable varTable;
        irTempVarTable tempVarTable;
        irFuncConstantPool constPool;

        void setFuncType(const irFuncType &v);

        irFuncType &getFuncType();

        int64_t newBasicBlock(const wstr &name);

        irBasicBlock &getBasicBlock(int64_t i);

        void setEntryBlock(int64_t i);

        int64_t getEntryBlock() const;
    };

    class irFuncTable {
        indexTable<wstr, irFunc> funcs;
    public:
        int64_t putFunc(const wstr &name, const irFunc &func);

        irFunc &getFunc(int64_t i);
    };

    class irModule {
    public:
        irConstantPool constPool;
        irStructTable structTable;
        irFuncTable funcTable;
        irVarTable globals;
    };

    class irValueType {
    public:
        enum class type {
            vInt,
            vDeci,
            vBool,
            vFunc,
            vStruct,
            vPtr
        } tp;

        int64_t typeIndex;
        std::shared_ptr<irValueType> ptrVal;

        void set(type t, int64_t idx);

        void set(const std::shared_ptr<irValueType> &v);
    };

    class irInstructionArg {
    public:
        enum class type {
            vUnknown,
            vConst,
            tempVar,
            bbVar,
            gVar,
        } tp;
        int64_t index;
    };

    class irInstruction {
    public:
        enum class type {
            ret,
            br,
            invoke,
            add,
            sub,
            mul,
            div,
            shl,
            shr,
            brAnd,
            brOr,
            band,
            bor,
            bXor,
            gep,
            store,
            load,
            tempVar,
        } tp;
        vec<irInstructionArg> args;
    };

    class irBasicBlock {
    public:
        vec<irInstruction> inst;

        void putInst(const irInstruction &ins);

    };

    class irContext {
        indexTable<wstr, irModule> modules;
    public:
        int64_t newModule(const wstr &name);

        irModule &getModule(int64_t idx);

        int64_t getModuleIdx(const wstr &name);
    };

    class irBuilder {
        irContext *cxt;
        int64_t moduleId;
        int64_t funcId;
        int64_t bbId;
    public:
        irBuilder(irContext *cxt);

        void setModule(int64_t v);

        void setInsertPoint(int64_t f, int64_t b);

        void putInst(const irInstruction &ins);

        irModule &getModule();

        irFunc &getFunc();
    };
}

#endif //HOSHI_LANG_IR_HPP
