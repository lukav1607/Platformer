// ================================================================================================
// File: Enemy.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 18, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <iostream>
#include <algorithm>
#include <string>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Text.hpp>
#include "Enemy.hpp"
#include "../../../core/Game.hpp"
#include "../../../core/Utility.hpp"
#include "../../../world/Pathfinding.hpp"

std::vector<sf::Vector2i> Enemy::usedPatrolPositions;
sf::Clock Enemy::clock;

Enemy::Enemy() :
	type(Type::Crawling),
	patrolSpeed(0.f),
	chaseSpeed(0.f),
	jumpCooldown(-1.f),
	jumpTimer(0.f),
	health(0),
	state(State::Patrol),
	isOnGround(false),
	aggroRange(0.f),
	followRange(0.f),
	currentPatrolTargetTile(0, 0),
	isCompleted(false),
	currentPathfindingIndex(0),
	timeSinceLastPathUpdate(0.f),
	stuckTimer(0.f),
	losTimer(0.f),
	losLostTimer(0.f)
{}

Enemy::Enemy(std::vector<sf::Vector2i> patrolPositions, Type type) :
	type(type),
	patrolSpeed(0.f),
	chaseSpeed(0.f),
	jumpCooldown(-1.f),
	jumpTimer(0.f),
	state(State::Patrol),
	patrolPositions(patrolPositions),
	isOnGround(false),
	isCompleted(true),
	currentPathfindingIndex(0),
	timeSinceLastPathUpdate(0.f),
	stuckTimer(0.f),
	losTimer(0.f),
	losLostTimer(0.f)
{
	initialize(type);
}

void Enemy::serialize(json& j) const
{
	j["type"] = static_cast<int>(type);
	//j["health"] = health;
	//j["isAggroed"] = isAggroed;
	//j["isReturningToPatrol"] = isReturningToPatrol;
	//j["aggroRange"] = aggroRange;
	//j["followRange"] = followRange;
	j["patrolPositions"] = json::array();
	for (const auto& pos : patrolPositions)
		j["patrolPositions"].push_back({ {"x", pos.x}, {"y", pos.y} });
	//j["currentPosition"] = { {"x", currentPosition.x}, {"y", currentPosition.y} };
	//j["previousPosition"] = { {"x", previousPosition.x}, {"y", previousPosition.y} };
}

void Enemy::deserialize(const json& j)
{
	type = static_cast<Type>(j["type"]);
	//health = j["health"];
	//isAggroed = j["isAggroed"];
	//isReturningToPatrol = j["isReturningToPatrol"];
	//aggroRange = j["aggroRange"];
	//followRange = j["followRange"];
	patrolPositions.clear();
	if (j.contains("patrolPositions"))
		for (const auto& pos : j["patrolPositions"])
			patrolPositions.emplace_back(pos["x"], pos["y"]);
	//currentPosition.x = j["currentPosition"]["x"];
	//currentPosition.y = j["currentPosition"]["y"];
	//previousPosition.x = j["previousPosition"]["x"];
	//previousPosition.y = j["previousPosition"]["y"];
	//size = { 32.f, 32.f }; // Default size, can be adjusted based on type
	//color = getColor(type);
	//shape.setSize(size);
	////shape.setFillColor(color);
	//for (const auto& patrolPosition : patrolPositions)
	//	usedPatrolPositions.push_back(patrolPosition);
	//initializePatrolPositions();
	initialize(type);
}

