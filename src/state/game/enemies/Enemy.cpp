// ================================================================================================
// File: Enemy.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 18, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <SFML/Graphics/Text.hpp>
#include "Enemy.hpp"
#include "../../../core/Time.hpp"
#include "../../../world/Pathfinding.hpp"
#include "../Player.hpp"

using lv::Enemy;

lv::Enemy::Enemy() :
	state(State::Patrolling),
	timeSinceLastPathUpdate(0.f),
	timeSinceGainedLOS(0.f),
	timeSinceLostLOS(0.f),
	health(0),
	patrolSpeed(0.f),
	chaseSpeed(0.f),
	aggroRange(0.f),
	followRange(0.f),
	currentPatrolIndex(0),
	isCompleted(false),
	maxJumpHeight(0.f),
	maxJumpDistance(0.f),
	jumpForce(0.f),
	isOnGround(false),
	size(0.f, 0.f),
	color(sf::Color::White),
	isSelected(false)
{}

void Enemy::serialize(json& j) const
{
	j["type"] = getType();
	j["patrolPositions"] = json::array();
	for (const auto& pos : patrolPositions)
		j["patrolPositions"].push_back({ {"x", pos.x}, {"y", pos.y} });
}

void Enemy::deserialize(const json& j)
{
	patrolPositions.clear();
	if (j.contains("patrolPositions"))
		for (const auto& pos : j["patrolPositions"])
			addPatrolPosition(sf::Vector2i(pos["x"], pos["y"]));

	isCompleted = true;
}

bool lv::Enemy::canSeePlayer(const Player& player) const
{
	return false;
}

void Enemy::addPatrolPosition(sf::Vector2i tilePosition)
{
	patrolPositions.push_back(tilePosition);
	if (patrolPositions.size() == 1)
	{
		setPosition(tilePosition);
	}
}

void Enemy::removePatrolPosition(sf::Vector2i tilePosition)
{
	if (patrolPositions.empty())
		return;

	for (auto it = patrolPositions.begin(); it != patrolPositions.end(); ++it)
	{
		if (*it == tilePosition)
		{ 
			// If the removed index is before or at the current index, adjust
			size_t index = std::distance(patrolPositions.begin(), it);
			if (index <= currentPatrolIndex && currentPatrolIndex > 0)
				currentPatrolIndex--;

			patrolPositions.erase(it);
			break;
		}
	}
	// Wrap index if out of bounds
	if (currentPatrolIndex >= patrolPositions.size() && !patrolPositions.empty())
		currentPatrolIndex = 0;
}

void Enemy::renderPatrolPositions(sf::RenderTarget& target, const sf::Font& font)
{
	if (patrolPositions.empty())
		return;

	sf::Color lineColor = sf::Color(255, 255, 255, 100);
	sf::Color rectColor = color;
	rectColor.a = 100;
	if (!isCompleted || isSelected)
	{
		rectColor.a = Utility::getBreathingAlpha(Time::get(), 25, 100);
		lineColor.a = Utility::getBreathingAlpha(Time::get(), 25, 100);
	}

	// ---- PASS 1: Draw all rectangles and numbers ----
	for (int i = 0; i < patrolPositions.size(); ++i)
	{
		const auto worldPos = Utility::tileToWorldCoords(patrolPositions[i]);

		sf::RectangleShape patrolShape(sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));
		patrolShape.setPosition(worldPos);
		patrolShape.setFillColor(rectColor);
		patrolShape.setOutlineColor(color);
		patrolShape.setOutlineThickness(2.f);
		target.draw(patrolShape);

		sf::Text patrolText(font, std::to_string(i), 20);
		patrolText.setFillColor(sf::Color::White);
		patrolText.setPosition(worldPos + sf::Vector2f(5.f, 5.f));
		target.draw(patrolText);
	}

	auto offset = sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f);

	// ---- PASS 2: Draw all lines ----
	for (int i = 0; i + 1 < patrolPositions.size(); ++i)
	{
		auto current = Utility::tileToWorldCoords(patrolPositions[i]) + offset;
		auto next = Utility::tileToWorldCoords(patrolPositions[i + 1]) + offset;

		///Utility::drawDashedLine(target, current, next, lineColor);
		Utility::drawAnimatedDashedLine(target, current, next, lineColor, Time::get(), 10.f, 10.f, 10.f);
		Utility::drawAnimatedArrowheadAtMidpoint(target, current, next, lineColor, Time::get());
	}

	if (isCompleted && patrolPositions.size() > 1)
	{
		auto end = Utility::tileToWorldCoords(patrolPositions.back()) + offset;
		auto start = Utility::tileToWorldCoords(patrolPositions.front()) + offset;
				
		if (patrolPositions.size() > 2) // Don't draw the final/closing line if there are only two positions, as that line already exists (it was the first line)
			Utility::drawAnimatedDashedLine(target, end, start, lineColor, Time::get(), 10.f, 10.f, 10.f);
			///Utility::drawDashedLine(target, end, start, lineColor);
		Utility::drawAnimatedArrowheadAtMidpoint(target, end, start, lineColor, Time::get());
	}
}

