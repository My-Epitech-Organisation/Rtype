#include <iostream>
#include <memory>

int main() {
    // Bad practice: using raw new/delete
    auto ptr = new int(42);
    std::cout << *ptr << std::endl;
    delete ptr;

    // Another example
    auto arr = new int[10];
    for (int i = 0; i < 10; ++i) {
        arr[i] = i;
    }
    delete[] arr;

    return 0;
}