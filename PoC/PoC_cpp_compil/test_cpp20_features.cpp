// Test C++20 features compilation with strict flags
#include <iostream>
#include <vector>
#include <ranges>
#include <concepts>
#include <span>

// Test concepts
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// Test ranges
void test_ranges() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};

    auto even = numbers | std::views::filter([](int n) { return n % 2 == 0; });

    std::cout << "Even numbers: ";
    for (int n : even) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

// Test span
void process_data(std::span<const int> data) {
    std::cout << "Processing " << data.size() << " elements" << std::endl;
    for (int value : data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== C++20 Features Test ===" << std::endl;

    // Test concepts
    std::cout << "5 + 3 = " << add(5, 3) << std::endl;
    std::cout << "2.5 + 3.7 = " << add(2.5, 3.7) << std::endl;

    // Test ranges
    test_ranges();

    // Test span
    std::vector<int> data = {10, 20, 30, 40};
    process_data(data);

    std::cout << "All C++20 features working!" << std::endl;
    return 0;
}
