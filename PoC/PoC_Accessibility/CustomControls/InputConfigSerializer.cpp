#include "InputConfigSerializer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

bool InputConfigSerializer::loadFromFile(const std::string& filename,
                                        InputManager& inputManager,
                                        bool& autoFireEnabled) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    std::unordered_map<Action, KeyCode> bindings;
    if (!parseJsonConfig(content, bindings, autoFireEnabled)) {
        std::cerr << "Failed to parse config file" << std::endl;
        return false;
    }
    
    inputManager.loadBindings(bindings);
    std::cout << "Successfully loaded config from: " << filename << std::endl;
    return true;
}

bool InputConfigSerializer::saveToFile(const std::string& filename,
                                      const InputManager& inputManager,
                                      bool autoFireEnabled) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << filename << std::endl;
        return false;
    }
    
    std::string jsonContent = generateJsonConfig(inputManager.getBindings(), autoFireEnabled);
    file << jsonContent;
    
    std::cout << "Successfully saved config to: " << filename << std::endl;
    return true;
}

// Simple manual JSON parsing for PoC purposes
bool InputConfigSerializer::parseJsonConfig(const std::string& content,
                                           std::unordered_map<Action, KeyCode>& bindings,
                                           bool& autoFireEnabled) {
    // Simple parser - in production, use a proper JSON library
    auto extractValue = [](const std::string& content, const std::string& key) -> std::string {
        size_t pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        
        size_t colonPos = content.find(":", pos);
        if (colonPos == std::string::npos) return "";
        
        size_t startQuote = content.find("\"", colonPos);
        if (startQuote == std::string::npos) {
            // Maybe it's a boolean
            size_t truePos = content.find("true", colonPos);
            size_t falsePos = content.find("false", colonPos);
            if (truePos != std::string::npos && truePos < colonPos + 20) return "true";
            if (falsePos != std::string::npos && falsePos < colonPos + 20) return "false";
            return "";
        }
        
        size_t endQuote = content.find("\"", startQuote + 1);
        if (endQuote == std::string::npos) return "";
        
        return content.substr(startQuote + 1, endQuote - startQuote - 1);
    };
    
    // Parse each binding
    std::string moveUp = extractValue(content, "move_up");
    std::string moveLeft = extractValue(content, "move_left");
    std::string moveDown = extractValue(content, "move_down");
    std::string moveRight = extractValue(content, "move_right");
    std::string fire = extractValue(content, "fire");
    std::string autoFire = extractValue(content, "auto_fire");
    
    if (moveUp.empty() || moveLeft.empty() || moveDown.empty() || 
        moveRight.empty() || fire.empty()) {
        return false;
    }
    
    bindings[Action::MoveUp] = InputManager::stringToKeyCode(moveUp);
    bindings[Action::MoveLeft] = InputManager::stringToKeyCode(moveLeft);
    bindings[Action::MoveDown] = InputManager::stringToKeyCode(moveDown);
    bindings[Action::MoveRight] = InputManager::stringToKeyCode(moveRight);
    bindings[Action::Fire] = InputManager::stringToKeyCode(fire);
    
    autoFireEnabled = (autoFire == "true");
    
    return true;
}

std::string InputConfigSerializer::generateJsonConfig(
    const std::unordered_map<Action, KeyCode>& bindings,
    bool autoFireEnabled) {
    
    std::ostringstream json;
    json << "{\n";
    json << "  \"controls\": {\n";
    
    // Get bindings for each action
    auto getBinding = [&bindings](Action action) -> std::string {
        auto it = bindings.find(action);
        if (it != bindings.end()) {
            return InputManager::keyCodeToString(it->second);
        }
        return "Unknown";
    };
    
    json << "    \"move_up\": \"" << getBinding(Action::MoveUp) << "\",\n";
    json << "    \"move_left\": \"" << getBinding(Action::MoveLeft) << "\",\n";
    json << "    \"move_down\": \"" << getBinding(Action::MoveDown) << "\",\n";
    json << "    \"move_right\": \"" << getBinding(Action::MoveRight) << "\",\n";
    json << "    \"fire\": \"" << getBinding(Action::Fire) << "\",\n";
    json << "    \"auto_fire\": " << (autoFireEnabled ? "true" : "false") << "\n";
    json << "  }\n";
    json << "}\n";
    
    return json.str();
}
