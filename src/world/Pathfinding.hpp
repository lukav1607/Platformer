// ================================================================================================
// File: Pathfinding.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 24, 2025
// Description: Defines the Pathfinding namespace, which contains functions for pathfinding algorithms.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <vector>
#include <unordered_map>
#include <SFML/System.hpp>
#include "../core/Utility.hpp"
#include "../state/game/enemies/FlyingEnemy.hpp"

namespace Pathfinding
{
	enum class Heuristic
	{
		Manhattan,
		Euclidean
	};

	struct Node
	{
        sf::Vector2i position;
        float costFromStart = 0.f; // g cost
		float estimatedTotalCost = 0.f; // f cost = g + h
        sf::Vector2i parent;
        bool hasParent = false;

        bool operator>(const Node& other) const
        {
            return estimatedTotalCost > other.estimatedTotalCost;
        }
	};

	inline float euclideanHeuristic(const sf::Vector2i& a, const sf::Vector2i& b)
	{
		return std::hypotf(static_cast<float>(a.x - b.x), static_cast<float>(a.y - b.y));
	}

	inline float manhattanHeuristic(const sf::Vector2i& a, const sf::Vector2i& b)
	{
		return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
	}

	std::vector<sf::Vector2i> getReachableNeighbors(const TileMap& map, const sf::Vector2i& tile);

	std::vector<sf::Vector2i> reconstructPath(const std::unordered_map<sf::Vector2i, Node, Utility::Vector2iHasher>& cameFrom, sf::Vector2i current);

	// Finds the shortest path from `start` to `goal` using the A* algorithm.
	// The heuristic can be either Manhattan or Euclidean.
	// Returns a vector of tile coordinates representing the path.
	std::vector<sf::Vector2i> findPathAStar(const TileMap& tileMap, sf::Vector2i start, sf::Vector2i goal);
}