void Enemy::initialize(Type type)
{
	switch (type)
	{
	case Type::Crawling:
		size = { 36.f, 20.f };
		health = 2;
		patrolSpeed = 50.f;
		chaseSpeed = 0.f;
		aggroRange = -1.f;
		followRange = -1.f;
		break;
	case Type::Walking:
		size = { 52.f, 84.f };
		health = 3;
		patrolSpeed = 50.f;
		chaseSpeed = 100.f;
		jumpCooldown = 1.5f;
		jumpTimer = 0.f;
		aggroRange = 6 * TileMap::TILE_SIZE;
		followRange = 15 * TileMap::TILE_SIZE;
		break;
	case Type::Flying:
		size = { 36.f, 36.f };
		health = 2;
		patrolSpeed = 75.f;
		chaseSpeed = 125.f;
		aggroRange = 8 * TileMap::TILE_SIZE;
		followRange = 12 * TileMap::TILE_SIZE;
		break;
	}
	color = getColor(type);

	d_patrolTargetCircle.setRadius(5.f);
	d_patrolTargetCircle.setFillColor(sf::Color(255, 0, 0, 100));
	d_patrolTargetCircle.setOutlineColor(sf::Color::Red);
	d_patrolTargetCircle.setOutlineThickness(1.f);

	d_aggroRangeCircle.setRadius(aggroRange);
	d_aggroRangeCircle.setFillColor(sf::Color(255, 0, 0, 5));
	d_aggroRangeCircle.setOutlineColor(sf::Color::Red);
	d_aggroRangeCircle.setOutlineThickness(1.f);

	d_positionBeforeAggroCircle.setRadius(5.f);
	d_positionBeforeAggroCircle.setFillColor(sf::Color(255, 255, 0, 100));
	d_positionBeforeAggroCircle.setOutlineColor(sf::Color::Yellow);
	d_positionBeforeAggroCircle.setOutlineThickness(1.f);

	d_followRangeCircle.setRadius(followRange);
	d_followRangeCircle.setFillColor(sf::Color(255, 255, 0, 5));
	d_followRangeCircle.setOutlineColor(sf::Color::Yellow);
	d_followRangeCircle.setOutlineThickness(1.f);

	shape.setSize(size);
	shape.setFillColor(color);

	for (const auto& patrolPosition : patrolPositions)
		usedPatrolPositions.push_back(patrolPosition);
	initializePatrolPositions();

	isCompleted = true;
}

void Enemy::update(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition)
{
	updateMovement(fixedTimeStep, tileMap, playerPosition);

	switch (type)
	{
	case Type::Flying:

		break;

	case Type::Walking:
		break;

	case Type::Crawling:
		break;
	}

	resolveCollisions(fixedTimeStep, tileMap);

	if (Game::isDebugModeOn())
		updateDebugVisuals(tileMap, playerPosition);
}

void Enemy::render(sf::RenderWindow& window, sf::Font& font, float interpolationFactor)
{
	shape.setPosition(Utility::interpolate(previousPosition, currentPosition, interpolationFactor));
	window.draw(shape);

	if (Game::isDebugModeOn())
	{
		for (int i = 0; i < currentPathfindingPath.size(); ++i)
		{
			sf::Text nodeText(font, std::to_string(i), 18);
			nodeText.setFillColor(sf::Color::White);
			nodeText.setPosition(
				Utility::tileToWorldCoords(
					currentPathfindingPath.at(i)) + 
				sf::Vector2f(TileMap::TILE_SIZE / 2.f - nodeText.getGlobalBounds().size.x / 2.f,
					TileMap::TILE_SIZE / 2.f - nodeText.getGlobalBounds().size.y / 2.f));
			window.draw(nodeText);
		}

		window.draw(d_lineOfSightLine);
		window.draw(d_patrolTargetCircle);
		window.draw(d_aggroRangeCircle);

		if (state == State::Chasing || state == State::Returning)//isAggroed || isReturningToPatrol)
		{
			window.draw(d_positionBeforeAggroCircle);
			window.draw(d_followRangeCircle);
		}
	}
}

