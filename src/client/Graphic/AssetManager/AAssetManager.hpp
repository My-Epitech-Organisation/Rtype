/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** AAssetManager
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_AASSETMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_AASSETMANAGER_HPP_

#include <unordered_map>
#include <utility>

#include "IAssetManager.hpp"
#include "Macros.hpp"
#include "rtype/display/IDisplay.hpp"

template <typename T>
class AAssetManager : public IAssetManager<T> {
   protected:
    ::rtype::display::IDisplay* _display;
    std::string _typeName;
    std::unordered_map<std::string, std::shared_ptr<T>> _assets;

   public:
    [[nodiscard]] bool isLoaded(const std::string& id) const override;
    bool unload(const std::string& id) override;
    void unloadAll() override;
    [[nodiscard]] std::size_t size() const override;

    std::shared_ptr<T> get(const std::string& id) override;
    AAssetManager(::rtype::display::IDisplay* display, std::string typeName)
        : _display(display), _typeName(std::move(typeName)) {}
    virtual ~AAssetManager() = default;
};

template <typename T>
bool AAssetManager<T>::isLoaded(const std::string& id) const {
    return this->_assets.find(id) != this->_assets.end();
}

template <typename T>
bool AAssetManager<T>::unload(const std::string& id) {
    auto it = this->_assets.find(id);
    if (it == this->_assets.end()) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                      (this->_typeName + " not found for unloading: ") << id);
        return false;
    }
    this->_assets.erase(it);
    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                  (this->_typeName + " unloaded: ") << id);
    return true;
}

template <typename T>
void AAssetManager<T>::unloadAll() {
    std::size_t count = this->_assets.size();
    this->_assets.clear();
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::Graphics,
        "All " + this->_typeName + " unloaded (" << count << " fonts)");
}

template <typename T>
std::size_t AAssetManager<T>::size() const {
    return this->_assets.size();
}

template <typename T>
std::shared_ptr<T> AAssetManager<T>::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        LOG_ERROR_CAT(::rtype::LogCategory::Graphics,
                      (this->_typeName + " not found: ") << id);
        throw std::out_of_range(this->_typeName + " not found: " + id);
    }

    return it->second;
}

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_AASSETMANAGER_HPP_
