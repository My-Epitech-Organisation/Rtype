/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** DLLoader.hpp
*/

#ifndef DLLOADER_HPP_
#define DLLOADER_HPP_

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

namespace rtype::common {

template <typename T>
class DLLoader {
   public:
    explicit DLLoader(const std::string& path) : _handle(nullptr) {
        #ifdef _WIN32
            this->_handle = LoadLibraryA(path.c_str())
            if (!this->_handle) {
                LPVOID lpMsgBuf;
                throw std::runtime_error("Cannot load library: " + std::string(formatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                0, NULL)));
            }
        #else
            this->_handle = dlopen(path.c_str(), RTLD_LAZY);
            if (!this->_handle) {
                throw std::runtime_error("Cannot load library: " + std::string(dlerror()));
            }
        #endif
    }

    ~DLLoader() {
        if (this->_handle) {
            #ifdef _WIN32
                FreeLibrary(static_cast<HMODULE>(this->_handle));
            #else
                dlclose(this->_handle);
            #endif
        }
    }

    T* getInstance(const std::string& entryPointName) {
        #ifdef _WIN32
            auto entryPoint = reinterpret_cast<T* (*)()>(GetProcAddress(static_cast<HMODULE>(this->_handle), entryPointName.c_str()));
            if (!entryPoint) {
                LPVOID lpMsgBuf;
                throw std::runtime_error("Cannot load symbol: " + std::string(formatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                0, NULL)));
            }
        #else
            auto entryPoint = reinterpret_cast<T* (*)()>(dlsym(this->_handle, entryPointName.c_str()));
            if (!entryPoint) {
                throw std::runtime_error("Cannot load symbol: " + std::string(dlerror()));
            }
        #endif
        return entryPoint();
    }

   private:
    void* _handle;
};

}  // namespace rtype::common

#endif // DLLOADER_HPP_
