/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Usage examples for EntityConfig + PrefabManager integration
*/

// ============================================================================
// USAGE - EntityConfig avec PrefabManager (ECS)
// ============================================================================

/*
// 1. INITIALISATION (au démarrage du jeu)
// ========================================

#include "games/rtype/shared/EntityConfig.hpp"
#include "games/rtype/shared/PrefabLoader.hpp"
#include "engine/ecs/ECS.hpp"

void GameServer::init() {
    // Charger les configs TOML
    auto& configRegistry = EntityConfigRegistry::getInstance();
    configRegistry.loadFromDirectory("config/game");

    // Créer le PrefabManager et enregistrer les prefabs depuis les configs
    ECS::PrefabManager prefabs(registry);
    PrefabLoader::registerAllPrefabs(prefabs);

    // Maintenant les prefabs sont disponibles:
    // - "enemy_basic", "enemy_wave", "enemy_boss_1", etc.
    // - "projectile_basic_bullet", "projectile_missile", etc.
    // - "player_default_ship", "player_speed_ship", etc.
    // - "powerup_health_small", "powerup_shield", etc.
}


// 2. SPAWN D'ENTITÉS AVEC PREFABS
// ================================

void SpawnSystem::spawnEnemy(float x, float y) {
    // Spawn avec position personnalisée
    auto enemy = prefabs.instantiate("enemy_basic", [x, y](auto& r, auto e) {
        r.getComponent<TransformComponent>(e) = {x, y, 0.0f};
    });
}

void SpawnSystem::spawnPlayer(uint32_t playerId, float x, float y) {
    auto player = prefabs.instantiate("player_default_ship", [x, y](auto& r, auto e) {
        r.getComponent<TransformComponent>(e) = {x, y, 0.0f};
    });
    
    // Récupérer les stats depuis le config
    float speed = PrefabLoader::getPlayerSpeed("default_ship");
    float fireRate = PrefabLoader::getPlayerFireRate("default_ship");
}

void ShootSystem::playerShoot(Entity player) {
    auto& transform = registry.getComponent<TransformComponent>(player);
    
    auto bullet = prefabs.instantiate("projectile_basic_bullet", [&](auto& r, auto e) {
        r.getComponent<TransformComponent>(e) = {
            transform.x + 32.0f, transform.y, 0.0f
        };
        // Velocity déjà configurée par le prefab (vers la droite)
    });
}


// 3. CHARGEMENT DE NIVEAU AVEC WAVES
// ===================================

void LevelManager::loadLevel(const std::string& levelId) {
    const auto* level = EntityConfigRegistry::getInstance().getLevel(levelId);
    if (!level) return;

    for (const auto& wave : level->waves) {
        for (const auto& spawn : wave.spawns) {
            // Spawn avec délai (utiliser un système de timer)
            scheduleSpawn(spawn.delay, [&, spawn]() {
                for (int i = 0; i < spawn.count; ++i) {
                    prefabs.instantiate("enemy_" + spawn.enemyId, [&](auto& r, auto e) {
                        r.getComponent<TransformComponent>(e) = {spawn.x, spawn.y, 0.0f};
                    });
                }
            });
        }
    }
}


// 4. SPAWN MULTIPLE
// ==================

void SpawnSystem::spawnWave() {
    // Spawn 5 ennemis basic d'un coup
    auto enemies = prefabs.instantiateMultiple("enemy_basic", 5);

    float y = 100.0f;
    for (auto enemy : enemies) {
        registry.getComponent<TransformComponent>(enemy) = {800.0f, y, 0.0f};
        y += 100.0f;
    }
}


// 5. ACCÈS AUX CONFIGS POUR LES SYSTEMS
// ======================================

void ScoreSystem::onEnemyKilled(const std::string& enemyType) {
    int32_t score = PrefabLoader::getEnemyScore(enemyType);
    m_playerScore += score;
}

void DamageSystem::applyProjectileDamage(Entity projectile, Entity target) {
    // Le damage est stocké dans HealthComponent du projectile
    auto& projHealth = registry.getComponent<HealthComponent>(projectile);
    auto& targetHealth = registry.getComponent<HealthComponent>(target);
    
    targetHealth.takeDamage(projHealth.current);
}


// 6. STRUCTURE DES PREFABS ENREGISTRÉS
// =====================================
//
// Nommage: "{type}_{id}" où id vient du TOML
//
// Enemies:      "enemy_basic", "enemy_wave", "enemy_shooter", "enemy_boss_1"
// Projectiles:  "projectile_basic_bullet", "projectile_missile"
// Players:      "player_default_ship", "player_speed_ship"
// PowerUps:     "powerup_health_small", "powerup_shield"

*/
