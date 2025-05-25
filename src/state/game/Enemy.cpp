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
#include "../../core/Game.hpp"
#include "../../core/Utility.hpp"
#include "../../world/Pathfinding.hpp"

std::vector<sf::Vector2i> Enemy::usedPatrolPositions;
sf::Clock Enemy::clock;

Enemy::Enemy() :
	type(Type::Crawling),
	health(0),
	isAggroed(false),
	isReturningToPatrol(false),
	isOnGround(false),
	aggroRange(0.f),
	followRange(0.f),
	currentPatrolTargetTile(0, 0),
	isCompleted(false),
	currentPathfindingIndex(0),
	timeSinceLastPathUpdate(0.f),
	//lastPathfindingTarget(-1, -1),
	stuckTimer(0.f),
	//stuckCounter(0),
	//stuckLastPosition(currentPosition),
	//losChecker(0.f, 0.f),
	losTimer(0.f)
{}

Enemy::Enemy(std::vector<sf::Vector2i> patrolPositions, Type type) :
	type(type),
	isAggroed(false),
	isReturningToPatrol(false),
	patrolPositions(patrolPositions),
	isOnGround(false),
	isCompleted(true),
	currentPathfindingIndex(0),
	timeSinceLastPathUpdate(0.f),
	//lastPathfindingTarget(-1, -1),
	stuckTimer(0.f),
	//stuckCounter(0),
	losTimer(0.f)
{
	switch (type)
	{
	case Type::Crawling:
		size = { 32.f, 16.f };
		health = 2;
		break;
	case Type::Walking:
		size = { 32.f, 48.f };
		health = 3;
		aggroRange = 300;
		followRange = 600;
		break;
	case Type::Flying:
		size = { 32.f, 32.f };
		health = 2;
		aggroRange = 400;
		followRange = 700;
		break;
	}

	//losChecker.clearanceThreshold = size.x * 1.5f; // Clearance for line of sight checks

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

	//aggroRange = isPassive ? 0.f : 500.f;
	shape.setSize(size);
	shape.setFillColor(color);

	for (const auto& patrolPosition : patrolPositions)
		usedPatrolPositions.push_back(patrolPosition);
	initializePatrolPositions();
	//stuckLastPosition = currentPosition;
}

