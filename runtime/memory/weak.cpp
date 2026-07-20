//
// Created by XIaokang00010 on 2026/7/20.
//

#include <mimalloc/include/mimalloc.h>
#include "weak.h"

WeakSlot *runtime_weak_slot_alloc(void *target, WeakSlot **weak_slots_head_ptr) {
    auto *slot = static_cast<WeakSlot *>(mi_calloc(1, sizeof(WeakSlot)));
    slot->target = target;
    // Prepend to target's linked list
    slot->next_in_target = *weak_slots_head_ptr;
    *weak_slots_head_ptr = slot;
    return slot;
}

void runtime_weak_slot_free(WeakSlot *slot, WeakSlot **weak_slots_head_ptr) {
    if (!slot) return;
    // Unlink from target's linked list
    if (*weak_slots_head_ptr == slot) {
        *weak_slots_head_ptr = slot->next_in_target;
    } else {
        auto *cur = *weak_slots_head_ptr;
        while (cur && cur->next_in_target != slot) cur = cur->next_in_target;
        if (cur) cur->next_in_target = slot->next_in_target;
    }
    mi_free(slot);
}

void runtime_weak_slot_nullify_all(WeakSlot **weak_slots_head_ptr) {
    auto *slot = *weak_slots_head_ptr;
    while (slot) {
        slot->target = nullptr;
        auto *next = slot->next_in_target;
        slot->next_in_target = nullptr;
        slot = next;
    }
    *weak_slots_head_ptr = nullptr;
}
