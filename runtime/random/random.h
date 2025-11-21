//
// Created by XIaokang00010 on 2025/11/21.
//

#ifndef HOSHI_LANG_RANDOM_H
#define HOSHI_LANG_RANDOM_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" void runtime_random_legacy_set_seed(uint64_t seed);

extern "C" uint64_t runtime_random_legacy_unsigned();

#endif //HOSHI_LANG_RANDOM_H