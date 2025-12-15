/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Components.hpp - Aggregate header for all client components with type aliases
*/

#pragma once

// Include all client components
#include "Components/BoxingComponent.hpp"
#include "Components/ButtonComponent.hpp"
#include "Components/HiddenComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/ParallaxComponent.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/TextInputComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Components/UserEventComponent.hpp"
#include "Components/ZIndexComponent.hpp"

// Include shared components commonly used with client components
#include "../shared/Components/PositionComponent.hpp"
#include "../shared/Components/VelocityComponent.hpp"

namespace rtype::client {

/**
 * @brief Type aliases for commonly used components.
 *
 * These aliases provide shorter names for components, reducing verbosity.
 * Use these in client code for cleaner syntax.
 *
 * Example:
 *   using namespace rtype::client;
 *   registry.emplaceComponent<Sprite>(entity, texture);
 *   registry.emplaceComponent<Position>(entity, 100.0f, 200.0f);
 */

// Visual components
using Sprite = ::rtype::games::rtype::client::Image;
using Rect = ::rtype::games::rtype::client::Rectangle;
using Label = ::rtype::games::rtype::client::Text;
using Depth = ::rtype::games::rtype::client::ZIndex;
using Scale = ::rtype::games::rtype::client::Size;
using TexRect = ::rtype::games::rtype::client::TextureRect;
using BgLayer = ::rtype::games::rtype::client::Parallax;

// UI interaction components
using Interaction = ::rtype::games::rtype::client::UserEvent;
using Hidden = ::rtype::games::rtype::client::HiddenComponent;

// Tag components
using PlayerTag = ::rtype::games::rtype::client::PlayerTag;
using ControllableTag = ::rtype::games::rtype::client::ControllableTag;
using ButtonTag = ::rtype::games::rtype::client::ButtonTag;
using StaticTextTag = ::rtype::games::rtype::client::StaticTextTag;
using PauseMenuTag = ::rtype::games::rtype::client::PauseMenuTag;
using HudTag = ::rtype::games::rtype::client::HudTag;
using TextInputTag = ::rtype::games::rtype::client::TextInputTag;

// Text input component
using TextInput = ::rtype::games::rtype::client::TextInput;

// Position and movement (from shared)
using Position = ::rtype::games::rtype::shared::Position;
using Velocity = ::rtype::games::rtype::shared::VelocityComponent;

}  // namespace rtype::client
