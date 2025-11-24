/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Scripted Movement PoC
*/

#ifndef SCRIPTED_MOVEMENT_HPP
    #define SCRIPTED_MOVEMENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace Movement {

    /**
     * @brief Position component
     */
    struct Position {
        float x = 0.0f;
        float y = 0.0f;

        Position() = default;
        Position(float x_, float y_) : x(x_), y(y_) {}
    };

    /**
     * @brief Base class for movement commands
     */
    class IMovementCommand {
    public:
        virtual ~IMovementCommand() = default;
        virtual void execute(Position& pos, float deltaTime) = 0;
        virtual bool isComplete() const = 0;
        virtual void reset() = 0;
        virtual std::string getName() const = 0;
    };

    /**
     * @brief Linear movement command
     * Example: Move(Linear, Speed=100, DirX=1, DirY=0)
     */
    class LinearCommand : public IMovementCommand {
    private:
        float speed;
        float dirX, dirY;

    public:
        LinearCommand(float spd, float dx, float dy)
            : speed(spd), dirX(dx), dirY(dy) {}

        void execute(Position& pos, float deltaTime) override {
            pos.x += dirX * speed * deltaTime;
            pos.y += dirY * speed * deltaTime;
        }

        bool isComplete() const override {
            return false; // Continuous movement
        }

        void reset() override {}

        std::string getName() const override {
            return "Linear(speed=" + std::to_string(speed) + ")";
        }
    };

    /**
     * @brief Wait/delay command
     * Example: Wait(Duration=2.0)
     */
    class WaitCommand : public IMovementCommand {
    private:
        float duration;
        float elapsed = 0.0f;

    public:
        WaitCommand(float dur) : duration(dur) {}

        void execute(Position& pos, float deltaTime) override {
            elapsed += deltaTime;
        }

        bool isComplete() const override {
            return elapsed >= duration;
        }

        void reset() override {
            elapsed = 0.0f;
        }

        std::string getName() const override {
            return "Wait(duration=" + std::to_string(duration) + ")";
        }
    };

    /**
     * @brief Move to specific position command
     * Example: MoveTo(X=100, Y=50, Speed=50)
     */
    class MoveToCommand : public IMovementCommand {
    private:
        float targetX, targetY;
        float speed;
        bool reached = false;

    public:
        MoveToCommand(float x, float y, float spd)
            : targetX(x), targetY(y), speed(spd) {}

        void execute(Position& pos, float deltaTime) override {
            if (reached) return;

            float dx = targetX - pos.x;
            float dy = targetY - pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < speed * deltaTime) {
                pos.x = targetX;
                pos.y = targetY;
                reached = true;
            } else {
                float dirX = dx / distance;
                float dirY = dy / distance;
                pos.x += dirX * speed * deltaTime;
                pos.y += dirY * speed * deltaTime;
            }
        }

        bool isComplete() const override {
            return reached;
        }

        void reset() override {
            reached = false;
        }

        std::string getName() const override {
            return "MoveTo(x=" + std::to_string(targetX) + ", y=" + std::to_string(targetY) + ")";
        }
    };

    /**
     * @brief Script component holding movement commands
     */
    struct MovementScript {
        std::vector<std::unique_ptr<IMovementCommand>> commands;
        size_t currentCommand = 0;
        bool loop = false;

        MovementScript() = default;

        void addCommand(std::unique_ptr<IMovementCommand> cmd) {
            commands.push_back(std::move(cmd));
        }

        bool isComplete() const {
            return currentCommand >= commands.size();
        }

        void reset() {
            currentCommand = 0;
            for (auto& cmd : commands) {
                cmd->reset();
            }
        }
    };

    /**
     * @brief Parser for movement scripts
     * Format examples:
     *   Move(Linear, Speed=100, DirX=1, DirY=0)
     *   Wait(Duration=2.0)
     *   MoveTo(X=100, Y=50, Speed=50)
     */
    class ScriptParser {
    public:
        static std::unique_ptr<MovementScript> parseFile(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open script file: " + filename);
            }

            auto script = std::make_unique<MovementScript>();
            std::string line;

            while (std::getline(file, line)) {
                // Skip empty lines and comments
                line = trim(line);
                if (line.empty() || line[0] == '#') continue;

                auto cmd = parseLine(line);
                if (cmd) {
                    script->addCommand(std::move(cmd));
                }
            }

            return script;
        }

        static std::unique_ptr<MovementScript> parseString(const std::string& scriptText) {
            auto script = std::make_unique<MovementScript>();
            std::istringstream stream(scriptText);
            std::string line;

            while (std::getline(stream, line)) {
                line = trim(line);
                if (line.empty() || line[0] == '#') continue;

                auto cmd = parseLine(line);
                if (cmd) {
                    script->addCommand(std::move(cmd));
                }
            }

            return script;
        }

    private:
        static std::string trim(const std::string& str) {
            size_t start = str.find_first_not_of(" \t\r\n");
            size_t end = str.find_last_not_of(" \t\r\n");
            return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
        }

        static float parseFloat(const std::string& str) {
            return std::stof(str);
        }

        static std::unique_ptr<IMovementCommand> parseLine(const std::string& line) {
            size_t openParen = line.find('(');
            size_t closeParen = line.find(')');

            if (openParen == std::string::npos || closeParen == std::string::npos) {
                return nullptr;
            }

            std::string cmdType = trim(line.substr(0, openParen));
            std::string params = line.substr(openParen + 1, closeParen - openParen - 1);

            auto paramMap = parseParams(params);

            if (cmdType == "Move" && paramMap.count("Type") && paramMap["Type"] == "Linear") {
                float speed = parseFloat(paramMap["Speed"]);
                float dirX = parseFloat(paramMap["DirX"]);
                float dirY = parseFloat(paramMap["DirY"]);
                return std::make_unique<LinearCommand>(speed, dirX, dirY);
            } else if (cmdType == "Wait") {
                float duration = parseFloat(paramMap["Duration"]);
                return std::make_unique<WaitCommand>(duration);
            } else if (cmdType == "MoveTo") {
                float x = parseFloat(paramMap["X"]);
                float y = parseFloat(paramMap["Y"]);
                float speed = parseFloat(paramMap["Speed"]);
                return std::make_unique<MoveToCommand>(x, y, speed);
            }

            return nullptr;
        }

        static std::unordered_map<std::string, std::string> parseParams(const std::string& params) {
            std::unordered_map<std::string, std::string> result;
            std::istringstream stream(params);
            std::string token;

            while (std::getline(stream, token, ',')) {
                token = trim(token);
                size_t eq = token.find('=');
                if (eq != std::string::npos) {
                    std::string key = trim(token.substr(0, eq));
                    std::string value = trim(token.substr(eq + 1));
                    result[key] = value;
                }
            }

            return result;
        }
    };

    /**
     * @brief Scripted movement system
     */
    class ScriptedMovementSystem {
    public:
        template<typename Registry>
        static void update(Registry& registry, float deltaTime) {
            registry.template view<Position, MovementScript>().each([deltaTime](
                auto entity, Position& pos, MovementScript& script
            ) {
                if (script.isComplete()) {
                    if (script.loop) {
                        script.reset();
                    } else {
                        return;
                    }
                }

                auto& currentCmd = script.commands[script.currentCommand];
                currentCmd->execute(pos, deltaTime);

                if (currentCmd->isComplete()) {
                    script.currentCommand++;
                }
            });
        }
    };

} // namespace Movement

#endif // SCRIPTED_MOVEMENT_HPP
