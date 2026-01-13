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
#include <utility>

namespace rtype::common {

template <typename T>
class DLLoader {
   public:
    explicit DLLoader(const std::string& path) : _handle(nullptr) {
        #ifdef _WIN32
            this->_handle = LoadLibraryA(path.c_str());
            if (!this->_handle) {
                LPVOID lpMsgBuf;
                DWORD errorMessageID = GetLastError();
                LPSTR messageBuffer = nullptr;

                size_t size = FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

                std::string message(messageBuffer, size);
                LocalFree(messageBuffer);
                throw std::runtime_error("Cannot load library: " + message);
            }
        #else
            this->_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_NODELETE);
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

    template <typename... Args>
    T* getInstance(const std::string& entryPointName, Args&&... args) {
        using FunctionType = T* (*)(Args...);

        #ifdef _WIN32
            auto entryPoint = reinterpret_cast<FunctionType>(
                GetProcAddress(static_cast<HMODULE>(this->_handle), entryPointName.c_str())
            );

            if (!entryPoint) {
                // ... (ton code de gestion d'erreur Windows reste identique) ...
                DWORD errorMessageID = GetLastError();
                LPSTR messageBuffer = nullptr;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                             NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                std::string message(messageBuffer, size);
                LocalFree(messageBuffer);
                throw std::runtime_error("Cannot load symbol: " + message);
            }
        #else
            dlerror();

            auto entryPoint = reinterpret_cast<FunctionType>(
                dlsym(this->_handle, entryPointName.c_str())
            );

            const char *dlsym_error = dlerror();
            if (dlsym_error) {
                throw std::runtime_error("Cannot load symbol: " + std::string(dlsym_error));
            }
            if (!entryPoint) {
                throw std::runtime_error("Cannot load symbol: " + entryPointName);
            }
        #endif
        return entryPoint(std::forward<Args>(args)...);
    }

   private:
    void* _handle;
};

}  // namespace rtype::common

#endif // DLLOADER_HPP_
