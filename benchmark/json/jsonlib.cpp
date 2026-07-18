#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>

// Use the library namespace
using json = nlohmann::json;

// Equivalent to the iteration function
void iteration(const std::string& s) {
    try {
        // Parse the string into a json object
        auto j = json::parse(s);
    } catch (json::parse_error& e) {
        // Handle potential parse errors (optional, for safety)
    }
}

int main() {
    const std::string path = "benchmark/json/test.json";
    
    // 1. Open the file
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << path << std::endl;
        return 1;
    }

    // 2. Read file content into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string s = buffer.str();
    
    // 3. Close the file (handled automatically by ifstream destructor, 
    // but explicit call mimics the original code)
    file.close();

    // 4. Print the string
    std::cout << s << "\n";

    // 5. Benchmark loop
    for (int i = 0; i < 1000000; ++i) {
        iteration(s);
    }

    return 0;
}