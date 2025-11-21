//
// Created by XIaokang00010 on 2025/11/21.
//

#include <cstdint>
#include <runtime/random/random.h>

extern "C" uint64_t runtime_random_legacy_unsigned() {
    union X {
        int32_t s[2];
        uint64_t u;
        X() : s{(int32_t) rand(), (int32_t) rand()} {}
    };
    return X().u;
}

extern "C" void runtime_random_legacy_set_seed(uint64_t seed) {
    srand(seed);
}
