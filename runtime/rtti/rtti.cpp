#include "rtti.h"
#include <runtime/memory/memory.h>


extern "C" YoiTypeInfoObject *rtti_object_alloc(YoiIntegerObject *type_id_object) {
    auto rtti_entry = rtti_table[type_id_object->value];
    auto *obj = (YoiTypeInfoObject *)runtime_object_alloc(sizeof(YoiTypeInfoObject));
    obj->gc_refcount = 1;
    obj->type_id_object = type_id_object;
    obj->type_name_object = (YoiStringObject *)runtime_object_alloc(sizeof(YoiStringObject));
    obj->type_enum_object = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    obj->type_affiliate_module_object = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    obj->type_index_object = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    obj->is_array_object = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));

    obj->type_name_object->gc_refcount = 1;
    obj->type_name_object->type_id = 4;
    obj->type_name_object->value = (char *)rtti_entry.type_name;

    obj->type_enum_object->gc_refcount = 1;
    obj->type_enum_object->type_id = 0;
    obj->type_enum_object->value = (long long)rtti_entry.type_enum;

    obj->type_affiliate_module_object->gc_refcount = 1;
    obj->type_affiliate_module_object->type_id = 0;
    obj->type_affiliate_module_object->value = rtti_entry.type_affiliate_module;

    obj->type_index_object->gc_refcount = 1;
    obj->type_index_object->type_id = 0;
    obj->type_index_object->value = rtti_entry.type_index;

    obj->is_array_object->gc_refcount = 1;
    obj->is_array_object->type_id = 0;
    obj->is_array_object->value = rtti_entry.is_array;

    return obj;
}