sf::Vector2i Enemy::getCurrentPatrolTarget() const
{
	return patrolPositions.at(currentPatrolIndex);
}

size_t Enemy::getNextPatrolIndex() const
{
	if (currentPatrolIndex + 1 >= patrolPositions.size())
		return 0; // Wrap around to the first position if at the end of the list
	else
		return currentPatrolIndex + 1; // Return the next index in the patrol positions
}

void Enemy::targetNextPatrolPosition()
{
	currentPatrolIndex = getNextPatrolIndex();
}

void Enemy::updateMovement(const TileMap& tileMap, const Player& player, float fixedTimeStep)
{
	timeSinceLastPathUpdate += fixedTimeStep;

	switch (state)
	{
	case State::Chasing:
		handleChasing(tileMap, player, fixedTimeStep);
		break;

	case State::Returning:
		handleReturning(tileMap, fixedTimeStep);
		break;

	case State::Patrolling:
		handlePatrolling(tileMap, player, fixedTimeStep);
		break;
	}
}

void Enemy::handlePatrolling(const TileMap& tileMap, const Player& player, float fixedTimeStep)
{
	using lv::Constants::PATHFINDING_UPDATE_INTERVAL;

	sf::Vector2f navPos = getNavigationPosition();
	sf::Vector2f target = getPathTargetPosition(getCurrentPatrolTarget());

	float distToPatrolTarget = std::hypotf(target.x - navPos.x, target.y - navPos.y);
	float distToPlayer = std::hypotf(player.getLogicPositionCenter().x - navPos.x, player.getLogicPositionCenter().y - navPos.y);

	if (distToPatrolTarget <= patrolSpeed * fixedTimeStep)
	{
		targetNextPatrolPosition();
	}

	if (distToPlayer < aggroRange && Utility::hasLineOfSight(navPos, player.getBounds(), tileMap))
	{
		positionBeforeAggro = navPos;
		d_positionBeforeAggroCircle.setPosition(positionBeforeAggro - sf::Vector2f(d_positionBeforeAggroCircle.getRadius(), d_positionBeforeAggroCircle.getRadius()));
		state = State::Chasing;
		return;
	}
	else
	{
		//if (Utility::hasLineOfSight(center, currentPatrolTargetPixels, tileMap))
		if (Utility::hasLineOfSightWithClearance(navPos, target, size, tileMap))
			timeSinceGainedLOS += fixedTimeStep;
		else
			timeSinceGainedLOS = 0.f;

		if (timeSinceGainedLOS >= LOS_GAINED_THRESHOLD)
		{
			path.clear();
			moveTowards(target, fixedTimeStep);
		}
		else
		{
			if (timeSinceLastPathUpdate >= PATHFINDING_UPDATE_INTERVAL)
			{
				recalculatePath(tileMap, Utility::worldToTileCoords(target));
				timeSinceLastPathUpdate = 0.f;
			}
			followPath(fixedTimeStep);
		}
	}
}

