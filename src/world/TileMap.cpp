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

TileMap::TileMap(int width, int height) :
	gridLines(sf::PrimitiveType::Lines),
	gridColor(sf::Color(255, 255, 255, 50)),
	isGridShown(false)
{
	resize(width, height);
	rebuildGridLines();
}

//bool TileMap::saveToJson(const std::string& filename) const
//{
//	namespace fs = std::filesystem;
//
//	fs::path filePath(filename);
//	fs::path directory = filePath.parent_path();
//
//	if (!fs::exists(directory)) {
//		if (!fs::create_directories(directory)) {
//			std::cerr << "Error: Failed to create directory: " << directory << '\n';
//			return false;
//		}
//	}
//
//    json j;
//
//	j["width"] = tiles[0].size();
//	j["height"] = tiles.size();
//    j["tiles"] = json::array();
//
//    for (int y = 0; y < tiles.size(); ++y)
//    {
//		for (int x = 0; x < tiles[y].size(); ++x)
//		{
//			const Tile& tile = tiles[y][x];
//			if (tile.type != Tile::Type::EMPTY)
//			{
//				j["tiles"].push_back
//				({
//					{"x", x},
//					{"y", y},
//					{"type", static_cast<int>(tile.type)}
//				});
//			}
//		}
//    }
//
//	std::ofstream file(filename);
//	if (!file.is_open())
//	{
//		std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
//		return false;
//	}
//	file << j.dump(4);
//	return true;
//}
//
//bool TileMap::loadFromJson(const std::string& filename)
//{
//	std::ifstream file(filename);
//	if (!file.is_open())
//	{
//		std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
//		return false;
//	}
//
//	json j;
//	file >> j;
//
//	if (j.contains("width") && j.contains("height"))
//	{
//		int width = j["width"];
//		int height = j["height"];
//		resize(width, height);
//	}
//	else
//	{
//		std::cerr << "Error: Invalid JSON format. Missing width or height." << std::endl;
//		return false;
//	}
//	if (j.contains("tiles"))
//	{
//		for (const auto& tileData : j["tiles"])
//		{
//			int x = tileData["x"];
//			int y = tileData["y"];
//			Tile::Type type = static_cast<Tile::Type>(tileData["type"]);
//			setTile(x, y, Tile{ type });
//		}
//	}
//	else
//	{
//		std::cerr << "Error: Invalid JSON format. Missing tiles." << std::endl;
//		return false;
//	}
//	return true;
//}

void TileMap::rebuildGridLines()
{
	gridLines.clear();

	//	Vertical lines:
	for (int x = 0; x <= getSize().x; ++x)
	{
		float xpos = static_cast<float>(x * TileMap::TILE_SIZE);
		gridLines.append(sf::Vertex{ { sf::Vector2f(xpos, 0.f) }, gridColor });
		gridLines.append(sf::Vertex{ { sf::Vector2f(xpos, getSize().y * TileMap::TILE_SIZE) }, gridColor });
	}
	//	Horizontal lines:
	for (int y = 0; y <= getSize().y; ++y)
	{
		float ypos = static_cast<float>(y * TileMap::TILE_SIZE);
		gridLines.append(sf::Vertex{ { sf::Vector2f(0.f, ypos) }, gridColor });
		gridLines.append(sf::Vertex{ { sf::Vector2f(getSize().x * TileMap::TILE_SIZE, ypos)}, gridColor });
	}
}

