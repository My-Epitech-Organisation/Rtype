/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Snake Game Engine - Implementation
*/

#include "GameEngine.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

#include "../shared/Components.hpp"

namespace rtype::games::snake::server {

using rtype::games::snake::shared::Direction;
using rtype::games::snake::shared::FoodComponent;
using rtype::games::snake::shared::GameStateComponent;
using rtype::games::snake::shared::PlayerInputComponent;
using rtype::games::snake::shared::PositionComponent;
using rtype::games::snake::shared::SnakeHeadComponent;
using rtype::games::snake::shared::SnakeSegmentComponent;
using rtype::games::snake::shared::VelocityComponent;

SnakeGameEngine::SnakeGameEngine(std::shared_ptr<ECS::Registry> registry,
                                 bool autoSpawnInitial)
    : _registry(registry),
      _rng(std::random_device()()),
      _autoSpawnInitial(autoSpawnInitial) {
    if (!_registry) {
        throw std::runtime_error(
            "SnakeGameEngine requires a valid ECS registry");
    }
}

SnakeGameEngine::~SnakeGameEngine() { shutdown(); }

bool SnakeGameEngine::initialize() {
    std::cout << "[SnakeGameEngine] Initializing..." << std::endl;

    try {
        setupSystems();
        if (_autoSpawnInitial) {
            spawnInitialSnakes();
            setRunning(true);
        }
        spawnFood();
        _moveTimer = 0.0F;

        std::cout << "[SnakeGameEngine] Initialization successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SnakeGameEngine] Initialization failed: " << e.what()
                  << std::endl;
        return false;
    }
}

void SnakeGameEngine::setupSystems() {
    _registry->setSingleton<GameStateComponent>();

    std::cout << "[SnakeGameEngine] Systems setup complete" << std::endl;
}

void SnakeGameEngine::spawnInitialSnakes() {
    auto head1 = _registry->spawnEntity();
    _registry->emplaceComponent<SnakeHeadComponent>(
        head1, SnakeHeadComponent{.playerId = 1});
    _registry->emplaceComponent<PositionComponent>(head1, PositionComponent{
                                                              .gridX = 10,
                                                              .gridY = 10,
                                                          });
    _registry->emplaceComponent<VelocityComponent>(head1, VelocityComponent{
                                                              .dirX = 1,
                                                              .dirY = 0,
                                                          });
    _registry->emplaceComponent<PlayerInputComponent>(
        head1, PlayerInputComponent{
                   .playerId = 1,
                   .nextDirection = Direction::RIGHT,
               });

    for (int i = 1; i < SnakeGameConfig::INITIAL_LENGTH; i++) {
        auto segment = _registry->spawnEntity();
        _registry->emplaceComponent<SnakeSegmentComponent>(
            segment, SnakeSegmentComponent{
                         .playerId = 1,
                         .segmentIndex = i,
                     });
        _registry->emplaceComponent<PositionComponent>(segment,
                                                       PositionComponent{
                                                           .gridX = 10 - i,
                                                           .gridY = 15,
                                                       });
    }

    std::cout << "[SnakeGameEngine] Player 1 snake spawned" << std::endl;
}

ECS::Entity SnakeGameEngine::spawnSnakeForPlayer(uint32_t playerId, int startX,
                                                 int startY) {
    auto head = _registry->spawnEntity();
    _registry->emplaceComponent<SnakeHeadComponent>(
        head, SnakeHeadComponent{.playerId = playerId});
    _registry->emplaceComponent<PositionComponent>(
        head, PositionComponent{.gridX = startX, .gridY = startY});
    _registry->emplaceComponent<VelocityComponent>(
        head, VelocityComponent{.dirX = 1, .dirY = 0});
    _registry->emplaceComponent<PlayerInputComponent>(
        head, PlayerInputComponent{.playerId = playerId,
                                   .nextDirection = Direction::NONE});

    for (int i = 1; i < SnakeGameConfig::INITIAL_LENGTH; i++) {
        auto segment = _registry->spawnEntity();
        _registry->emplaceComponent<SnakeSegmentComponent>(
            segment,
            SnakeSegmentComponent{.playerId = playerId, .segmentIndex = i});
        _registry->emplaceComponent<PositionComponent>(
            segment, PositionComponent{.gridX = startX - i, .gridY = startY});
    }

    std::cout << "[SnakeGameEngine] Player " << playerId << " snake spawned"
              << std::endl;
    return head;
}

void SnakeGameEngine::spawnFood() {
    for (int attempt = 0; attempt < 10; attempt++) {
        std::uniform_int_distribution<int> distX(
            0, SnakeGameConfig::GRID_WIDTH - 1);
        std::uniform_int_distribution<int> distY(
            0, SnakeGameConfig::GRID_HEIGHT - 1);
        int foodX = distX(_rng);
        int foodY = distY(_rng);

        bool occupied = false;
        auto anySnake = _registry->view<PositionComponent>();
        anySnake.each([&](ECS::Entity /*id*/, PositionComponent& pos) {
            if (pos.gridX == foodX && pos.gridY == foodY) {
                occupied = true;
            }
        });

        if (occupied) {
            continue;
        }

        auto food = _registry->spawnEntity();
        _registry->emplaceComponent<FoodComponent>(food,
                                                   FoodComponent{.value = 10});
        _registry->emplaceComponent<PositionComponent>(food, PositionComponent{
                                                                 .gridX = foodX,
                                                                 .gridY = foodY,
                                                             });
        return;
    }

    auto food = _registry->spawnEntity();
    _registry->emplaceComponent<FoodComponent>(food,
                                               FoodComponent{.value = 10});
    _registry->emplaceComponent<PositionComponent>(food, PositionComponent{
                                                             .gridX = 0,
                                                             .gridY = 0,
                                                         });
}

void SnakeGameEngine::update(float deltaTime) {
    if (!isRunning()) {
        return;
    }

    _moveTimer += deltaTime;
    if (_moveTimer >= SnakeGameConfig::MOVE_INTERVAL) {
        _moveTimer = 0.0F;

        auto headView =
            _registry->view<SnakeHeadComponent, VelocityComponent,
                            PositionComponent, PlayerInputComponent>();
        headView.each([this](ECS::Entity headId, SnakeHeadComponent& head,
                             VelocityComponent& velocity,
                             PositionComponent& position,
                             PlayerInputComponent& input) {
            if (input.nextDirection != Direction::NONE) {
                bool canMove = true;
                if (input.nextDirection == Direction::UP &&
                    velocity.dirY == 1) {
                    canMove = false;
                } else if (input.nextDirection == Direction::DOWN &&
                           velocity.dirY == -1) {
                    canMove = false;
                } else if (input.nextDirection == Direction::LEFT &&
                           velocity.dirX == 1) {
                    canMove = false;
                } else if (input.nextDirection == Direction::RIGHT &&
                           velocity.dirX == -1) {
                    canMove = false;
                }

                if (canMove) {
                    switch (input.nextDirection) {
                        case Direction::UP:
                            velocity.dirX = 0;
                            velocity.dirY = -1;
                            break;
                        case Direction::DOWN:
                            velocity.dirX = 0;
                            velocity.dirY = 1;
                            break;
                        case Direction::LEFT:
                            velocity.dirX = -1;
                            velocity.dirY = 0;
                            break;
                        case Direction::RIGHT:
                            velocity.dirX = 1;
                            velocity.dirY = 0;
                            break;
                        default:
                            break;
                    }
                }
                input.nextDirection = Direction::NONE;
            }

            PositionComponent prevHeadPos = position;

            position.gridX += velocity.dirX;
            position.gridY += velocity.dirY;

            if (position.gridX < 0 ||
                position.gridX >= SnakeGameConfig::GRID_WIDTH ||
                position.gridY < 0 ||
                position.gridY >= SnakeGameConfig::GRID_HEIGHT) {
                auto& gameState = _registry->getSingleton<GameStateComponent>();
                gameState.isGameOver = true;
                setRunning(false);
                std::cout << "[SnakeGameEngine] GAME OVER - Hit wall!"
                          << std::endl;
                return;
            }

            auto foodView = _registry->view<FoodComponent, PositionComponent>();
            foodView.each([this, &position, headId, &input](
                              ECS::Entity foodId, FoodComponent& food,
                              PositionComponent& foodPos) {
                if (position.gridX == foodPos.gridX &&
                    position.gridY == foodPos.gridY) {
                    _registry->killEntity(foodId);
                    _foodEaten = true;

                    int maxSegmentIndex = -1;
                    ECS::Entity lastSegmentId;
                    auto segmentView =
                        _registry
                            ->view<SnakeSegmentComponent, PositionComponent>();
                    segmentView.each([&maxSegmentIndex, &lastSegmentId, &input](
                                         ECS::Entity segId,
                                         SnakeSegmentComponent& seg,
                                         PositionComponent& /*pos*/) {
                        if (seg.playerId == input.playerId &&
                            seg.segmentIndex > maxSegmentIndex) {
                            maxSegmentIndex = seg.segmentIndex;
                            lastSegmentId = segId;
                        }
                    });

                    if (maxSegmentIndex >= 0) {
                        auto& lastPos =
                            _registry->getComponent<PositionComponent>(
                                lastSegmentId);

                        auto newSegment = _registry->spawnEntity();
                        _registry->emplaceComponent<SnakeSegmentComponent>(
                            newSegment, SnakeSegmentComponent{
                                            .playerId = input.playerId,
                                            .segmentIndex = maxSegmentIndex + 1,
                                        });
                        _registry->emplaceComponent<PositionComponent>(
                            newSegment, PositionComponent{
                                            .gridX = lastPos.gridX,
                                            .gridY = lastPos.gridY,
                                        });
                    }

                    auto& gameState =
                        _registry->getSingleton<GameStateComponent>();
                    gameState.score += food.value;

                    engine::GameEvent eatEvent;
                    eatEvent.type = engine::GameEventType::PowerUpApplied;
                    eatEvent.entityNetworkId = headId.id;
                    emitEvent(eatEvent);
                }
            });

            auto segmentView =
                _registry->view<SnakeSegmentComponent, PositionComponent>();
            bool selfCollision = false;
            segmentView.each([this, &position, &input, &selfCollision](
                                 ECS::Entity segmentId,
                                 SnakeSegmentComponent& seg,
                                 PositionComponent& segPos) {
                if (seg.playerId != input.playerId) {
                    return;
                }
                if (position.gridX == segPos.gridX &&
                    position.gridY == segPos.gridY) {
                    selfCollision = true;
                }
            });

            if (selfCollision) {
                auto& gameState = _registry->getSingleton<GameStateComponent>();
                gameState.isGameOver = true;
                setRunning(false);
                std::cout << "[SnakeGameEngine] GAME OVER - Hit self!"
                          << std::endl;
                return;
            }

            std::vector<std::pair<ECS::Entity, int>> segmentsByIndex;
            auto playerSegments =
                _registry->view<SnakeSegmentComponent, PositionComponent>();
            playerSegments.each(
                [&segmentsByIndex, &input](ECS::Entity segId,
                                           SnakeSegmentComponent& seg,
                                           PositionComponent& /*pos*/) {
                    if (seg.playerId == input.playerId) {
                        segmentsByIndex.emplace_back(segId, seg.segmentIndex);
                    }
                });

            std::sort(segmentsByIndex.begin(), segmentsByIndex.end(),
                      [](const auto& a, const auto& b) {
                          return a.second < b.second;
                      });

            PositionComponent nextPos = prevHeadPos;

            for (auto& [segId, idx] : segmentsByIndex) {
                auto& segPos =
                    _registry->getComponent<PositionComponent>(segId);
                PositionComponent oldSegPos = segPos;
                segPos.gridX = nextPos.gridX;
                segPos.gridY = nextPos.gridY;
                nextPos = oldSegPos;
            }
        });
    }

    if (_foodEaten) {
        spawnFood();
        _foodEaten = false;
    }

    auto foodCheckView = _registry->view<FoodComponent>();
    int foodCount = 0;
    foodCheckView.each([&foodCount](ECS::Entity /*id*/,
                                    FoodComponent& /*food*/) { foodCount++; });

    if (foodCount == 0) {
        spawnFood();
    }

    auto headViewWin = _registry->view<SnakeHeadComponent, PositionComponent,
                                       PlayerInputComponent>();
    headViewWin.each([this](ECS::Entity headId, SnakeHeadComponent& /*head*/,
                            PositionComponent& /*pos*/,
                            PlayerInputComponent& input) {
        int snakeSize = 1;
        auto segmentViewWin = _registry->view<SnakeSegmentComponent>();
        segmentViewWin.each([&snakeSize, &input](ECS::Entity /*id*/,
                                                 SnakeSegmentComponent& seg) {
            if (seg.playerId == input.playerId) {
                snakeSize++;
            }
        });

        int maxSize =
            (SnakeGameConfig::GRID_WIDTH * SnakeGameConfig::GRID_HEIGHT) - 1;
        if (snakeSize >= maxSize) {
            auto& gameState = _registry->getSingleton<GameStateComponent>();
            gameState.isGameOver = true;
            setRunning(false);
            std::cout << "[SnakeGameEngine] YOU WIN! Snake size: " << snakeSize
                      << " / " << maxSize << std::endl;
        }
    });
}

void SnakeGameEngine::shutdown() {
    std::cout << "[SnakeGameEngine] Shutting down..." << std::endl;
    setRunning(false);
}

void SnakeGameEngine::startGame() { setRunning(true); }

void SnakeGameEngine::stopGame() { setRunning(false); }

engine::ProcessedEvent SnakeGameEngine::processEvent(
    const engine::GameEvent& event) {
    engine::ProcessedEvent result;
    result.type = event.type;
    result.valid = false;

    return result;
}

void SnakeGameEngine::syncEntityPositions(
    std::function<void(uint32_t, float, float, float, float)> callback) {
    auto headView = _registry->view<SnakeHeadComponent, PositionComponent>();
    headView.each([&callback](ECS::Entity headId, SnakeHeadComponent& /*head*/,
                              PositionComponent& pos) {
        callback(headId.id, static_cast<float>(pos.gridX),
                 static_cast<float>(pos.gridY), 0.0F, 0.0F);
    });

    auto segmentView =
        _registry->view<SnakeSegmentComponent, PositionComponent>();
    segmentView.each([&callback](ECS::Entity segmentId,
                                 SnakeSegmentComponent& /*seg*/,
                                 PositionComponent& pos) {
        callback(segmentId.id, static_cast<float>(pos.gridX),
                 static_cast<float>(pos.gridY), 0.0F, 0.0F);
    });

    auto foodView = _registry->view<FoodComponent, PositionComponent>();
    foodView.each([&callback](ECS::Entity foodId, FoodComponent& /*food*/,
                              PositionComponent& pos) {
        callback(foodId.id, static_cast<float>(pos.gridX),
                 static_cast<float>(pos.gridY), 0.0F, 0.0F);
    });
}

void registerSnakeGameEngine() {
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    rtype::engine::GameEngineFactory::registerGame(
        "snake", [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<SnakeGameEngine>(std::move(registry));
        });
}

}  // namespace rtype::games::snake::server
