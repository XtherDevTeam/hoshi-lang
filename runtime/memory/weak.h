//
// Created by XIaokang00010 on 2026/7/20.
//

#ifndef HOSHI_LANG_WEAK_H
#define HOSHI_LANG_WEAK_H

#include <cstdint>

struct WeakSlot {
    void *target;
    WeakSlot *next_in_target;
};

#ifdef __cplusplus
extern "C" {
#endif

    WeakSlot *runtime_weak_slot_alloc(void *target, WeakSlot **weak_slots_head_ptr);
    void runtime_weak_slot_free(WeakSlot *slot, WeakSlot **weak_slots_head_ptr);
    void runtime_weak_slot_nullify_all(WeakSlot **weak_slots_head_ptr);

#ifdef __cplusplus
}
#endif

#endif //HOSHI_LANG_WEAK_H
