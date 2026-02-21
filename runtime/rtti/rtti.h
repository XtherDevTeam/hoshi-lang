//
// Created by XIaokang00010 on 2025/8/15.
//

#ifndef HOSHI_LANG_RTTI_H
#define HOSHI_LANG_RTTI_H

#include <cstddef>
#include <cstdint>

enum class valueType : uint64_t {
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
    virtualMethod,
    pointerObject,
    interfaceObject,
    none,
    charRaw,
    incompleteTemplateType,
    foreignInt32Type,
    foreignFloatType,
};

struct YoiIntegerObject;
struct YoiStringObject;

struct YoiTypeInfo {
    int64_t type_id;
    const char *type_name;
    valueType type_enum;
    int64_t type_affiliate_module;
    int64_t type_index;
    int64_t is_array;
};

struct YoiTypeInfoObject {
    int64_t gc_refcount;
    int64_t type_info_struct_type_id;
    int64_t type_id_object;
    char *type_name_object;
    int64_t type_enum_object;
    int64_t type_affiliate_module_object;
    int64_t type_index_object;
    int64_t is_array_object;
};

extern "C" YoiTypeInfo rtti_table[];

extern "C" YoiTypeInfoObject *rtti_object_alloc(YoiIntegerObject *type_id_object);

#endif