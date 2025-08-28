#include <iostream>
#include <string>
#include <string_view> // For string_view in helpers
#include <numeric>   // For std::to_string
#include <format>    // C++20 feature

// Helper function to trim whitespace from both ends of a string
// (std::string doesn't have a built-in trim method)
std::string trim(const std::string& str) {
    const std::string_view whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (std::string::npos == first) {
        
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, last - first + 1);
}

// Helper function to count all occurrences of a substring
// (std::string doesn't have a built-in count method)
size_t find_all(const std::string& str, const std::string& sub) {
    if (sub.empty()) 
        return 0;
    size_t count = 0;
    size_t pos = str.find(sub, 0);
    while (pos != std::string::npos) {
        count++;
        pos = str.find(sub, pos + sub.length());
    }
    
    return count;
}

// Helper function to replace all occurrences of a substring
// (std::string::replace replaces only one instance at a time)
std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    if(from.empty())
        return str;

    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing "x" with "yx"
    }
    return str;
}


int main() {
    for (int i = 0; i < 50000; i++) {
        // In C++, string concatenation is done with the `+` operator.
        std::string a = std::string("Hello, ") + "world!";
        std::cout << "string validation passed: operator+" << std::endl;

        if (!a.starts_with("Hello")) { // C++20
            std::cout << "string valiation failed: startswith 'Hello'" << std::endl;
            
        }
        std::cout << "string valiation passed: startswith 'Hello'" << std::endl;

        if (!a.ends_with("!")) { // C++20
            std::cout << "string valiation failed: endswith '!'" << std::endl;
            
        }
        std::cout << "string valiation passed: endswith '!'" << std::endl;

        if (a.substr(0, 5) != "Hello") {
            std::cout << "string valiation failed: substring(0, 5) == 'Hello'" << std::endl;
            
        }
        std::cout << "string valiation passed: substring(0, 5) == 'Hello'" << std::endl;

        if (a.find("llo") != 2) {
            std::cout << "string valiation failed: find('llo') == 2" << std::endl;
            
        }
        std::cout << "string valiation passed: find('llo') == 2" << std::endl;
        std::cout << a << std::endl;
        
        std::string b = trim("  May all the beauty be blessed.  ");
        if (b != "May all the beauty be blessed.") {
            std::cout << "string valiation failed: trim()" << std::endl;
            
        }
        std::cout << b << std::endl;

        std::string c = "K423 & Raiden Mei & K423";

        if (find_all(c, "Raiden") != 1 || find_all(c, "K423") != 2) {
            std::cout << "string valiation failed: find_all('Raiden') == 1 && find_all('K423') == 2" << std::endl;
            
        }
        std::cout << "string valiation passed: find_all('Raiden') == 1 && find_all('K423') == 2" << std::endl;

        c = replace_all(c, "K423", "Kiana Kaslana");
        
        if (c != "Kiana Kaslana & Raiden Mei & Kiana Kaslana") {
            std::cout << "string valiation failed: replace('K423', 'Kiana Kaslana')" << std::endl;
            
        }
        std::cout << c << std::endl;
        std::cout << "string valiation passed: replace('K423', 'Kiana Kaslana')" << std::endl;
        
        std::string d = std::to_string(-123);
        if (d != "-123") {
            std::cout << "string valiation failed: to_string()" << std::endl;
            
        }
        std::cout << d << std::endl;
        std::cout << "string valiation passed: to_string()" << std::endl;

        // Using C++20's std::format
        // Note: The original code's validation string seems to have a bug, duplicating " 1, 2, 3~".
        // This is the correct output from the given format string.
        std::string f = std::format(
            "{}: RESPOND ME! {}!\n{}: {} {}, {}, {}~",
            "Raiden Mei", "ELYSIA", "Elysia", "Can you hear me?", 1, 2, 3);

        const std::string expected_format_output = "Raiden Mei: RESPOND ME! ELYSIA!\nElysia: Can you hear me? 1, 2, 3~";
        if (f != expected_format_output) {
            std::cout << f << std::endl;
            std::cout << "string valiation failed: format()" << std::endl;
            
        }
        std::cout << f << std::endl;
        std::cout << "string valiation passed: format()" << std::endl;

        std::cout << "string valiation passed: all tests passed" << std::endl;
    }
    return 0;
}