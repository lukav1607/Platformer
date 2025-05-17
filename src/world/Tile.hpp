// ================================================================================================
// File: Tile.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the Tile struct, which represents a tile in the tile map.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

struct Tile
{
	enum class Type
	{
		EMPTY,
		Background,
		Solid,
		Water,
		Door
	};
	Type type;
};