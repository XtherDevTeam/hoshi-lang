#include "rtti.h"
#include <runtime/memory/memory.h>


extern "C" YoiTypeInfoObject *rtti_object_alloc(YoiIntegerObject *type_id_object) {
    auto rtti_entry = rtti_table[type_id_object->value];
    auto *obj = (YoiTypeInfoObject *)runtime_object_alloc(sizeof(YoiTypeInfoObject));
    obj->gc_refcount = 1;

    obj->type_name_object = (char *)rtti_entry.type_name;

    obj->type_enum_object = (long long)rtti_entry.type_enum;

    obj->type_affiliate_module_object = (long long)rtti_entry.type_affiliate_module;

    obj->type_index_object = (long long)rtti_entry.type_index;

    obj->is_array_object = (long long)rtti_entry.is_array;
    return obj;
}
