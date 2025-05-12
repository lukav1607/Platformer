// ================================================================================================
// File: TileMap.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <iostream>
#include "TileMap.hpp"

TileMap::TileMap(int width, int height)
{
	resize(width, height);
}

void TileMap::rebuildVisuals()
{
    visuals.clear();

    for (size_t y = 0; y < tiles.size(); ++y) {
        for (size_t x = 0; x < tiles[y].size(); ++x) {
            const Tile& tile = tiles[y][x];
            if (tile.type == Tile::Type::EMPTY) continue;

            sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            rect.setPosition({ static_cast<float>(x * TILE_SIZE), static_cast<float>(y * TILE_SIZE) });

            // Color by tile type
            switch (tile.type) {
            case Tile::Type::SOLID:
                rect.setFillColor(sf::Color::Blue);
                break;
            default:
                rect.setFillColor(sf::Color::Transparent);
                break;
            }

            visuals.push_back(rect);
        }
    }
}

void TileMap::resize(int width, int height)
{
    tiles = std::vector<std::vector<Tile>>(height, std::vector<Tile>(width));
	rebuildVisuals();
}

void TileMap::setTile(int x, int y, Tile tile)
{
	if (x < 0 || x >= static_cast<int>(tiles[0].size()) || y < 0 || y >= static_cast<int>(tiles.size()))
	{
		std::cerr << "Error: Tile coordinates out of bounds!" << std::endl;
		return;
	}
    tiles[y][x] = tile;
	rebuildVisuals();
}

bool TileMap::isWithinBounds(int x, int y) const
{
	return x >= 0 && x < static_cast<int>(tiles[0].size()) && y >= 0 && y < static_cast<int>(tiles.size());
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    for (const auto& rect : visuals)
        target.draw(rect, states);
}