void Enemy::handleChasing(const TileMap& tileMap, const Player& player, float fixedTimeStep)
{
	using lv::Constants::PATHFINDING_UPDATE_INTERVAL;

	sf::Vector2f navPos = getNavigationPosition();
	float distToReturn = std::hypotf(positionBeforeAggro.x - navPos.x, positionBeforeAggro.y - navPos.y);

	//if (Utility::hasLineOfSight(center, playerPosition, tileMap))
	if (Utility::hasLineOfSightWithClearance(navPos, player.getLogicPositionCenter(), size, tileMap))
	{
		timeSinceGainedLOS += fixedTimeStep;
		timeSinceLostLOS = 0.f;
	}
	else
	{
		timeSinceGainedLOS = 0.f;
		timeSinceLostLOS += fixedTimeStep;
	}

	if (distToReturn > followRange || timeSinceLostLOS >= LOS_LOST_THRESHOLD)
	{
		timeSinceLostLOS = 0.f;
		timeSinceGainedLOS = 0.f;
		path.clear();
		state = State::Returning;
		return;
	}

	if (timeSinceGainedLOS >= LOS_GAINED_THRESHOLD)
	{
		path.clear();
		moveTowards(player.getLogicPosition(), fixedTimeStep);
	}
	else
	{
		if (timeSinceLastPathUpdate >= PATHFINDING_UPDATE_INTERVAL)
		{
			recalculatePath(tileMap, Utility::worldToTileCoords(player.getLogicPosition()));
			timeSinceLastPathUpdate = 0.f;
		}
		followPath(fixedTimeStep);
	}
}

void Enemy::handleReturning(const TileMap& tileMap, float fixedTimeStep)
{
	using lv::Constants::PATHFINDING_UPDATE_INTERVAL;

	sf::Vector2f navPos = getNavigationPosition();
	float distToReturn = std::hypotf(positionBeforeAggro.x - navPos.x, positionBeforeAggro.y - navPos.y);

	if (distToReturn <= chaseSpeed * fixedTimeStep)
	{
		timeSinceGainedLOS = 0.f;
		state = State::Patrolling;
		return;
	}

	//if (Utility::hasLineOfSight(center, positionBeforeAggro, tileMap))
	if (Utility::hasLineOfSightWithClearance(navPos, positionBeforeAggro, size / 2.f, tileMap))
		timeSinceGainedLOS += fixedTimeStep;
	else
		timeSinceGainedLOS = 0.f;

	if (timeSinceGainedLOS >= LOS_GAINED_THRESHOLD)
	{
		path.clear();
		moveTowards(positionBeforeAggro, fixedTimeStep);
	}
	else
	{
		if (timeSinceLastPathUpdate >= PATHFINDING_UPDATE_INTERVAL)
		{
			recalculatePath(tileMap, Utility::worldToTileCoords(positionBeforeAggro));
			timeSinceLastPathUpdate = 0.f;
		}
		followPath(fixedTimeStep);
	}
}

void lv::Enemy::resolveCollisions(float fixedTimeStep, const TileMap& tileMap)
{
	isOnGround = false;

	sf::Vector2f futurePosition = position.get() + velocity * fixedTimeStep;
	sf::FloatRect futureBounds = { futurePosition, size };

	sf::Vector2f verticalFuturePos = { position.get().x, position.get().y + velocity.y * fixedTimeStep };
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

	sf::Vector2f horizontalFuturePos = { position.get().x + velocity.x * fixedTimeStep, futurePosition.y };
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
	position.set(futurePosition);
}

void Enemy::setPosition(sf::Vector2i tilePosition)
{
	position.set(getPathTargetPosition(tilePosition) - getNavigationPositionLocal());
	position.sync();
}

