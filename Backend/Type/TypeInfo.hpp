//
// Created by theflysong on 2022/10/4.
//

#ifndef XSCRIPT2_TYPEINFO_HPP
#define XSCRIPT2_TYPEINFO_HPP

#include <Utils.hpp>

namespace Hoshi {
    class TypeInfo {
    public:
        virtual const XString GetTypeName() = 0;

        virtual const XSize GetSize() = 0;
    };

    class Type {
    public:
        enum class TTypes {
            Basic = 0,
            Undefined,
            Interface,
            Struct,
            Array,
            Class
        };
    private:
        const TTypes TType;
        const TypeInfo *Info;
    public:
        Type(TTypes TType, const TypeInfo *Info);

        ~Type();

        const TypeInfo *GetInfo();

        TTypes GetType();
    };

    class BasicTypeInfo : public TypeInfo {
    public:
        enum class BasicTypes {
            BYTE, // 1B
            SCHAR, // 1B
            CHAR, // 2B
            SHORT, // 2B
            USHORT, // 2B
            SINT, // 4B
            USINT, // 4B
            INT, // 8B
            UINT, // 8B
            FLOAT, // 4B
            REAL // 8B
        };
    private:
        const BasicTypes BasicType;
    public:
        BasicTypeInfo(BasicTypes BasicType);

        BasicTypes GetType();

        const XString GetTypeName() override;

        const XSize GetSize() override;
    };

    class UndefinedTypeInfo : public TypeInfo {
    public:
        UndefinedTypeInfo();

        const XString GetTypeName() override;

        const XSize GetSize() override;
    };

    //TODO:
    class InterfaceTypeInfo : public TypeInfo {
    public:
    };

    class StructTypeInfo : public TypeInfo {
    public:
    };

    class ArrayTypeInfo : public TypeInfo {
    public:
    };

    class ClassTypeInfo : public TypeInfo {
    public:
    };
}

#endif