void Enemy::renderPatrolPositions(sf::RenderWindow& window, sf::Font& font)
{
	if (patrolPositions.empty())
		return;

	float time = clock.getElapsedTime().asSeconds();

	sf::Color lineColor = sf::Color(255, 255, 255, 200);
	if (!isCompleted)
		lineColor.a = Utility::getBreathingAlpha(time, 100, 200);

	for (int i = 0; i < patrolPositions.size(); ++i)
	{
		if (i + 1 < patrolPositions.size())
		{
			sf::VertexArray line(sf::PrimitiveType::Lines, 2);
			line.append(sf::Vertex{ Utility::tileToWorldCoords(patrolPositions.at(i)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f), lineColor });
			line.append(sf::Vertex{ Utility::tileToWorldCoords(patrolPositions.at(i + 1)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f), lineColor });
			window.draw(line);
		}
		else
		{
			if (isCompleted)
			{
				sf::VertexArray line(sf::PrimitiveType::Lines, 2);
				line.append(sf::Vertex{ Utility::tileToWorldCoords(patrolPositions.at(i)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f), lineColor });
				line.append(sf::Vertex{ Utility::tileToWorldCoords(patrolPositions.at(0)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f), lineColor });
				window.draw(line);
			}
		}

		sf::RectangleShape patrolShape(sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));
		patrolShape.setPosition(Utility::tileToWorldCoords(patrolPositions.at(i)));
		sf::Color color = getColor(type);
		color.a = isCompleted ? 100 : Utility::getBreathingAlpha(time, 50, 100);
		patrolShape.setFillColor(color);
		color.a = isCompleted ? 200 : Utility::getBreathingAlpha(time, 100, 200);
		patrolShape.setOutlineColor(color);
		patrolShape.setOutlineThickness(2.f);
		window.draw(patrolShape);

		sf::Text patrolText(font, std::to_string(i), 20);
		patrolText.setFillColor(sf::Color::White);
		patrolText.setPosition(Utility::tileToWorldCoords(patrolPositions.at(i)) + sf::Vector2f(5.f, 5.f));
		window.draw(patrolText);
	}
}

bool Enemy::isValidPatrolPosition(const TileMap& tileMap, Type type, sf::Vector2i tileCoords) const
{
	if (!patrolPositions.empty() && tileCoords == patrolPositions.at(0))
		return true;

	for (auto& patrolPos : patrolPositions)
		if (patrolPos == tileCoords)
			return false;

	for (auto& usedPatrolPos : usedPatrolPositions)
		if (usedPatrolPos == tileCoords)
			return false;

	if (!tileMap.isWithinBounds(tileCoords))
		return false;

	if (tileMap.getTile(tileCoords).type == Tile::Type::Solid)
		return false;

	sf::Vector2i above = tileCoords + sf::Vector2i(0, -1);
	sf::Vector2i below = tileCoords + sf::Vector2i(0, 1);
	sf::Vector2i left = tileCoords + sf::Vector2i(-1, 0);
	sf::Vector2i right = tileCoords + sf::Vector2i(1, 0);

	switch (type)
	{
	case Type::Crawling:
		if (tileMap.isWithinBounds(above))
			if (tileMap.getTile(above).type == Tile::Type::Solid)
				return true;
		if (tileMap.isWithinBounds(below))
			if (tileMap.getTile(below).type == Tile::Type::Solid)
				return true;
		if (tileMap.isWithinBounds(left))
			if (tileMap.getTile(left).type == Tile::Type::Solid)
				return true;
		if (tileMap.isWithinBounds(right))
			if (tileMap.getTile(right).type == Tile::Type::Solid)
				return true;
		return false;

	case Type::Walking:
		if (tileMap.isWithinBounds(below))
			if (tileMap.getTile(below).type == Tile::Type::Solid)
				return true;
		return false;

	case Type::Flying:
		return true;
	}
	return false;
}

void Enemy::addPatrolPosition(sf::Vector2i tileCoords)
{
	patrolPositions.push_back(tileCoords);
	if (patrolPositions.size() == 1)
		initializePatrolPositions();
}

