// ================================================================================================
// File: Area.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 25, 2025
// Description: Defines the Area class, which represents a rectangular area in the game world.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <memory>
#include "TileMap.hpp"
#include "../state/game/Enemy.hpp"
#include "../state/game/Player.hpp"

class Area
{
public:
	Area(Player& player);

	bool load(const std::string& filename);
	bool save(const std::string& filename) const;

	TileMap map;
	Player& player;
	std::vector<std::unique_ptr<Enemy>> enemies;

private:
};