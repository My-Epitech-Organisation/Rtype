/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossComponent - Implementation of boss component utility functions
*/

#include "BossComponent.hpp"

namespace rtype::games::rtype::shared {

const char* bossAttackPatternToString(BossAttackPattern pattern) noexcept {
    switch (pattern) {
        case BossAttackPattern::None:
            return "None";
        case BossAttackPattern::CircularShot:
            return "CircularShot";
        case BossAttackPattern::SpreadFan:
            return "SpreadFan";
        case BossAttackPattern::LaserSweep:
            return "LaserSweep";
        case BossAttackPattern::MinionSpawn:
            return "MinionSpawn";
        case BossAttackPattern::TailSweep:
            return "TailSweep";
        case BossAttackPattern::ChargeAttack:
            return "ChargeAttack";
        case BossAttackPattern::HomingMissile:
            return "HomingMissile";
        case BossAttackPattern::GroundPound:
            return "GroundPound";
        default:
            return "Unknown";
    }
}

const char* bossTypeToString(BossType type) noexcept {
    switch (type) {
        case BossType::Generic:
            return "Generic";
        case BossType::Serpent:
            return "Serpent";
        case BossType::Scorpion:
            return "Scorpion";
        case BossType::Battleship:
            return "Battleship";
        case BossType::Hive:
            return "Hive";
        default:
            return "Unknown";
    }
}

BossAttackPattern stringToBossAttackPattern(const std::string& str) noexcept {
    if (str == "circular_shot") return BossAttackPattern::CircularShot;
    if (str == "spread_fan") return BossAttackPattern::SpreadFan;
    if (str == "laser_sweep") return BossAttackPattern::LaserSweep;
    if (str == "minion_spawn") return BossAttackPattern::MinionSpawn;
    if (str == "tail_sweep") return BossAttackPattern::TailSweep;
    if (str == "charge_attack") return BossAttackPattern::ChargeAttack;
    if (str == "homing_missile") return BossAttackPattern::HomingMissile;
    if (str == "ground_pound") return BossAttackPattern::GroundPound;
    return BossAttackPattern::None;
}

BossType stringToBossType(const std::string& str) noexcept {
    if (str == "serpent") return BossType::Serpent;
    if (str == "scorpion") return BossType::Scorpion;
    if (str == "battleship") return BossType::Battleship;
    if (str == "hive") return BossType::Hive;
    return BossType::Generic;
}

}  // namespace rtype::games::rtype::shared
