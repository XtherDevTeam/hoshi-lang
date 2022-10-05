//
// Created by theflysong on 2022/10/4.
//

#include <Type/TypeInfo.hpp>

namespace Hoshi {
    Type::Type(const TTypes TType, const TypeInfo *Info)
            : TType(TType), Info(Info) {
    }

    Type::~Type() {
        if (Info != nullptr) {
            delete Info;
        }
    }

    const TypeInfo *Type::GetInfo() {
        return Info;
    }

    Type::TTypes Type::GetType() {
        return TType;
    }

    BasicTypeInfo::BasicTypeInfo(BasicTypes BasicType)
            : BasicType(BasicType) {
    }

    BasicTypeInfo::BasicTypes BasicTypeInfo::GetType() {
        return BasicType;
    }

    const XString BasicTypeInfo::GetTypeName() {
        switch (BasicType) {
            case BasicTypes::BYTE:
                return L"byte";
            case BasicTypes::SCHAR:
                return L"schar";
            case BasicTypes::CHAR:
                return L"char";
            case BasicTypes::SHORT:
                return L"short";
            case BasicTypes::USHORT:
                return L"ushort";
            case BasicTypes::SINT:
                return L"sint";
            case BasicTypes::USINT:
                return L"usint";
            case BasicTypes::INT:
                return L"int";
            case BasicTypes::UINT:
                return L"uint";
            case BasicTypes::FLOAT:
                return L"float";
            case BasicTypes::REAL:
                return L"real";
            default:
                return L"unknown";
        }
    }

    const XSize BasicTypeInfo::GetSize() {
        switch (BasicType) {
            case BasicTypes::BYTE:
            case BasicTypes::SCHAR:
                return 1;
            case BasicTypes::CHAR:
            case BasicTypes::SHORT:
            case BasicTypes::USHORT:
                return 2;
            case BasicTypes::SINT:
            case BasicTypes::USINT:
            case BasicTypes::FLOAT:
                return 4;
            case BasicTypes::INT:
            case BasicTypes::UINT:
            case BasicTypes::REAL:
                return 8;
            default:
                return 0;
        }
    }

    UndefinedTypeInfo::UndefinedTypeInfo() = default;

    const XString UndefinedTypeInfo::GetTypeName() {
        return L"undefined";
    }

    const XSize UndefinedTypeInfo::GetSize() {
        return 0;
    }
}