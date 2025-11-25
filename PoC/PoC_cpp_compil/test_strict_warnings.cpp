// Test compilation with -Wall -Wextra -Werror
// This file should compile cleanly without warnings
#include <iostream>
#include <string>
#include <vector>

class TestClass {
private:
    int value_;
    std::string name_;

public:
    TestClass(int value, std::string name)
        : value_(value), name_(std::move(name)) {}

    int getValue() const { return value_; }
    const std::string& getName() const { return name_; }
};

// Function with all parameters used
void process(int count, const std::string& message) {
    for (int i = 0; i < count; ++i) {
        std::cout << message << std::endl;
    }
}

// Template function
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

int main() {
    std::cout << "=== Strict Warnings Test ===" << std::endl;

    TestClass obj(42, "test");
    std::cout << "Object: " << obj.getName()
              << " = " << obj.getValue() << std::endl;

    process(3, "Hello");

    std::cout << "max(10, 20) = " << max(10, 20) << std::endl;

    // Vector operations
    std::vector<int> numbers;
    numbers.reserve(10);
    for (int i = 0; i < 5; ++i) {
        numbers.push_back(i * 2);
    }

    std::cout << "Numbers: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    std::cout << "No warnings generated!" << std::endl;
    return 0;
}