void Enemy::removePatrolPosition(sf::Vector2i tileCoords)
{
	for (auto it = patrolPositions.begin(); it != patrolPositions.end(); ++it)
	{
		if (*it == tileCoords)
		{
			currentPatrolTargetTile = getNextPatrolTarget();
			patrolPositions.erase(it);
			break;
		}
	}
	for (auto it = usedPatrolPositions.begin(); it != usedPatrolPositions.end(); ++it)
	{
		if (*it == tileCoords)
		{
			//currentPatrolTargetTile = getNextPatrolTarget();
			usedPatrolPositions.erase(it);
			return;
		}
	}
}

void Enemy::applyHit(float damage, sf::Vector2f knockback)
{

}

void Enemy::resolveCollisions(float fixedTimeStep, const TileMap& tileMap)
{
	isOnGround = false;

	sf::Vector2f futurePosition = currentPosition + velocity * fixedTimeStep;
	sf::FloatRect futureBounds = { futurePosition, size };

	sf::Vector2f verticalFuturePos = { currentPosition.x, currentPosition.y + velocity.y * fixedTimeStep };
	sf::FloatRect verticalBounds(verticalFuturePos, size);

	for (int x = (int)(verticalBounds.position.x) / TileMap::TILE_SIZE - 1;
		x <= (int)(verticalBounds.position.x + verticalBounds.size.x) / TileMap::TILE_SIZE + 1;
		++x)
	{
		for (int y = (int)(verticalBounds.position.y) / TileMap::TILE_SIZE - 1;
			y <= (int)(verticalBounds.position.y + verticalBounds.size.y) / TileMap::TILE_SIZE + 1;
			++y)
		{
			sf::Vector2i tileCoords(x, y);

			if (!tileMap.isWithinBounds(tileCoords))
				continue;

			if (tileMap.getTile(tileCoords).type != Tile::Type::Solid)
				continue;

			sf::FloatRect tileBounds(
				sf::Vector2f(tileCoords.x * TileMap::TILE_SIZE, tileCoords.y * TileMap::TILE_SIZE),
				sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));

			auto intersection = verticalBounds.findIntersection(tileBounds);
			if (intersection.has_value())
			{
				if (velocity.y > 0.f)
				{
					verticalFuturePos.y -= intersection.value().size.y;
					isOnGround = true;
				}
				else if (velocity.y < 0.f)
				{
					verticalFuturePos.y += intersection.value().size.y;
				}
				velocity.y = 0.f;
				verticalBounds.position.y = verticalFuturePos.y;
			}
		}
	}
	futurePosition.y = verticalFuturePos.y;

	sf::Vector2f horizontalFuturePos = { currentPosition.x + velocity.x * fixedTimeStep, futurePosition.y };
	sf::FloatRect horizontalBounds(horizontalFuturePos, size);

	for (int x = (int)(horizontalBounds.position.x) / TileMap::TILE_SIZE - 1;
		x <= (int)(horizontalBounds.position.x + horizontalBounds.size.x) / TileMap::TILE_SIZE + 1;
		++x)
	{
		for (int y = (int)(horizontalBounds.position.y) / TileMap::TILE_SIZE - 1;
			y <= (int)(horizontalBounds.position.y + horizontalBounds.size.y) / TileMap::TILE_SIZE + 1;
			++y)
		{
			sf::Vector2i tileCoords(x, y);

			if (!tileMap.isWithinBounds(tileCoords))
				continue;

			if (tileMap.getTile(tileCoords).type != Tile::Type::Solid)
				continue;

			sf::FloatRect tileBounds(
				sf::Vector2f(tileCoords.x * TileMap::TILE_SIZE, tileCoords.y * TileMap::TILE_SIZE),
				sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));

			auto intersection = horizontalBounds.findIntersection(tileBounds);
			if (intersection.has_value())
			{
				if (velocity.x > 0.f) // Moving right
				{
					horizontalFuturePos.x -= intersection->size.x;
				}
				else if (velocity.x < 0.f) // Moving left
				{
					horizontalFuturePos.x += intersection->size.x;
				}
				velocity.x = 0.f;
				horizontalBounds.position.x = horizontalFuturePos.x;				
			}
		}
	}
	futurePosition.x = horizontalFuturePos.x;

	previousPosition = currentPosition;
	currentPosition = futurePosition;
}

