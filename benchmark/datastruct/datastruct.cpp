#include <iostream>
#include <chrono>

struct Point {
    long x, y, z;
};

int main() {
    const int iterations = 10000000;
    long total_sum = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // Allocated on stack, zero overhead
        Point p { (long)i, 1, 1 }; 
        total_sum += p.x + p.y + p.z;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "C++ (Value Type/Stack): " << diff.count() << "s" << std::endl;
    std::cout << "Final Sum: " << total_sum << std::endl;

    return 0;
}