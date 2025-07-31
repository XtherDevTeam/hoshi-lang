//
// Created by XIaokang00010 on 2025/7/31.
//

#include "string.h" 

YoiIntegerObject *basic_string_length(YoiStringObject *obj) {
    auto *length = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    basic_int_gc_refcount_increase(length);
    length->value = static_cast<int64_t>(std::wstring_view(obj->value).length());
return length;
}
YoiStringObject *basic_string_add(YoiStringObject *obj1, YoiStringObject *obj2) {
    auto *result = (YoiStringObject *)runtime_object_alloc(sizeof(YoiStringObject));
    basic_string_gc_refcount_increase(result);
    result->value = (wchar_t *)runtime_object_alloc(
        sizeof(wchar_t) *
        (std::wstring_view(obj1->value).length() + std::wstring_view(obj2->value).length() + 1));
    std::wcscpy(result->value, obj1->value);
    std::wcscat(result->value, obj2->value);
    return result;
}
YoiCharObject *basic_string_at(YoiStringObject *obj, YoiIntegerObject *index) {
    auto *result = (YoiCharObject *)runtime_object_alloc(sizeof(YoiCharObject));
    basic_char_gc_refcount_increase(result);
    result->value = obj->value[index->value];
    return result;
}
YoiStringObject *
basic_string_slice(YoiStringObject *obj, YoiIntegerObject *start, YoiIntegerObject *end) {
    auto *result = (YoiStringObject *)runtime_object_alloc(sizeof(YoiStringObject));
    basic_string_gc_refcount_increase(result);
    result->value =
        (wchar_t *)runtime_object_alloc(sizeof(wchar_t) * (end->value - start->value + 1));
    std::wmemcpy(result->value, obj->value + start->value, end->value - start->value + 1);
    return result;
}
