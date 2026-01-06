/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** IScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ISCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ISCENE_HPP_
#include <memory>
#include "../../../../../include/rtype/display/IDisplay.hpp"

class IScene {
   public:
    virtual void pollEvents(const rtype::display::Event& e) = 0;
    virtual void update(float dt) = 0;
    virtual void render(std::shared_ptr<rtype::display::IDisplay> display) = 0;

    virtual ~IScene() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_ISCENE_HPP_