void Enemy::updateMovement(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition)
{
	timeSinceLastPathUpdate += fixedTimeStep;

	switch (state)
	{
	case State::Chasing:
		handleChasing(tileMap, playerPosition, fixedTimeStep);
		break;

	case State::Returning:
		handleReturning(tileMap, fixedTimeStep);
		break;

	case State::Patrol:
		handlePatrolling(tileMap, playerPosition, fixedTimeStep);
		break;
	}
}

void Enemy::handlePatrolling(const TileMap& tileMap, sf::Vector2f playerPosition, float fixedTimeStep)
{
	sf::Vector2f center = getLogicPositionCenter();		
	currentPatrolTargetPixels = TileMap::getTileCenter(currentPatrolTargetTile);

	float distToPatrolTarget = std::hypotf(currentPatrolTargetPixels.x - center.x, currentPatrolTargetPixels.y - center.y);
	float distToPlayer = std::hypotf(playerPosition.x - center.x, playerPosition.y - center.y);

	if (distToPatrolTarget <= patrolSpeed * fixedTimeStep)
	{
		currentPatrolTargetTile = getNextPatrolTarget();
	}

	if (distToPlayer < aggroRange && Utility::hasLineOfSight(center, playerPosition, tileMap))
	{
		positionBeforeAggro = currentPosition + size / 2.f;
		d_positionBeforeAggroCircle.setPosition(positionBeforeAggro - sf::Vector2f(d_positionBeforeAggroCircle.getRadius(), d_positionBeforeAggroCircle.getRadius()));
		state = State::Chasing;
		return;
	}
	else
	{
		//if (Utility::hasLineOfSight(center, currentPatrolTargetPixels, tileMap))
		if (Utility::hasLineOfSightWithClearance(center, currentPatrolTargetPixels, size, tileMap))
			losTimer += fixedTimeStep;
		else
			losTimer = 0.f;

		if (losTimer >= LOS_THRESHOLD)
		{
			currentPathfindingPath.clear();
			moveTowards(currentPatrolTargetPixels, fixedTimeStep);
		}
		else
		{
			if (timeSinceLastPathUpdate >= PATH_UPDATE_INTERVAL)
			{
				updatePathfinding(tileMap, currentPatrolTargetPixels, fixedTimeStep);
				timeSinceLastPathUpdate = 0.f;
			}
			followPath(tileMap, fixedTimeStep);
		}
	}
}

void Enemy::handleChasing(const TileMap& tileMap, sf::Vector2f playerPosition, float fixedTimeStep)
{
	sf::Vector2f center = getLogicPositionCenter();
	float distToReturn = std::hypotf(positionBeforeAggro.x - center.x, positionBeforeAggro.y - center.y);

	//if (Utility::hasLineOfSight(center, playerPosition, tileMap))
	if (Utility::hasLineOfSightWithClearance(center, playerPosition, size, tileMap))
	{
		losTimer += fixedTimeStep;
		losLostTimer = 0.f;
	}
	else
	{
		losTimer = 0.f;
		losLostTimer += fixedTimeStep;
	}

	if (distToReturn > followRange || losLostTimer >= LOS_LOST_THRESHOLD)
	{
		losLostTimer = 0.f;
		losTimer = 0.f;
		currentPathfindingPath.clear();
		state = State::Returning;
		return;
	}

	if (losTimer >= LOS_THRESHOLD)
	{
		currentPathfindingPath.clear();
		moveTowards(playerPosition, fixedTimeStep);
	}
	else
	{
		if (timeSinceLastPathUpdate >= PATH_UPDATE_INTERVAL)
		{
			updatePathfinding(tileMap, playerPosition, fixedTimeStep);
			timeSinceLastPathUpdate = 0.f;
		}
		followPath(tileMap, fixedTimeStep);
	}
}

