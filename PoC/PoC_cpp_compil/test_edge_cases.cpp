// Test edge cases and potential warning sources
#include <iostream>
#include <cstring>
#include <vector>

// Test signed/unsigned comparisons (properly handled)
void test_comparisons() {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    for (std::size_t i = 0; i < vec.size(); ++i) {
        std::cout << "vec[" << i << "] = " << vec[i] << std::endl;
    }
}

// Test string operations
void test_strings() {
    const char* str1 = "Hello";
    const char* str2 = "World";

    if (std::strcmp(str1, str2) != 0) {
        std::cout << "Strings are different" << std::endl;
    }
}

// Test switch with all cases handled
enum class Color { Red, Green, Blue };

void test_switch(Color color) {
    switch (color) {
        case Color::Red:
            std::cout << "Red" << std::endl;
            break;
        case Color::Green:
            std::cout << "Green" << std::endl;
            break;
        case Color::Blue:
            std::cout << "Blue" << std::endl;
            break;
    }
}

// Test proper initialization
struct Data {
    int x;
    int y;

    Data() : x(0), y(0) {}
    Data(int x_val, int y_val) : x(x_val), y(y_val) {}
};

int main() {
    std::cout << "=== Edge Cases Test ===" << std::endl;

    test_comparisons();
    test_strings();
    test_switch(Color::Green);

    Data d1;
    Data d2(10, 20);

    std::cout << "d1: (" << d1.x << ", " << d1.y << ")" << std::endl;
    std::cout << "d2: (" << d2.x << ", " << d2.y << ")" << std::endl;

    std::cout << "All edge cases handled!" << std::endl;
    return 0;
}
