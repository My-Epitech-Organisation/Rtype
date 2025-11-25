#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

#include "ColorblindSimulator.hpp"
#include "VisualCueSystem.hpp"
#include "Projectile.hpp"

/**
 * @brief Colorblind Accessibility PoC
 * 
 * This PoC demonstrates the accessibility features described in the documentation:
 * 1. Colorblind-safe palette with high-contrast outlines
 * 2. Shape-based projectile differentiation
 * 3. Visual sound cues (hit indicator, missile warning, power-up spawn)
 * 4. Real-time colorblind vision simulation
 * 
 * Controls:
 * - Arrow Keys: Move player ship
 * - Space: Fire player bullet
 * - M: Spawn missile (with warning visual cue)
 * - H: Trigger hit indicator
 * - P: Trigger power-up spawn cue
 * - 1-4: Cycle through colorblind simulation modes
 * - E: Spawn enemy bullets
 * - ESC: Exit
 */

class ColorblindPoC {
public:
    ColorblindPoC() 
        : m_window(sf::VideoMode(1280, 720), "Colorblind Accessibility PoC - R-Type")
        , m_cvdType(CVDType::Normal)
        , m_playerPos(100.0f, 360.0f)
        , m_playerSpeed(300.0f)
        , m_spawnTimer(0.0f)
        , m_showHelp(true)
    {
        m_window.setFramerateLimit(60);

        // Create starfield background
        createStarfield();

        // Load font for UI (if available)
        // In production, ensure font is available
        if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
            std::cerr << "Warning: Could not load font. Using shapes for text.\n";
        }
    }

    void run() {
        sf::Clock clock;

        while (m_window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();

            handleEvents();
            update(deltaTime);
            render();
        }
    }

