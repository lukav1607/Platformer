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
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../../world/TileMap.hpp"

class Enemy
{
public:
	enum class Type
	{
		//None,
		Crawling,
		Walking,
		Flying
	};
	inline static constexpr sf::Color getColor(Type type)
	{
		switch (type)
		{
		case Type::Crawling: return sf::Color(200, 160, 100);
		case Type::Walking: return 	sf::Color(140, 180, 220);
		case Type::Flying: return 	sf::Color(180, 140, 220);
		default: return sf::Color::White;
		}
	}

	Enemy();
	Enemy(std::vector<sf::Vector2i> patrolPositions, Type type/*, sf::Vector2f size, sf::Color color, int health, bool isPassive*/);

	void update(float fixedTimeStep, const TileMap& tileMap);
	void render(sf::RenderWindow& window, float interpolationFactor);
	void renderPatrolPositions(sf::RenderWindow& window, sf::Font& font);

	void setType(Type type) { this->type = type; }

	bool isValidPatrolPosition(const TileMap& tileMap, Type type, sf::Vector2i tileCoords) const;
	void addPatrolPosition(sf::Vector2i tileCoords);
	void removePatrolPosition(sf::Vector2i tileCoords);
	void clearPatrolPositions() { patrolPositions.clear(); }
	const std::vector<sf::Vector2i>& getPatrolPositions() const { return patrolPositions; }

	void applyHit(float damage, sf::Vector2f knockback);

	void equalizePositions() { currentPosition = previousPosition; }

	//bool isAtPatrolTarget() const;

	inline sf::FloatRect getBounds() const { return sf::FloatRect(currentPosition, size); }
	inline sf::Vector2f getCenter() const { return currentPosition + size / 2.f; }
	inline bool isAlive() const { return health > 0; }

private:
	void resolveCollisions(float fixedTimeStep, const TileMap& tileMap);

	void initializePatrolPositions();
	sf::Vector2i getNextPatrolTarget() const;

	const float MOVE_SPEED = 100.f;
	const float GRAVITY = 1500.f;
	const float MAX_FALL_SPEED = 500.f;

	static std::vector<sf::Vector2i> usedPatrolPositions;
	static sf::Clock clock;

	Type type;
	bool isCompleted;
	int health;
	bool isAggroed;
	float aggroRange;

	std::vector<sf::Vector2i> patrolPositions;
	sf::Vector2i currentTargetTile;
	sf::Vector2f currentTargetPixels;

	sf::Vector2f currentPosition;
	sf::Vector2f previousPosition;
	sf::Vector2f direction;
	sf::Vector2f velocity;
	bool isOnGround;

	sf::RectangleShape shape;
	sf::Vector2f size;
	sf::Color color;
};