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
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../../world/TileMap.hpp"
#include "../../core/Utility.hpp"
#include "../../core/Serializable.hpp"

class Enemy : public Serializable
{
public:
	enum class Type
	{
		//None,
		Crawling,
		Walking,
		Flying
	}; 
	enum class State {
		Patrol,
		Chasing,
		Returning,
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

	void serialize(json& j) const override;
	void deserialize(const json& j) override;

	void initialize(Type type);

	void update(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition);
	void render(sf::RenderWindow& window, sf::Font& font, float interpolationFactor);
	void renderPatrolPositions(sf::RenderWindow& window, sf::Font& font);

	void setType(Type type) { this->type = type; }

	bool isValidPatrolPosition(const TileMap& tileMap, Type type, sf::Vector2i tileCoords) const;
	void addPatrolPosition(sf::Vector2i tileCoords);
	void removePatrolPosition(sf::Vector2i tileCoords);
	void clearPatrolPositions() { patrolPositions.clear(); }
	const std::vector<sf::Vector2i>& getPatrolPositions() const { return patrolPositions; }

	void applyHit(float damage, sf::Vector2f knockback);

	void equalizePositions() { currentPosition = previousPosition; }
	void updateDebugVisuals(const TileMap& tileMap, sf::Vector2f playerPosition);

	//bool isAtPatrolTarget() const;

	inline sf::FloatRect getBounds() const { return sf::FloatRect(currentPosition, size); }
	inline sf::Vector2f getLogicPositionCenter() const { return currentPosition + sf::Vector2f(size.x / 2.f, size.y / 2.f); }
	inline bool isAlive() const { return health > 0; }

private:
	//void updateFlying(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition);
	void updateMovement(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition);

	void handlePatrolling(const TileMap& tileMap, sf::Vector2f playerPosition, float fixedTimeStep);
	void handleChasing(const TileMap& tileMap, sf::Vector2f playerPosition, float fixedTimeStep);
	void handleReturning(const TileMap& tileMap, float fixedTimeStep);

	void updatePathfinding(const TileMap& tileMap, sf::Vector2f target, float fixedTimeStep);
	void followPath(const TileMap& tileMap, /*sf::Vector2f target, */float fixedTimeStep);
	void smoothPath(const TileMap& tileMap, float fixedTimeStep);
	//bool tryInsertDetourNode(const TileMap& tileMap);
	//bool isStuck();

	void initializePatrolPositions();
	sf::Vector2i getNextPatrolTarget() const;

	void moveTowards(sf::Vector2f target, float fixedTimeStep);
	void resolveCollisions(float fixedTimeStep, const TileMap& tileMap);

	//const float PATROL_SPEED = 75.f;
	//const float CHASE_SPEED = 125.f;
	const float JUMP_VELOCITY = -875.f;
	const float GRAVITY = 1500.f;
	const float TERMINAL_VELOCITY = 1250.f;
	float patrolSpeed;
	float chaseSpeed;

	static std::vector<sf::Vector2i> usedPatrolPositions;
	static sf::Clock clock;

	Type type;
	State state;
	bool isCompleted;
	int health;
	//bool isAggroed;
	//bool isReturningToPatrol;
	float aggroRange;
	float followRange;

	std::vector<sf::Vector2i> patrolPositions;
	sf::Vector2i currentPatrolTargetTile;
	sf::Vector2f currentPatrolTargetPixels;

	const float PATH_UPDATE_INTERVAL = 0.3f; // Time in seconds between pathfinding updates
	float timeSinceLastPathUpdate;
	float stuckTimer;
	std::vector<sf::Vector2i> currentPathfindingPath;
	std::size_t currentPathfindingIndex;
	//sf::Vector2i lastPathfindingTarget;
	//sf::Vector2f lastPosition;
	//Utility::LoSWithHysteresis losChecker;
	float losTimer;
	float losLostTimer;
	const float LOS_THRESHOLD = 0.3f;
	const float LOS_LOST_THRESHOLD = 8.0f;
	//sf::Vector2f stuckLastPosition;
	//int stuckCounter = 0;
	//const int STUCK_THRESHOLD_FRAMES = 10;    // Number of frames stuck to trigger detour
	//const float STUCK_DISTANCE_THRESHOLD = 0.05f; // Distance moved threshold to consider stuck

	sf::Vector2f currentPosition;
	sf::Vector2f previousPosition;
	sf::Vector2f positionBeforeAggro;
	sf::Vector2f velocity;
	bool isOnGround;

	sf::RectangleShape shape;
	sf::Vector2f size;
	sf::Color color;

	// DEBUG
	sf::CircleShape d_patrolTargetCircle;
	sf::CircleShape d_aggroRangeCircle;
	sf::CircleShape d_positionBeforeAggroCircle;
	sf::CircleShape d_followRangeCircle;
	sf::VertexArray d_lineOfSightLine;
};