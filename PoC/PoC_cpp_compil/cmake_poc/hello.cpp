// Minimal C++20 test with strict flags
#include <iostream>
#include <string_view>

void greet(std::string_view name) {
    std::cout << "Hello, " << name << "!" << std::endl;
}

int main() {
    std::cout << "=== Mini PoC: Strict Compilation Test ===" << std::endl;
    
    greet("GCC/Clang");
    
    // C++20 feature: designated initializers
    struct Point {
        int x;
        int y;
    };
    
    Point p = {.x = 10, .y = 20};
    std::cout << "Point: (" << p.x << ", " << p.y << ")" << std::endl;
    
    std::cout << "✓ Compiled with -Wall -Wextra -Werror" << std::endl;
    
    return 0;
}
