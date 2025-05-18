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
#include "Enemy.hpp"
#include "../../core/Utility.hpp"

Enemy::Enemy(std::vector<sf::Vector2i> patrolPositions, sf::Vector2f size, sf::Color color, int health, bool isPassive) :
	health(health),
	isAggroed(false),
	patrolPositions(std::move(patrolPositions)),
	hasJustChangedPatrolTarget(true),
	isOnGround(false),
	size(size),
	color(color)
{
	aggroRange = isPassive ? 0.f : 500.f;

	if (this->patrolPositions.empty())
	{
		std::cerr << "Error: Failed to create Enemy - patrol positions cannot be empty." << std::endl;
		return;
	}

	if (this->patrolPositions.size() > 1)
		currentPatrolTarget = this->patrolPositions.at(1);
	else
		currentPatrolTarget = this->patrolPositions.at(0);

	currentPosition = Utility::tileToWorldCoords(this->patrolPositions.at(0));
	previousPosition = currentPosition;

	shape.setSize(size);
	shape.setFillColor(color);
}

void Enemy::update(float fixedTimeStep, const TileMap& tileMap)
{
	if (patrolPositions.size() > 1)
	{
		if (Utility::worldToTileCoords(currentPosition) == currentPatrolTarget)
		{
			currentPatrolTarget = getNextPatrolTarget();
			direction = Utility::normalize(Utility::tileToWorldCoords(currentPatrolTarget) - currentPosition);
		}
	}
	velocity.x = direction.x * MOVE_SPEED * fixedTimeStep;
	velocity.y = std::max(velocity.y + GRAVITY * fixedTimeStep, MAX_FALL_SPEED);

	resolveCollisions(fixedTimeStep, tileMap);
}

void Enemy::render(sf::RenderWindow& window, float interpolationFactor)
{
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

	sf::Vector2f horizontalFuturePos = { futurePosition.x + velocity.x * fixedTimeStep, futurePosition.y };
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

sf::Vector2i Enemy::getNextPatrolTarget() const
{
	if (currentPatrolTarget == patrolPositions.back())
		return patrolPositions.front();
	for (int i = 0; i < patrolPositions.size() - 1; ++i)
	{
		if (currentPatrolTarget == patrolPositions.at(i))
			return patrolPositions.at(i + 1);
	}
	return currentPatrolTarget;
}
