/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_

#include <memory>
#include <vector>

#include "AudioLib/AudioLib.hpp"
#include "ECS.hpp"
#include "Graphic/AssetManager/AssetManager.hpp"
#include "IScene.hpp"

class AScene : public IScene {
   protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetsManager;
    std::shared_ptr<rtype::display::IDisplay> _window;
    std::shared_ptr<AudioLib> _audio;
    std::vector<ECS::Entity> _listEntity;

   public:
    void pollEvents(const rtype::display::Event& e) override = 0;
    void update(float dt) override = 0;
    void render(std::shared_ptr<rtype::display::IDisplay> window) override = 0;

    explicit AScene(std::shared_ptr<ECS::Registry> registry,
                    std::shared_ptr<AssetManager> assetsManager,
                    std::shared_ptr<rtype::display::IDisplay> window,
                    std::shared_ptr<AudioLib> audio)
        : _registry(registry),
          _assetsManager(assetsManager),
          _window(window),
          _audio(audio) {}
    ~AScene() override {
        for (auto& entity : this->_listEntity) {
            this->_registry->killEntity(entity);
        }
    };
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ASCENE_HPP_
