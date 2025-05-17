// ================================================================================================
// File: TileMap.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the TileMap class, which is responsible for managing the tile-based map.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "Tile.hpp"

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
	TileMap(int width, int height);

	bool saveToJson(const std::string& filename) const;
	bool loadFromJson(const std::string& filename);

	void resize(int width, int height);
	inline sf::Vector2i getSize() const { return sf::Vector2i(tiles[0].size(), tiles.size()); }

	void rebuildVisuals();
	void setTile(int x, int y, Tile tile, bool shouldRebuildVisuals = true);
	inline void setTile(sf::Vector2i coords, Tile tile, bool shouldRebuildVisuals = true) { setTile(coords.x, coords.y, tile, shouldRebuildVisuals); }
	inline const Tile& getTile(int x, int y) const { return tiles[y][x]; }
	inline const Tile& getTile(sf::Vector2i coords) const { return getTile(coords.x, coords.y); }
	const sf::Color& getTileColor(Tile::Type type) const;

	bool isWithinBounds(int x, int y) const;
	inline bool isWithinBounds(sf::Vector2i coords) const { return isWithinBounds(coords.x, coords.y); }

	static constexpr float TILE_SIZE = 64.f;
	bool drawTransparentOnly = false;

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	std::vector<std::vector<Tile>> tiles;
	std::vector<sf::RectangleShape> visuals;
};