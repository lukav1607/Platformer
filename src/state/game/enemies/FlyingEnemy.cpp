// ================================================================================================
// File: FlyingEnemy.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 28, 2025
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

#include "FlyingEnemy.hpp"
#include "../../../core/Game.hpp"
#include "../../../world/Pathfinding.hpp"
#include "../Player.hpp"

using lv::Enemy;
using lv::FlyingEnemy;

FlyingEnemy::FlyingEnemy() :
	Enemy()
{
	health = 2;
	size = { 40.f, 40.f };
	color = sf::Color(180, 140, 220);
	patrolSpeed = 75.f; // TODO: Change to tiles per second?
	chaseSpeed = 125.f; // TODO: Change to tiles per second?
	aggroRange = 8 * TileMap::TILE_SIZE;
	followRange = 12 * TileMap::TILE_SIZE;

	initializeDebugVisuals();

	bounds.setFillColor(color);
	bounds.setSize(size);
}
std::unique_ptr<Enemy> FlyingEnemy::clone() const
{
	return std::make_unique<FlyingEnemy>(*this);
}

void FlyingEnemy::serialize(json& j) const
{
	Enemy::serialize(j);
}

void FlyingEnemy::deserialize(const json& j)
{
	Enemy::deserialize(j);
}

void FlyingEnemy::update(float fixedTimeStep, const TileMap& tileMap, const Player& player)
{
	updateMovement(tileMap, player, fixedTimeStep);
	resolveCollisions(fixedTimeStep, tileMap);

	if (Game::getInstance().isDebugModeOn())
		updateDebugVisuals(tileMap, player.getBounds());
}

void FlyingEnemy::render(sf::RenderTarget& target, const sf::Font& font, float interpolationFactor)
{
	if (Game::getInstance().isDebugModeOn() && isSelected)
	{
		renderPatrolPositions(target, font);
	}

	bounds.setPosition(position.getInterpolated(interpolationFactor));
	target.draw(bounds);

	if (Game::getInstance().isDebugModeOn() && isSelected)
	{
		renderDebugVisuals(target, font, interpolationFactor);
	}
}

sf::Vector2f lv::FlyingEnemy::getEyePosition() const
{
	return getNavigationPosition();
}

bool FlyingEnemy::isValidPatrolPosition(const TileMap& tileMap, sf::Vector2i tilePosition) const
{
	//if (!patrolPositions.empty() && tilePosition == patrolPositions.at(0))
	//	return true;

	for (auto& patrolPos : patrolPositions)
		if (patrolPos == tilePosition)
			return false;

	/*for (auto& usedPatrolPos : usedPatrolPositions)
		if (usedPatrolPos == tileCoords)
			return false;*/

	if (!tileMap.isWithinBounds(tilePosition))
		return false;

	if (tileMap.isSolid(tilePosition))
		return false;

	return true;
}

void FlyingEnemy::moveTowards(sf::Vector2f target, float fixedTimeStep)
{
	sf::Vector2f direction = target - getNavigationPosition();
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

sf::Vector2f lv::FlyingEnemy::getNavigationPositionLocal() const
{
	return size / 2.f;
}

sf::Vector2f lv::FlyingEnemy::getPathTargetPosition(sf::Vector2i targetTile) const
{
	return Utility::tileToWorldCoords(targetTile) + sf::Vector2f(TileMap::TILE_SIZE / 2.f, TileMap::TILE_SIZE / 2.f);
}
