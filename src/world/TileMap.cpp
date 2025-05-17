// ================================================================================================
// File: TileMap.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <iostream>
#include <fstream>
#include <filesystem>
#include "TileMap.hpp"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "../external/json.hpp"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

using json = nlohmann::json;

TileMap::TileMap(int width, int height)
{
	resize(width, height);
}

bool TileMap::saveToJson(const std::string& filename) const
{
	namespace fs = std::filesystem;

	fs::path filePath(filename);
	fs::path directory = filePath.parent_path();

	if (!fs::exists(directory)) {
		if (!fs::create_directories(directory)) {
			std::cerr << "Error: Failed to create directory: " << directory << '\n';
			return false;
		}
	}

    json j;

	j["width"] = tiles[0].size();
	j["height"] = tiles.size();
    j["tiles"] = json::array();

    for (int y = 0; y < tiles.size(); ++y)
    {
		for (int x = 0; x < tiles[y].size(); ++x)
		{
			const Tile& tile = tiles[y][x];
			if (tile.type != Tile::Type::EMPTY)
			{
				j["tiles"].push_back
				({
					{"x", x},
					{"y", y},
					{"type", static_cast<int>(tile.type)}
				});
			}
		}
    }

	std::ofstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
		return false;
	}
	file << j.dump(4);
	return true;
}

bool TileMap::loadFromJson(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
		return false;
	}

	json j;
	file >> j;

	if (j.contains("width") && j.contains("height"))
	{
		int width = j["width"];
		int height = j["height"];
		resize(width, height);
	}
	else
	{
		std::cerr << "Error: Invalid JSON format. Missing width or height." << std::endl;
		return false;
	}
	if (j.contains("tiles"))
	{
		for (const auto& tileData : j["tiles"])
		{
			int x = tileData["x"];
			int y = tileData["y"];
			Tile::Type type = static_cast<Tile::Type>(tileData["type"]);
			setTile(x, y, Tile{ type });
		}
	}
	else
	{
		std::cerr << "Error: Invalid JSON format. Missing tiles." << std::endl;
		return false;
	}
	return true;
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
			rect.setFillColor(getTileColor(tile.type));
            visuals.push_back(rect);
        }
    }
}

void TileMap::resize(int width, int height)
{
    tiles = std::vector<std::vector<Tile>>(height, std::vector<Tile>(width));
	rebuildVisuals();
}

void TileMap::setTile(int x, int y, Tile tile, bool shouldRebuildVisuals)
{
	if (x < 0 || x >= static_cast<int>(tiles[0].size()) || y < 0 || y >= static_cast<int>(tiles.size()))
	{
		std::cerr << "Error: Tile coordinates out of bounds!" << std::endl;
		return;
	}
    tiles[y][x] = tile;
	if (shouldRebuildVisuals)
		rebuildVisuals();
}

const sf::Color& TileMap::getTileColor(Tile::Type type) const
{
	switch (type)
	{
	case Tile::Type::Background:
		return sf::Color(50, 40, 75);
	case Tile::Type::Solid:
		return sf::Color(90, 75, 110);
	case Tile::Type::Water:
		return sf::Color(30, 100, 150, 180);
	case Tile::Type::Door:
		return sf::Color(200, 180, 255);
	default:
		return sf::Color::Transparent;
	}
}

bool TileMap::isWithinBounds(int x, int y) const
{
	return x >= 0 && x < static_cast<int>(tiles[0].size()) && y >= 0 && y < static_cast<int>(tiles.size());
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
	for (const auto& rect : visuals)
	{
		if (drawTransparentOnly && rect.getFillColor().a == 255)
			continue;
		if (!drawTransparentOnly && rect.getFillColor().a != 255)
			continue;

		target.draw(rect, states);
	}
}