void Enemy::handleReturning(const TileMap& tileMap, float fixedTimeStep)
{
	sf::Vector2f center = getLogicPositionCenter();
	float distToReturn = std::hypotf(positionBeforeAggro.x - center.x, positionBeforeAggro.y - center.y);

	if (distToReturn <= chaseSpeed * fixedTimeStep && (type != Type::Walking || isOnGround))
	{
		losTimer = 0.f;
		state = State::Patrol;
		return;
	}

	//if (Utility::hasLineOfSight(center, positionBeforeAggro, tileMap))
	if (Utility::hasLineOfSightWithClearance(center, positionBeforeAggro, size / 2.f, tileMap))
		losTimer += fixedTimeStep;
	else
		losTimer = 0.f;

	if (losTimer >= LOS_THRESHOLD)
	{
		currentPathfindingPath.clear();
		moveTowards(positionBeforeAggro, fixedTimeStep);
	}
	else
	{
		if (timeSinceLastPathUpdate >= PATH_UPDATE_INTERVAL)
		{
			updatePathfinding(tileMap, positionBeforeAggro, fixedTimeStep);
			timeSinceLastPathUpdate = 0.f;
		}
		followPath(tileMap, fixedTimeStep);
	}
}

void Enemy::updateDebugVisuals(const TileMap& tileMap, sf::Vector2f playerPosition)
{
	bool hasLineOfSight = Utility::hasLineOfSight(getLogicPositionCenter(), playerPosition, tileMap);
	sf::Color lineColor = hasLineOfSight ? sf::Color::Green : sf::Color::Red;
	d_lineOfSightLine = sf::VertexArray(sf::PrimitiveType::Lines, 2);
	d_lineOfSightLine.append(sf::Vertex{ getLogicPositionCenter(), lineColor });
	d_lineOfSightLine.append(sf::Vertex{ playerPosition, lineColor });

	d_patrolTargetCircle.setPosition(currentPatrolTargetPixels - sf::Vector2f(d_patrolTargetCircle.getRadius(), d_patrolTargetCircle.getRadius()));
	d_aggroRangeCircle.setPosition(getLogicPositionCenter() - sf::Vector2f(aggroRange, aggroRange));
	d_followRangeCircle.setPosition(positionBeforeAggro - sf::Vector2f(followRange, followRange));
}

void Enemy::updatePathfinding(const TileMap& tileMap, sf::Vector2f target, float fixedTimeStep)
{
	sf::Vector2i start = Utility::worldToTileCoords(getLogicPositionCenter());
	sf::Vector2i goal = Utility::worldToTileCoords(target);

	if (!currentPathfindingPath.empty() && goal == currentPathfindingPath.back())
		return; // Already at the goal

	using namespace Pathfinding;
	currentPathfindingPath = findPathAStar(type, tileMap, start, goal);
	//smoothPath(tileMap, fixedTimeStep);
	currentPathfindingIndex = 0;
}

void Enemy::followPath(const TileMap& tileMap, /*sf::Vector2f target, */float fixedTimeStep)
{
	if (currentPathfindingPath.empty() || currentPathfindingIndex >= currentPathfindingPath.size())
	{
		velocity = { 0.f, 0.f };
		return; // No path to follow or index out of bounds
	}

	sf::Vector2f center = getLogicPositionCenter();
	sf::Vector2f target = TileMap::getTileCenter(currentPathfindingPath.at(currentPathfindingIndex));

	float dist = std::hypotf(target.x - center.x, target.y - center.y);

	if (dist <= chaseSpeed * fixedTimeStep)
	{
		currentPathfindingIndex++;
		if (currentPathfindingIndex >= currentPathfindingPath.size())
		{
			velocity = { 0.f, 0.f };
			return;
		}

		target = TileMap::getTileCenter(currentPathfindingPath.at(currentPathfindingIndex));
	}
	moveTowards(target, fixedTimeStep);
}