void Enemy::recalculatePath(const TileMap& tileMap, sf::Vector2i target)
{
	sf::Vector2i start = getTilePosition();
	sf::Vector2i goal = target;

	if (!path.empty() && goal == path.back())
		return; // Already at the goal

	using namespace Pathfinding;
	path = findPathAStar(tileMap, start, goal);
	currentPathIndex = 0;
}

void lv::Enemy::followPath(float fixedTimeStep)
{
	if (path.empty() || currentPathIndex >= path.size())
	{
		velocity = { 0.f, 0.f };
		return; // No path to follow or index out of bounds
	}

	sf::Vector2f center = getNavigationPosition();
	sf::Vector2f target = getPathTargetPosition(path.at(currentPathIndex));

	float dist = std::hypotf(target.x - center.x, target.y - center.y);

	if (dist <= PATH_TOLERANCE/*chaseSpeed * fixedTimeStep*/)
	{
		currentPathIndex++;
		if (currentPathIndex >= path.size())
		{
			velocity = { 0.f, 0.f };
			return;
		}

		target = getPathTargetPosition(path.at(currentPathIndex));
	}
	moveTowards(target, fixedTimeStep);
}

sf::Vector2f Enemy::getNavigationPosition() const
{
	return position.get() + getNavigationPositionLocal();
}

void Enemy::initializeDebugVisuals()
{
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
}

void Enemy::updateDebugVisuals(const TileMap& tileMap, sf::FloatRect playerBounds)
{
	bool hasLineOfSight = Utility::hasLineOfSight(getEyePosition(), playerBounds, tileMap);
	sf::Color lineColor = hasLineOfSight ? sf::Color(0, 255, 0, 150) : sf::Color(255, 0, 0, 150);
	d_lineOfSightLine = sf::VertexArray(sf::PrimitiveType::Lines, 2);

	std::vector<sf::Vector2f> corners =
	{
		{playerBounds.position.x, playerBounds.position.y},
		{playerBounds.position.x + playerBounds.size.x, playerBounds.position.y},
		{playerBounds.position.x, playerBounds.position.y + playerBounds.size.y},
		{playerBounds.position.x + playerBounds.size.x, playerBounds.position.y + playerBounds.size.y}
	};

	for (const auto& corner : corners)
	{
		//if (Utility::hasLineOfSight(getEyePosition(), corner, tileMap))
		//{
			d_lineOfSightLine.append(sf::Vertex{ getEyePosition(), lineColor });
			d_lineOfSightLine.append(sf::Vertex{ corner, lineColor });
		//}
	}

	d_patrolTargetCircle.setPosition(TileMap::getTileCenter(patrolPositions.at(currentPatrolIndex)) - sf::Vector2f(d_patrolTargetCircle.getRadius(), d_patrolTargetCircle.getRadius()));
	d_aggroRangeCircle.setPosition(position.get() + size / 2.f - sf::Vector2f(aggroRange, aggroRange));
	d_followRangeCircle.setPosition(positionBeforeAggro - sf::Vector2f(followRange, followRange));
}

void lv::Enemy::renderDebugVisuals(sf::RenderTarget& target, const sf::Font& font, float interpolationFactor)
{
	for (int i = 0; i < path.size(); ++i)
	{
		sf::Text nodeText(font, std::to_string(i), 18);
		nodeText.setFillColor(sf::Color::White);
		nodeText.setPosition(
			Utility::tileToWorldCoords(
				path.at(i)) +
			sf::Vector2f(TileMap::TILE_SIZE / 2.f - nodeText.getGlobalBounds().size.x / 2.f,
				TileMap::TILE_SIZE / 2.f - nodeText.getGlobalBounds().size.y / 2.f));
		target.draw(nodeText);
	}

	target.draw(d_lineOfSightLine);
	target.draw(d_patrolTargetCircle);
	target.draw(d_aggroRangeCircle);

	if (state == State::Chasing || state == State::Returning)//isAggroed || isReturningToPatrol)
	{
		target.draw(d_positionBeforeAggroCircle);
		target.draw(d_followRangeCircle);
	}
}