private:
    sf::RenderWindow m_window;
    sf::Font m_font;
    CVDType m_cvdType;
    
    // Player
    sf::Vector2f m_playerPos;
    float m_playerSpeed;

    // Game objects
    std::vector<Projectile> m_projectiles;
    std::vector<sf::CircleShape> m_starfield;
    
    // Systems
    VisualCueSystem m_visualCues;

    // Timing
    float m_spawnTimer;
    bool m_showHelp;

    void createStarfield() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distX(0.0f, 1280.0f);
        std::uniform_real_distribution<float> distY(0.0f, 720.0f);
        std::uniform_real_distribution<float> distSize(1.0f, 3.0f);

        for (int i = 0; i < 200; ++i) {
            sf::CircleShape star(distSize(gen));
            star.setPosition(distX(gen), distY(gen));
            star.setFillColor(sf::Color(200, 200, 200, 150));
            m_starfield.push_back(star);
        }
    }

    void handleEvents() {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        m_window.close();
                        break;

                    // Colorblind mode switching
                    case sf::Keyboard::Num1:
                        m_cvdType = CVDType::Normal;
                        std::cout << "Switched to: " << ColorblindSimulator::getCVDTypeName(m_cvdType) << "\n";
                        break;
                    case sf::Keyboard::Num2:
                        m_cvdType = CVDType::Protanopia;
                        std::cout << "Switched to: " << ColorblindSimulator::getCVDTypeName(m_cvdType) << "\n";
                        break;
                    case sf::Keyboard::Num3:
                        m_cvdType = CVDType::Deuteranopia;
                        std::cout << "Switched to: " << ColorblindSimulator::getCVDTypeName(m_cvdType) << "\n";
                        break;
                    case sf::Keyboard::Num4:
                        m_cvdType = CVDType::Tritanopia;
                        std::cout << "Switched to: " << ColorblindSimulator::getCVDTypeName(m_cvdType) << "\n";
                        break;

                    // Fire player bullet
                    case sf::Keyboard::Space:
                        m_projectiles.emplace_back(
                            Projectile::Type::PlayerBullet,
                            m_playerPos,
                            sf::Vector2f(500.0f, 0.0f)
                        );
                        break;

                    // Spawn enemy bullets
                    case sf::Keyboard::E:
                        spawnEnemyBullets();
                        break;

                    // Trigger visual cues (simulating audio events)
                    case sf::Keyboard::H:
                        m_visualCues.triggerCue(VisualCueType::HitIndicator, m_playerPos);
                        std::cout << "Triggered: Hit Indicator (simulates taking damage sound)\n";
                        break;

                    case sf::Keyboard::M:
                        {
                            // Spawn missile with warning
                            sf::Vector2f missilePos(1200.0f, 360.0f);
                            sf::Vector2f direction = m_playerPos - missilePos;
                            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                            direction /= length;

                            m_projectiles.emplace_back(
                                Projectile::Type::Missile,
                                missilePos,
                                direction * 200.0f
                            );

                            m_visualCues.triggerCue(
                                VisualCueType::MissileWarning,
                                m_playerPos + direction * 100.0f,
                                direction
                            );
                            std::cout << "Triggered: Missile Warning (simulates missile lock sound)\n";
                        }
                        break;

                    case sf::Keyboard::P:
                        {
                            sf::Vector2f powerUpPos(640.0f, 200.0f);
                            m_visualCues.triggerCue(VisualCueType::PowerUpSpawn, powerUpPos);
                            std::cout << "Triggered: Power-Up Spawn (simulates power-up spawn sound)\n";
                        }
                        break;

                    case sf::Keyboard::F1:
                        m_showHelp = !m_showHelp;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    void spawnEnemyBullets() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distY(50.0f, 670.0f);

        for (int i = 0; i < 5; ++i) {
            m_projectiles.emplace_back(
                Projectile::Type::EnemyBullet,
                sf::Vector2f(1200.0f, distY(gen)),
                sf::Vector2f(-300.0f, 0.0f)
            );
        }
    }

    void update(float deltaTime) {
        // Update player movement
        sf::Vector2f movement(0.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            movement.x -= 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            movement.x += 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            movement.y -= 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            movement.y += 1.0f;
        }

        // Normalize diagonal movement
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        if (length > 0.0f) {
            movement /= length;
        }

        m_playerPos += movement * m_playerSpeed * deltaTime;

        // Clamp player position
        m_playerPos.x = std::max(20.0f, std::min(m_playerPos.x, 1260.0f));
        m_playerPos.y = std::max(20.0f, std::min(m_playerPos.y, 700.0f));

        // Update projectiles
        for (auto& projectile : m_projectiles) {
            projectile.update(deltaTime);
        }

        // Remove off-screen projectiles
        m_projectiles.erase(
            std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                [this](const Projectile& p) {
                    return p.isOffScreen(m_window.getSize());
                }),
            m_projectiles.end()
        );

        // Update visual cue system
        m_visualCues.update(deltaTime);

        // Auto-spawn enemy bullets for demo
        m_spawnTimer += deltaTime;
        if (m_spawnTimer > 3.0f) {
            spawnEnemyBullets();
            m_spawnTimer = 0.0f;
        }
    }

    void render() {
        m_window.clear(sf::Color(10, 10, 30)); // Dark space background

        // Draw starfield
        for (const auto& star : m_starfield) {
            m_window.draw(star);
        }

        // Draw projectiles
        for (auto& projectile : m_projectiles) {
            projectile.draw(m_window, m_cvdType);
        }

        // Draw player ship (simple triangle)
        drawPlayerShip();

        // Draw visual cues (always on top)
        m_visualCues.draw(m_window);

        // Draw UI
        drawUI();

        m_window.display();
    }

    void drawPlayerShip() {
        sf::ConvexShape ship(3);
        ship.setPoint(0, sf::Vector2f(20, 0));
        ship.setPoint(1, sf::Vector2f(-15, -12));
        ship.setPoint(2, sf::Vector2f(-15, 12));
        
        sf::Color shipColor = ColorblindSimulator::transformColor(
            sf::Color(0, 255, 100), 
            m_cvdType
        );
        
        ship.setFillColor(shipColor);
        ship.setOutlineColor(sf::Color::White);
        ship.setOutlineThickness(2.0f);
        ship.setPosition(m_playerPos);

        m_window.draw(ship);
    }

    void drawUI() {
        // Current CVD mode indicator
        sf::RectangleShape modeBox(sf::Vector2f(300.0f, 40.0f));
        modeBox.setPosition(10.0f, 10.0f);
        modeBox.setFillColor(sf::Color(0, 0, 0, 180));
        modeBox.setOutlineColor(sf::Color::White);
        modeBox.setOutlineThickness(2.0f);
        m_window.draw(modeBox);

        if (m_font.getInfo().family != "") {
            sf::Text modeText;
            modeText.setFont(m_font);
            modeText.setString(std::string("Mode: ") + ColorblindSimulator::getCVDTypeName(m_cvdType));
            modeText.setCharacterSize(20);
            modeText.setFillColor(sf::Color::White);
            modeText.setPosition(20.0f, 17.0f);
            m_window.draw(modeText);
        }

        // Controls help
        if (m_showHelp) {
            sf::RectangleShape helpBox(sf::Vector2f(400.0f, 280.0f));
            helpBox.setPosition(870.0f, 10.0f);
            helpBox.setFillColor(sf::Color(0, 0, 0, 200));
            helpBox.setOutlineColor(sf::Color::White);
            helpBox.setOutlineThickness(2.0f);
            m_window.draw(helpBox);

            if (m_font.getInfo().family != "") {
                std::vector<std::string> helpLines = {
                    "CONTROLS:",
                    "Arrow Keys - Move",
                    "Space - Fire",
                    "E - Spawn Enemy Bullets",
                    "M - Spawn Missile + Warning",
                    "H - Hit Indicator",
                    "P - Power-Up Spawn Cue",
                    "",
                    "COLORBLIND MODES:",
                    "1 - Normal Vision",
                    "2 - Protanopia (Red-blind)",
                    "3 - Deuteranopia (Green-blind)",
                    "4 - Tritanopia (Blue-blind)",
                    "",
                    "F1 - Toggle Help",
                    "ESC - Exit"
                };

                float yOffset = 20.0f;
                for (const auto& line : helpLines) {
                    sf::Text text;
                    text.setFont(m_font);
                    text.setString(line);
                    text.setCharacterSize(14);
                    text.setFillColor(sf::Color::White);
                    text.setPosition(880.0f, yOffset);
                    m_window.draw(text);
                    yOffset += 18.0f;
                }
            }
        }

        // Projectile count (for demo purposes)
        if (m_font.getInfo().family != "") {
            sf::Text countText;
            countText.setFont(m_font);
            std::ostringstream oss;
            oss << "Projectiles: " << m_projectiles.size();
            countText.setString(oss.str());
            countText.setCharacterSize(16);
            countText.setFillColor(sf::Color::White);
            countText.setPosition(10.0f, 670.0f);
            m_window.draw(countText);
        }
    }
};

int main() {
    std::cout << "=== Colorblind Accessibility PoC ===" << std::endl;
    std::cout << "This PoC demonstrates:" << std::endl;
    std::cout << "1. Colorblind-safe palette with high-contrast outlines" << std::endl;
    std::cout << "2. Shape-based projectile differentiation" << std::endl;
    std::cout << "3. Visual sound cues for important audio events" << std::endl;
    std::cout << "4. Real-time colorblind vision simulation" << std::endl;
    std::cout << std::endl;
    std::cout << "Use keys 1-4 to cycle through colorblind modes." << std::endl;
    std::cout << "Press H, M, P to trigger visual cues." << std::endl;
    std::cout << std::endl;

    try {
        ColorblindPoC poc;
        poc.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
