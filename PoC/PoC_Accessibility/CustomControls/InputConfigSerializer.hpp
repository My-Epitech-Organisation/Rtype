#pragma once
#include "InputManager.hpp"
#include <string>
#include <optional>

/**
 * @brief Loads and saves keybindings from/to a config file
 *
 * This class handles serialization of input bindings to JSON format
 * as specified in the accessibility document.
 */
class InputConfigSerializer {
public:
    /**
     * @brief Load keybindings from a JSON config file
     * @param filename Path to the config file
     * @param inputManager The InputManager to load bindings into
     * @param autoFireEnabled Output parameter for auto-fire setting
     * @return true if loading was successful
     */
    static bool loadFromFile(const std::string& filename,
                           InputManager& inputManager,
                           bool& autoFireEnabled);

    /**
     * @brief Save keybindings to a JSON config file
     * @param filename Path to the config file
     * @param inputManager The InputManager to save bindings from
     * @param autoFireEnabled The auto-fire setting to save
     * @return true if saving was successful
     */
    static bool saveToFile(const std::string& filename,
                          const InputManager& inputManager,
                          bool autoFireEnabled);

private:
    /**
     * @brief Parse a simple JSON config (manual parsing for PoC)
     * @param content The JSON content as string
     * @param bindings Output map of parsed bindings
     * @param autoFireEnabled Output for auto-fire setting
     * @return true if parsing was successful
     */
    static bool parseJsonConfig(const std::string& content,
                               std::unordered_map<Action, KeyCode>& bindings,
                               bool& autoFireEnabled);

    /**
     * @brief Generate JSON string from bindings
     * @param bindings The bindings to serialize
     * @param autoFireEnabled The auto-fire setting
     * @return JSON string
     */
    static std::string generateJsonConfig(const std::unordered_map<Action, KeyCode>& bindings,
                                         bool autoFireEnabled);
};