void TileMap::renderGrid(sf::RenderWindow& window)
{
	if (!isGridShown)
		return;

	sf::RectangleShape border(sf::Vector2f(getSize().x * TileMap::TILE_SIZE, getSize().y * TileMap::TILE_SIZE));
	border.setFillColor(sf::Color::Transparent);
	border.setOutlineThickness(2.f);
	border.setOutlineColor(sf::Color(255, 255, 255, 128));
	window.draw(border);
	window.draw(gridLines);
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

void TileMap::serialize(json& j) const
{
	j["width"] = static_cast<int>(tiles[0].size());
	j["height"] = static_cast<int>(tiles.size());
	j["tiles"] = json::array();

	for (int y = 0; y < tiles.size(); ++y)
	{
		for (int x = 0; x < tiles[y].size(); ++x)
		{
			const Tile& tile = tiles[y][x];
			if (tile.type != Tile::Type::EMPTY)
			{
				j["tiles"].push_back({
					{"x", x},
					{"y", y},
					{"type", static_cast<int>(tile.type)}
					});
			}
		}
	}
}

void TileMap::deserialize(const json& j)
{
	int width = j["width"];
	int height = j["height"];
	resize(width, height);

	for (const auto& tileData : j["tiles"])
	{
		int x = tileData["x"];
		int y = tileData["y"];
		Tile::Type type = static_cast<Tile::Type>(tileData["type"]);
		setTile(x, y, Tile{ type });
	}
}

void TileMap::resize(int width, int height)
{
    tiles = std::vector<std::vector<Tile>>(height, std::vector<Tile>(width));
	rebuildVisuals();
	rebuildGridLines();
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

sf::Color TileMap::getTileColor(Tile::Type type) const
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

bool TileMap::collidesWith(const sf::FloatRect& rect) const
{
	int left = static_cast<int>(std::floor(rect.position.x / TILE_SIZE));
	int top = static_cast<int>(std::floor(rect.position.y / TILE_SIZE));
	int right = static_cast<int>(std::floor((rect.position.x + rect.size.x) / TILE_SIZE));
	int bottom = static_cast<int>(std::floor((rect.position.y + rect.size.y) / TILE_SIZE));

	// Quick bounds check
	if (!isWithinBounds(sf::Vector2i(left, top)) || !isWithinBounds(sf::Vector2i(right, bottom)))
		return true; // Treat out-of-bounds as solid (blocking)

	for (int y = top; y <= bottom; ++y)
	{
		for (int x = left; x <= right; ++x)
		{
			sf::Vector2i tilePos(x, y);
			if (!isWithinBounds(tilePos))
				return true; // Treat outside as blocking

			if (getTile(tilePos).type == Tile::Type::Solid)
				return true;
		}
	}

	// Diagonal corner check
	// Check if rect overlaps a corner between two solid tiles (top-left corner of rect for example)
	// We do this by checking for diagonal adjacency of solid tiles overlapping the rect corners.

	// Corners of the rect in world coordinates
	sf::Vector2f corners[4] = {
		{rect.position.x, rect.position.y},                              // Top-left
		{rect.position.x + rect.size.x, rect.position.y},                 // Top-right
		{rect.position.x, rect.position.y + rect.size.y},                // Bottom-left
		{rect.position.x + rect.size.x, rect.position.y + rect.size.y}    // Bottom-right
	};

	for (const auto& corner : corners)
	{
		// Tile coordinates of the corner
		int cx = static_cast<int>(std::floor(corner.x / TILE_SIZE));
		int cy = static_cast<int>(std::floor(corner.y / TILE_SIZE));

		// Check diagonal neighbors: tiles that share a vertex with the corner tile
		// Example: For top-left corner, check (cx-1, cy), (cx, cy-1), and (cx-1, cy-1)
		// If two of these neighbors are solid and adjacent diagonally, it blocks corner movement

		// Offsets for neighbor pairs
		const std::pair<sf::Vector2i, sf::Vector2i> neighborPairs[] = {
			{{-1, 0}, {0, -1}},   // top-left corner neighbors
			{{1, 0}, {0, -1}},    // top-right corner neighbors
			{{-1, 0}, {0, 1}},    // bottom-left corner neighbors
			{{1, 0}, {0, 1}}      // bottom-right corner neighbors
		};

		// Determine which neighbor pair to check for this corner
		int index = 0;
		if (corner == corners[0]) index = 0; // top-left
		else if (corner == corners[1]) index = 1; // top-right
		else if (corner == corners[2]) index = 2; // bottom-left
		else if (corner == corners[3]) index = 3; // bottom-right

		auto& pair = neighborPairs[index];

		sf::Vector2i n1(cx + pair.first.x, cy + pair.first.y);
		sf::Vector2i n2(cx + pair.second.x, cy + pair.second.y);

		if (isWithinBounds(n1) && isWithinBounds(n2))
		{
			if (getTile(n1).type == Tile::Type::Solid &&
				getTile(n2).type == Tile::Type::Solid)
			{
				// Additionally check the diagonal tile between these two neighbors
				sf::Vector2i diag(cx + pair.first.x + pair.second.x, cy + pair.first.y + pair.second.y);
				if (isWithinBounds(diag))
				{
					if (getTile(diag).type == Tile::Type::Solid)
					{
						return true; // Solid diagonal corner blocking movement
					}
				}
				else
				{
					// If diagonal tile out of bounds, treat as blocking
					return true;
				}
			}
		}
	}

	return false; // No collisions found
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