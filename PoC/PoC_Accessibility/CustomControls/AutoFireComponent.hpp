#pragma once
#include <chrono>
#include <functional>

/**
 * @brief Component implementing auto-fire functionality
 * 
 * This class implements both Hold and Toggle modes for firing,
 * as specified in the accessibility document.
 */
class AutoFireComponent {
public:
    /**
     * @brief Auto-fire modes
     */
    enum class Mode {
        Hold,   // Default: user must keep the key pressed
        Toggle  // Press once to enable, press again to disable
    };
    
    /**
     * @brief Constructor
     * @param fireInterval Time between shots in milliseconds
     */
    explicit AutoFireComponent(int fireIntervalMs = 200);
    
    /**
     * @brief Enable or disable auto-fire
     * @param enabled Whether auto-fire is enabled
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief Check if auto-fire is enabled
     * @return true if auto-fire is enabled
     */
    bool isEnabled() const;
    
    /**
     * @brief Set the auto-fire mode
     * @param mode The mode (Hold or Toggle)
     */
    void setMode(Mode mode);
    
    /**
     * @brief Get the current mode
     * @return The current mode
     */
    Mode getMode() const;
    
    /**
     * @brief Handle fire key press
     * @param fireCallback Function to call when firing
     */
    void handleFireKeyPress(std::function<void()> fireCallback);
    
    /**
     * @brief Update the auto-fire state (call every frame)
     * @param fireCallback Function to call when firing
     */
    void update(std::function<void()> fireCallback);
    
    /**
     * @brief Set the fire interval
     * @param intervalMs Time between shots in milliseconds
     */
    void setFireInterval(int intervalMs);
    
    /**
     * @brief Check if currently auto-firing
     * @return true if auto-fire is active
     */
    bool isAutoFiring() const;

private:
    bool enabled_;              // Whether auto-fire feature is enabled
    Mode mode_;                 // Current auto-fire mode
    bool autoFireActive_;       // Whether auto-fire is currently active (for toggle mode)
    int fireIntervalMs_;        // Time between shots in milliseconds
    std::chrono::steady_clock::time_point lastFireTime_;  // Time of last shot
    
    /**
     * @brief Check if enough time has passed to fire again
     * @return true if we can fire
     */
    bool canFire() const;
};
