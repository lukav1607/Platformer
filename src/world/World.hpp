// ================================================================================================
// File: World.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 25, 2025
// Description: Defines the World class, which represents the game world and manages areas.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <vector>
#include "Area.hpp"

class World
{
public:
	World(Player& player);

	Area& getCurrentArea() { return areas.at(currentAreaIndex); }
	
private:
	std::vector<Area> areas; // List of areas in the world
	size_t currentAreaIndex = 0; // Index of the currently active area
};