// ================================================================================================
// File: Area.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 25, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <iostream>
#include <fstream>
#include <filesystem>
#include "Area.hpp"
#include "../state/game//enemies/EnemyFactory.hpp"

Area::Area(Player& player) :
	player(player),
	map(16, 16)
{
}

bool Area::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file for reading: " << filename << '\n';
        return false;
    }

    json j;
    file >> j;

    // Load tilemap
    if (j.contains("tilemap"))
    {
        map.deserialize(j["tilemap"]);
    }

    // Load player spawn
    if (j.contains("player_spawn"))
    {
        player.spawnPosition.x = j["player_spawn"]["x"];
        player.spawnPosition.y = j["player_spawn"]["y"];
    }
    player.setPosition(player.spawnPosition);

    // Load enemies
    enemies.clear();
    if (j.contains("enemies"))
    {
        for (const auto& enemyJson : j["enemies"])
        {
            /*Enemy enemy;
            enemy.deserialize(enemyJson);*/
            enemies.emplace_back(lv::createEnemyFromJson(enemyJson));
        }
    }

    return true;
}

bool Area::save(const std::string& filename) const
{
    namespace fs = std::filesystem;
    fs::path filePath(filename);
    fs::path directory = filePath.parent_path();
    if (!fs::exists(directory) && !fs::create_directories(directory))
    {
        std::cerr << "Error: Failed to create directory: " << directory << '\n';
        return false;
    }

    json j;

    // Save tiles
    json tileData;
    map.serialize(tileData);
    j["tilemap"] = tileData;

    // Save player spawn
    j["player_spawn"] = { {"x", player.spawnPosition.x}, {"y", player.spawnPosition.y} };

    // Save enemies
    j["enemies"] = json::array();
    for (const auto& enemy : enemies)
    {
        json enemyData;
        enemy->serialize(enemyData);
		j["enemies"].push_back(enemyData);
        /* enemyJson["x"] = enemy.position.x;
        enemyJson["y"] = enemy.position.y;
        enemyJson["type"] = enemy.type;

        json patrolJson = json::array();
        for (const auto& point : enemy.getPatrolPositions())
        {
            patrolJson.push_back({ {"x", point.x}, {"y", point.y} });
        }
        enemyJson["patrol_path"] = patrolJson;

        j["enemies"].push_back(enemyJson);*/
    }

    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file for writing: " << filename << '\n';
        return false;
    }

    file << j.dump(4);
    return true;
}
