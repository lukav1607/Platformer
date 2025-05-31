// ================================================================================================
// File: EnemyFactory.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 18, 2025
// Description: Defines the EnemyFactory function, which is responsible for creating instances of
//              Enemy objects from serialized JSON data. This factory function allows for
//              dynamic creation of different enemy types based on their serialized data, enabling
//              easy addition of new enemy types without modifying the core game logic.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <memory>
#include "FlyingEnemy.hpp"

namespace lv
{
	// Factory function to create an Enemy instance from serialized JSON data
	std::unique_ptr<Enemy> createEnemyFromJson(const json& j)
	{
		// Dummy instances to check type
		FlyingEnemy flyingEnemy;

        std::string type = j.at("type").get<std::string>();

        if (type == flyingEnemy.getType())
        {
            auto enemy = std::make_unique<FlyingEnemy>();
            enemy->deserialize(j);
            return enemy;
        }
        /*else if (type == "WalkingEnemy")
        {
            auto enemy = std::make_unique<WalkingEnemy>();
            enemy->deserialize(j);
            return enemy;
        }*/
		return nullptr;
	}

	// Factory function to create a default Enemy instance (for testing or fallback)
	///std::unique_ptr<Enemy> createDefaultEnemy();
}