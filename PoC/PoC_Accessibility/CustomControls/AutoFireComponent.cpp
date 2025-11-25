#include "AutoFireComponent.hpp"
#include <iostream>

AutoFireComponent::AutoFireComponent(int fireIntervalMs)
    : enabled_(false)
    , mode_(Mode::Hold)
    , autoFireActive_(false)
    , fireIntervalMs_(fireIntervalMs)
    , lastFireTime_(std::chrono::steady_clock::now()) {
}

void AutoFireComponent::setEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled_) {
        autoFireActive_ = false;
    }
    std::cout << "Auto-fire " << (enabled_ ? "enabled" : "disabled") << std::endl;
}

bool AutoFireComponent::isEnabled() const {
    return enabled_;
}

void AutoFireComponent::setMode(Mode mode) {
    mode_ = mode;
    autoFireActive_ = false; // Reset active state when changing modes
    std::cout << "Auto-fire mode set to: " 
              << (mode_ == Mode::Hold ? "Hold" : "Toggle") << std::endl;
}

AutoFireComponent::Mode AutoFireComponent::getMode() const {
    return mode_;
}

void AutoFireComponent::handleFireKeyPress(std::function<void()> fireCallback) {
    if (!enabled_) {
        // If auto-fire is disabled, just fire once
        if (canFire()) {
            fireCallback();
            lastFireTime_ = std::chrono::steady_clock::now();
        }
        return;
    }
    
    // Auto-fire logic based on mode
    if (mode_ == Mode::Toggle) {
        // Toggle mode: press once to activate, press again to deactivate
        autoFireActive_ = !autoFireActive_;
        std::cout << "Auto-fire " << (autoFireActive_ ? "activated" : "deactivated") 
                  << " (Toggle mode)" << std::endl;
        
        // Fire immediately when activated
        if (autoFireActive_ && canFire()) {
            fireCallback();
            lastFireTime_ = std::chrono::steady_clock::now();
        }
    } else {
        // Hold mode: fire immediately, then continue firing in update()
        if (canFire()) {
            fireCallback();
            lastFireTime_ = std::chrono::steady_clock::now();
            autoFireActive_ = true;
        }
    }
}

void AutoFireComponent::update(std::function<void()> fireCallback) {
    if (!enabled_ || !autoFireActive_) {
        return;
    }
    
    // Fire weapon at intervals
    if (canFire()) {
        fireCallback();
        lastFireTime_ = std::chrono::steady_clock::now();
    }
}

void AutoFireComponent::setFireInterval(int intervalMs) {
    fireIntervalMs_ = intervalMs;
}

bool AutoFireComponent::isAutoFiring() const {
    return enabled_ && autoFireActive_;
}

bool AutoFireComponent::canFire() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastFireTime_).count();
    return elapsed >= fireIntervalMs_;
}
