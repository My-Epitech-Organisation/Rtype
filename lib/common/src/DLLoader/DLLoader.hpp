/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** DLLoader.hpp
*/

#ifndef DLLOADER_HPP_
#define DLLOADER_HPP_

#include <dlfcn.h>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

namespace rtype::common {

template <typename T>
class DLLoader {
   public:
    explicit DLLoader(const std::string& path) : _handle(nullptr) {
        _handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!_handle) {
            throw std::runtime_error("Cannot load library: " + std::string(dlerror()));
        }
    }

    ~DLLoader() {
        if (_handle) {
            dlclose(_handle);
        }
    }

    T* getInstance(const std::string& entryPointName) {
        auto entryPoint = reinterpret_cast<T* (*)()>(dlsym(_handle, entryPointName.c_str()));
        if (!entryPoint) {
            throw std::runtime_error("Cannot load symbol: " + std::string(dlerror()));
        }
        return entryPoint();
    }

   private:
    void* _handle;
};

}  // namespace rtype::common

#endif // DLLOADER_HPP_
