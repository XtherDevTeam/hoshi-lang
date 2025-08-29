#include <iostream>

int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n-1) + fib(n-2);
}

int main() {
    fib(40);
    return 0;
}