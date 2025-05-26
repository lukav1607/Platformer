// ================================================================================================
// File: World.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 25, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include "World.hpp"

World::World(Player& player)
{
	// Initialize the world with a default area
	areas.emplace_back(Area(player));
	areas.back().map.resize(48, 32); // Default tile map size
	areas.back().player.spawnPosition = sf::Vector2i(0, 0); // Default player spawn position
	currentAreaIndex = 0; // Start with the first area
}
