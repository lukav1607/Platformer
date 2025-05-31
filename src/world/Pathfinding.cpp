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

std::vector<sf::Vector2i> Pathfinding::getReachableNeighbors(const TileMap& map, const sf::Vector2i& tile)
{
	std::vector<sf::Vector2i> neighbors;

	//switch (type)
	//{
	//case FlyingEnemy::Type::Walking:
	//{
	//	sf::Vector2i below(tile.x, tile.y + 1);
	//	bool hasGroundBelow = map.isSolid(below);

	//	// Standing on solid ground
	//	if (hasGroundBelow)
	//	{
	//		// Walk left/right if destination and below are solid
	//		for (int dx : {-1, 1})
	//		{
	//			sf::Vector2i side(tile.x + dx, tile.y);
	//			sf::Vector2i sideBelow(side.x, side.y + 1);

	//			if (!map.isSolid(side) && map.isSolid(sideBelow))
	//				neighbors.push_back(side);
	//		}

	//		// Jump to tiles up-left, up, up-right (1 tile high)
	//		for (int dx : {-1, 0, 1})
	//		{
	//			sf::Vector2i jumpTarget(tile.x + dx, tile.y - 1);
	//			sf::Vector2i jumpBelow(jumpTarget.x, jumpTarget.y + 1);
	//			sf::Vector2i jumpAbove(jumpTarget.x, jumpTarget.y - 1); // Assuming 2-tile high enemies

	//			if (!map.isSolid(jumpTarget) && map.isSolid(jumpBelow) && !map.isSolid(jumpAbove))
	//				neighbors.push_back(jumpTarget);
	//		}

	//		// Jump over small 1-tile-wide hole
	//		for (int dx : {-2, 2})
	//		{
	//			sf::Vector2i target(tile.x + dx, tile.y);
	//			sf::Vector2i targetBelow(target.x, target.y + 1);

	//			sf::Vector2i mid(tile.x + dx / 2, tile.y);
	//			sf::Vector2i midBelow(mid.x, mid.y + 1);

	//			if (!map.isSolid(target) && map.isSolid(targetBelow) &&
	//				!map.isSolid(mid) && !map.isSolid(midBelow))
	//			{
	//				neighbors.push_back(target);
	//			}
	//		}
	//	}

	//	// Allow drop down to one tile below if the target tile is walkable
	//	sf::Vector2i oneBelow(tile.x, tile.y + 1);
	//	sf::Vector2i twoBelow(tile.x, tile.y + 2);

	//	if (!map.isSolid(oneBelow) && map.isSolid(twoBelow))
	//	{
	//		neighbors.push_back(oneBelow);
	//	}
	//	break;
	//}
	//case FlyingEnemy::Type::Flying:
		for (int dx = -1; dx <= 1; ++dx)
		{
			for (int dy = -1; dy <= 1; ++dy)
			{
				if (dx == 0 && dy == 0)
					continue;

				sf::Vector2i neighbor = tile + sf::Vector2i(dx, dy);

				if (!map.isWithinBounds(neighbor) || map.isSolid(neighbor))
					continue;

				if (dx != 0 && dy != 0)
				{
					if (map.isSolid(tile + sf::Vector2i(dx, 0)) ||
						map.isSolid(tile + sf::Vector2i(0, dy)))
					{
						continue; // Diagonal movement blocked by adjacent solid tile
					}
				}
				neighbors.push_back(neighbor);
			}
		}
		//break;
	//}
	return neighbors;
}

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

std::vector<sf::Vector2i> Pathfinding::findPathAStar(const TileMap& tileMap, sf::Vector2i start, sf::Vector2i goal)
{
	using NodeMap = std::unordered_map<sf::Vector2i, Node, Utility::Vector2iHasher>;

	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
	NodeMap cameFrom;

	auto hFunc = /*type == Enemy::Type::Flying ?*/ euclideanHeuristic /*: manhattanHeuristic*/;

	Node startNode
	{
		start,
		0.f,
		hFunc(start, goal)
	};
	openSet.emplace(startNode);
	cameFrom[start] = startNode;

	while (!openSet.empty())
	{
		Node current = openSet.top();
		openSet.pop();

		if (current.position == goal)
			return reconstructPath(cameFrom, goal);

		for (const auto& neighbor : getReachableNeighbors(tileMap, current.position))
		{
			float cost = 1.f;
			//if (type == FlyingEnemy::Type::Flying)
			//{
				sf::Vector2i delta = neighbor - current.position;
				cost = (delta.x != 0 && delta.y != 0) ? 1.414f : 1.f;
			//}

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
	return {}; // Return empty path if no path is found
}