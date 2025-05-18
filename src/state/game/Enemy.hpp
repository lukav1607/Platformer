// ================================================================================================
// File: Enemy.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 18, 2025
// Description: Defines the Enemy class, which represents an enemy character in the game. The Enemy
//              class is responsible for handling enemy behavior, movement, and interactions with the game world.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../../world/TileMap.hpp"

class Enemy
{
public:
	Enemy(std::vector<sf::Vector2i> patrolPositions, sf::Vector2f size, sf::Color color, int health, bool isPassive);

	void update(float fixedTimeStep, const TileMap& tileMap);
	void render(sf::RenderWindow& window, float interpolationFactor);

	void applyHit(float damage, sf::Vector2f knockback);

	inline sf::FloatRect getBounds() const { return sf::FloatRect(currentPosition, size); }
	inline bool isAlive() const { return health > 0; }

private:
	void resolveCollisions(float fixedTimeStep, const TileMap& tileMap);
	sf::Vector2i getNextPatrolTarget() const;

	const float MOVE_SPEED = 100.f;
	const float GRAVITY = 1500.f;
	const float MAX_FALL_SPEED = 500.f;

	int health;
	bool isAggroed;
	float aggroRange;

	std::vector<sf::Vector2i> patrolPositions;
	sf::Vector2i currentPatrolTarget;
	bool hasJustChangedPatrolTarget;

	sf::Vector2f currentPosition;
	sf::Vector2f previousPosition;
	sf::Vector2f direction;
	sf::Vector2f velocity;
	bool isOnGround;

	sf::RectangleShape shape;
	sf::Vector2f size;
	sf::Color color;
};