void Enemy::smoothPath(const TileMap& tileMap, float fixedTimeStep)
{
	if (currentPathfindingPath.size() <= 2)
		return; // Already as smooth as it gets

	std::vector<sf::Vector2i> smoothed;
	std::size_t startIndex = 0;

	while (startIndex < currentPathfindingPath.size() - 1)
	{
		// Look as far ahead as possible
		std::size_t nextIndex = startIndex + 1;

		while (nextIndex < currentPathfindingPath.size() &&
				/*!Utility::hasLineOfSight(
					TileMap::getTileCenter(currentPathfindingPath.at(startIndex)),
					TileMap::getTileCenter(currentPathfindingPath.at(nextIndex)),
					tileMap)*/
			!Utility::hasLineOfSightWithClearance(
				TileMap::getTileCenter(currentPathfindingPath.at(startIndex)),
				TileMap::getTileCenter(currentPathfindingPath.at(nextIndex)),
				size,
				tileMap))
		{
			++nextIndex;
		}
		std::size_t bestIndex = nextIndex > startIndex + 1 ? nextIndex - 1 : startIndex + 1;

		smoothed.push_back(currentPathfindingPath.at(startIndex));
		startIndex = bestIndex;
	}

	smoothed.push_back(currentPathfindingPath.back());
	currentPathfindingPath = smoothed;
	currentPathfindingIndex = 0;
}

void Enemy::initializePatrolPositions()
{
	if (this->patrolPositions.empty())
	{
		std::cerr << "Error: Failed to create Enemy - patrol positions cannot be empty." << std::endl;
		return;
	}

	if (this->patrolPositions.size() > 1)
		currentPatrolTargetTile = this->patrolPositions.at(1);
	else
		currentPatrolTargetTile = this->patrolPositions.at(0);

	switch (type)
	{
	case Type::Crawling:
		currentPosition = Utility::tileToWorldCoords(this->patrolPositions.at(0)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f) - size / 2.f;
		break;
	case Type::Walking:
		currentPosition = Utility::tileToWorldCoords(this->patrolPositions.at(0)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE) - sf::Vector2f(size.x / 2.f, size.y);
		break;
	case Type::Flying:
		currentPosition = Utility::tileToWorldCoords(this->patrolPositions.at(0)) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f) - size / 2.f;
		break;
	}
	previousPosition = currentPosition;
}

sf::Vector2i Enemy::getNextPatrolTarget() const
{
	if (currentPatrolTargetTile == patrolPositions.back())
		return patrolPositions.front();
	for (int i = 0; i < patrolPositions.size() - 1; ++i)
	{
		if (currentPatrolTargetTile == patrolPositions.at(i))
			return patrolPositions.at(i + 1);
	}
	return currentPatrolTargetTile;
}

void Enemy::moveTowards(sf::Vector2f target, float fixedTimeStep)
{
	if (type == Type::Walking)
	{
		sf::Vector2f direction = target - getPixelPositionBottomCenter();
		float distance = std::hypotf(direction.x, direction.y);
		return;

		if (distance != 0.f)
		{
			direction /= distance;
			velocity.x = direction.x * std::min(state == State::Chasing || state == State::Returning ? chaseSpeed : patrolSpeed, distance / fixedTimeStep);
		}
		else
		{
			velocity.x = 0.f; // Already at target
		}
	}
	sf::Vector2f direction = target - getLogicPositionCenter();
	float distance = std::hypotf(direction.x, direction.y);

	if (distance != 0.f)
	{
		direction /= distance;
		velocity = direction * std::min(state == State::Chasing || state == State::Returning ? chaseSpeed : patrolSpeed, distance / fixedTimeStep);
	}
	else
	{
		velocity = { 0.f, 0.f }; // Already at target
	}
}