void Enemy::update(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition)
{
	switch (type)
	{
	case Type::Flying:
		updateFlying(fixedTimeStep, tileMap, playerPosition);
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

		if (isAggroed || isReturningToPatrol)
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
			patrolPositions.erase(it);
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

void Enemy::updateFlying(float fixedTimeStep, const TileMap& tileMap, sf::Vector2f playerPosition)
{
	timeSinceLastPathUpdate += fixedTimeStep;

	sf::Vector2f center = getLogicPositionCenter();
	float distToPlayer = std::hypotf(playerPosition.x - center.x, playerPosition.y - center.y);

	if (isAggroed)
	{
		float distToReturn = std::hypotf(positionBeforeAggro.x - center.x, positionBeforeAggro.y - center.y);

		if (distToReturn > followRange)
		{
			losTimer = 0.f;
			isAggroed = false;
			isReturningToPatrol = true;
			currentPathfindingPath.clear();
			return;
		}

		if (Utility::hasLineOfSight(center, playerPosition, tileMap))
			losTimer += fixedTimeStep;
		else
			losTimer = 0.f;

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
			followPath(tileMap, /*playerPosition, */fixedTimeStep);
		}
		return;
	}

	if (isReturningToPatrol)
	{
		float distToReturn = std::hypotf(positionBeforeAggro.x - center.x, positionBeforeAggro.y - center.y);

		if (distToReturn <= CHASE_SPEED * fixedTimeStep)
		{
			isReturningToPatrol = false;
			losTimer = 0.f;
		}

		if (Utility::hasLineOfSight(center, positionBeforeAggro, tileMap))
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
			followPath(tileMap, /*positionBeforeAggro, */fixedTimeStep);
		}
		return;
	}

	// Patrolling
	currentPatrolTargetPixels = TileMap::getTileCenter(currentPatrolTargetTile);
	float distToPatrolTarget = std::hypotf(currentPatrolTargetPixels.x - center.x, currentPatrolTargetPixels.y - center.y);

	if (distToPatrolTarget <= PATROL_SPEED * fixedTimeStep)
		currentPatrolTargetTile = getNextPatrolTarget();

	if (distToPlayer < aggroRange && Utility::hasLineOfSight(center, playerPosition, tileMap))
	{
		isAggroed = true;
		positionBeforeAggro = currentPosition + size / 2.f;
		d_positionBeforeAggroCircle.setPosition(positionBeforeAggro - sf::Vector2f(d_positionBeforeAggroCircle.getRadius(), d_positionBeforeAggroCircle.getRadius()));
	}
	else
	{
		moveTowards(currentPatrolTargetPixels, fixedTimeStep);
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
	//lastPathfindingTarget = goal;
	currentPathfindingPath = findPathAStar(tileMap, start, goal, Heuristic::Euclidean);
	smoothPath(tileMap, fixedTimeStep);
	currentPathfindingIndex = 0;
}

void Enemy::followPath(const TileMap& tileMap, /*sf::Vector2f target, */float fixedTimeStep)
{
	//const float NODE_REACH_EPSILON = CHASE_SPEED * 1.5f / 60.f; // adjust for frame rate
	//const float STUCK_TIMEOUT = 0.75f;
	//const float STUCK_EPSILON = 2.f; // distance threshold for stuck detection

	//if (currentPathfindingPath.empty() || currentPathfindingIndex >= currentPathfindingPath.size())
	//{
	//	velocity = { 0.f, 0.f };
	//	return;
	//}

	//sf::Vector2f currentTarget = TileMap::getTileCenter(currentPathfindingPath[currentPathfindingIndex]);
	//moveTowards(currentTarget, fixedTimeStep);

	//sf::Vector2f center = getLogicPositionCenter();
	//float dist = std::hypot(currentTarget.x - center.x, currentTarget.y - center.y);

	//// Progress check
	//if (dist <= NODE_REACH_EPSILON)
	//{
	//	currentPathfindingIndex++;
	//	stuckTimer = 0.f;
	//	lastPosition = center;
	//}
	//else
	//{
	//	float moved = std::hypotf(center.x - lastPosition.x, center.y - lastPosition.y);
	//	if (moved <= STUCK_EPSILON)
	//	{
	//		stuckTimer += fixedTimeStep;
	//		if (stuckTimer > STUCK_TIMEOUT)
	//		{
	//			bool wasDetourInserted = tryInsertDetourNode(tileMap);
	//			if (wasDetourInserted)
	//			{
	//				stuckTimer = 0.f;
	//				return; // Detour inserted, no need to fallback
	//			}

	//			updatePathfinding(tileMap, target, fixedTimeStep); // fallback full A*
	//			stuckTimer = 0.f;
	//			return;
	//		}
	//	}
	//	else
	//	{
	//		stuckTimer = 0.f;
	//		lastPosition = center;
	//	}
	//}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (currentPathfindingPath.empty() || currentPathfindingIndex >= currentPathfindingPath.size())
	{
		velocity = { 0.f, 0.f };
		return; // No path to follow or index out of bounds
	}

	sf::Vector2f center = getLogicPositionCenter();
	sf::Vector2f target = TileMap::getTileCenter(currentPathfindingPath.at(currentPathfindingIndex));
	float dist = std::hypotf(target.x - center.x, target.y - center.y);

	//constexpr float NODE_REACH_EPSILON = 4.f;
	if (dist <= CHASE_SPEED * fixedTimeStep/* ||
		dist <= TileMap::TILE_SIZE * 0.25f ||
		dist <= NODE_REACH_EPSILON*/)
	{
		currentPathfindingIndex++;
		if (currentPathfindingIndex >= currentPathfindingPath.size())
		{
			velocity = { 0.f, 0.f }; // Reached the end of the path
			return;
		}

		target = TileMap::getTileCenter(currentPathfindingPath.at(currentPathfindingIndex));
	}
	moveTowards(target, fixedTimeStep);
}

void Enemy::smoothPath(const TileMap& tileMap, float fixedTimeStep)
{
	//if (currentPathfindingPath.size() < 2) return;

	//std::vector<sf::Vector2i> smoothed;
	//sf::Vector2i current = currentPathfindingPath.front();
	//smoothed.push_back(current);

	//for (std::size_t i = 1; i < currentPathfindingPath.size(); ++i)
	//{
	//	sf::Vector2f start = TileMap::getTileCenter(current);
	//	sf::Vector2f goal = TileMap::getTileCenter(currentPathfindingPath[i]);

	//	if (!losChecker.check(start, goal, tileMap))//Utility::hasLineOfSightWithClearance(start, goal, tileMap, size.x * 1.f))
	//	{
	//		current = currentPathfindingPath[i - 1];
	//		smoothed.push_back(current);
	//	}
	//}

	//smoothed.push_back(currentPathfindingPath.back());
	//currentPathfindingPath = std::move(smoothed);
	//currentPathfindingIndex = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (currentPathfindingPath.size() <= 2)
		return; // Already as smooth as it gets

	std::vector<sf::Vector2i> smoothed;
	std::size_t startIndex = 0;

	while (startIndex < currentPathfindingPath.size() - 1)
	{
		// Look as far ahead as possible
		std::size_t nextIndex = startIndex + 1;

		while (nextIndex < currentPathfindingPath.size() &&
				!Utility::hasLineOfSight/*WithClearance*/(
					TileMap::getTileCenter(currentPathfindingPath.at(startIndex)),
					TileMap::getTileCenter(currentPathfindingPath.at(nextIndex)),
					tileMap/*,
					size.x * 0.7f)*/))
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

//bool Enemy::tryInsertDetourNode(const TileMap& tileMap)
//{
//	if (currentPathfindingPath.empty() || currentPathfindingIndex >= (int)currentPathfindingPath.size())
//		return false;
//
//	sf::Vector2i currentTarget = currentPathfindingPath[currentPathfindingIndex];
//
//	// Directions to try (left, right, up, down)
//	std::vector<sf::Vector2i> detourOffsets = {
//		sf::Vector2i(-1, 0),
//		sf::Vector2i(1, 0),
//		sf::Vector2i(0, -1),
//		sf::Vector2i(0, 1)
//	};
//
//	for (auto& offset : detourOffsets) {
//		sf::Vector2i detourTile = currentTarget + offset;
//
//		if (!tileMap.isWithinBounds(detourTile))
//			continue;
//
//		if (tileMap.getTile(detourTile).type == Tile::Type::Solid)
//			continue;
//
//		// Avoid inserting the same detour repeatedly
//		if (currentPathfindingIndex + 1 < (int)currentPathfindingPath.size() && currentPathfindingPath[currentPathfindingIndex + 1] == detourTile)
//			continue;
//
//		currentPathfindingPath.insert(currentPathfindingPath.begin() + currentPathfindingIndex + 1, detourTile);
//		return true; // Insert only one detour tile at a time
//	}
//	return false; // No valid detour found
//}
//
//bool Enemy::isStuck() {
//	float distMoved = std::hypotf(currentPosition.x - stuckLastPosition.x, currentPosition.y - stuckLastPosition.y);
//
//	if (distMoved < STUCK_DISTANCE_THRESHOLD) {
//		stuckCounter++;
//	}
//	else {
//		stuckCounter = 0;
//	}
//
//	stuckLastPosition = currentPosition;
//
//	return stuckCounter >= STUCK_THRESHOLD_FRAMES;
//}

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
	sf::Vector2f direction = target - getLogicPositionCenter();
	float distance = std::hypotf(direction.x, direction.y);

	if (distance != 0.f)
	{
		direction /= distance;
		velocity = direction * std::min(isAggroed || isReturningToPatrol ? CHASE_SPEED : PATROL_SPEED, distance / fixedTimeStep);
	}
	else
	{
		velocity = { 0.f, 0.f }; // Already at target
	}
}