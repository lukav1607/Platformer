// ================================================================================================
// File: Pathfinding.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 24, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <queue>
#include <cmath>
#include <limits>
#include <functional>
#include "Pathfinding.hpp"

std::vector<sf::Vector2i> Pathfinding::reconstructPath(const std::unordered_map<sf::Vector2i, Node, Utility::Vector2iHasher>& cameFrom, sf::Vector2i current)
{
	std::vector<sf::Vector2i> path;
	while (cameFrom.at(current).hasParent)
	{
		path.push_back(current);
		current = cameFrom.at(current).parent;
	}
	std::reverse(path.begin(), path.end());

	return path;
}

std::vector<sf::Vector2i> Pathfinding::findPathAStar(const TileMap& tileMap, sf::Vector2i start, sf::Vector2i goal, Heuristic heuristic)
{
	using NodeMap = std::unordered_map<sf::Vector2i, Node, Utility::Vector2iHasher>;

	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
	NodeMap cameFrom;

	auto hFunc = (heuristic == Heuristic::Manhattan) ? manhattanHeuristic : euclideanHeuristic;

	Node startNode
	{
		start,
		0.f,
		hFunc(start, goal)
	};
	openSet.emplace(startNode);
	cameFrom[start] = startNode;

	const std::vector<sf::Vector2i> directions = (heuristic == Heuristic::Manhattan)
		? std::vector<sf::Vector2i>{ {1, 0}, { -1, 0 }, { 0, 1 }, { 0, -1 } }
	: std::vector<sf::Vector2i>{ {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };

	while (!openSet.empty())
	{
		Node current = openSet.top();
		openSet.pop();

		if (current.position == goal)
			return reconstructPath(cameFrom, goal);

		for (int dx = -1; dx <= 1; ++dx)
		{
			for (int dy = -1; dy <= 1; ++dy)
			{
				if (dx == 0 && dy == 0)
					continue;

				sf::Vector2i neighbor = current.position + sf::Vector2i(dx, dy);
				//int nx = current.position.x + dx;
				//int ny = current.position.y + dy;

				if (!tileMap.isWithinBounds(neighbor) || tileMap.getTile(neighbor).type == Tile::Type::Solid)
					continue;

				if (dx != 0 && dy != 0)
				{
					if (tileMap.getTile(current.position + sf::Vector2i(dx, 0)).type == Tile::Type::Solid ||
						tileMap.getTile(current.position + sf::Vector2i(0, dy)).type == Tile::Type::Solid)
					{
						continue; // Diagonal movement blocked by adjacent solid tile
					}
				}

				float cost = (dx != 0 && dy != 0) ? 1.414f : 1.f;
				float tentativeG = current.costFromStart + cost;

				if (cameFrom.find(neighbor) == cameFrom.end() || tentativeG < cameFrom[neighbor].costFromStart)
				{
					auto h = hFunc(neighbor, goal);

					Node neighborNode
					{
						neighbor,
						tentativeG,
						tentativeG + h,
						current.position,
						true
					};

					cameFrom[neighbor] = neighborNode;
					openSet.emplace(neighborNode);
				}
			}
		}
		/*for (const auto& dir : directions)
		{
			sf::Vector2i neighbor = current.position + dir;

			if (!tileMap.isWithinBounds(neighbor) || tileMap.getTile(neighbor).type == Tile::Type::Solid)
				continue;

			float cost = (dir.x != 0 && dir.y != 0) ? 1.414f : 1.f;
			float tentativeG = current.costFromStart + cost;

			if (cameFrom.find(neighbor) == cameFrom.end() || tentativeG < cameFrom[neighbor].costFromStart)
			{
				auto h = hFunc(neighbor, goal);

				Node neighborNode
				{
					neighbor,
					tentativeG,
					tentativeG + h,
					current.position,
					true
				};

				cameFrom[neighbor] = neighborNode;
				openSet.emplace(neighborNode);
			}
		}*/
	}
	return {}; // Return empty path if no path is found
}