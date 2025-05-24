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
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Text.hpp>
#include "Enemy.hpp"
#include "../../core/Utility.hpp"

std::vector<sf::Vector2i> Enemy::usedPatrolPositions;
sf::Clock Enemy::clock;

Enemy::Enemy() :
	type(Type::Crawling),
	health(0),
	isAggroed(false),
	isOnGround(false),
	aggroRange(0.f),
	currentTargetTile(0, 0),
	isCompleted(false)
{}

Enemy::Enemy(std::vector<sf::Vector2i> patrolPositions, Type type/*sf::Vector2f size, sf::Color color, int health, bool isPassive*/) :
	type(type),
	isAggroed(false),
	patrolPositions(patrolPositions),
	isOnGround(false),
	isCompleted(true)
{
	switch (type)
	{
	case Type::Crawling:
		size = { 32.f, 16.f };
		health = 2;
		aggroRange = 0;
		break;
	case Type::Walking:
		size = { 30.f, 40.f };
		health = 3;
		aggroRange = 500;
		break;
	case Type::Flying:
		size = { 28.f, 28.f };
		health = 2;
		aggroRange = 600;
		break;
	}
	color = getColor(type);

	//aggroRange = isPassive ? 0.f : 500.f;
	shape.setSize(size);
	shape.setFillColor(color);

	for (const auto& patrolPosition : patrolPositions)
		usedPatrolPositions.push_back(patrolPosition);
	initializePatrolPositions();
}

void Enemy::update(float fixedTimeStep, const TileMap& tileMap)
{
	switch (type)
	{
	case Type::Crawling:
		break;

	case Type::Walking:
		break;

	case Type::Flying:
		/*sf::Vector2f */currentTargetPixels = TileMap::getTileCenter(currentTargetTile) - size / 2.f;
		sf::Vector2f direction = currentTargetPixels - currentPosition;

		float distance = std::hypotf(direction.x, direction.y);
		//float step = MOVE_SPEED * fixedTimeStep;

		if (distance <= MOVE_SPEED * fixedTimeStep)
		{
			//currentPosition = currentTargetPixels;
			currentTargetTile = getNextPatrolTarget();
			//velocity = { 0.f, 0.f };
		}
		else
		{
			/*direction /= distance;
			velocity = direction * step;*/
		}
		if (distance != 0.f)
			direction /= distance;
		velocity = direction * MOVE_SPEED;
		break;
	}
	//previousPosition = currentPosition;
	//currentPosition += velocity * fixedTimeStep;
	resolveCollisions(fixedTimeStep, tileMap);
}

void Enemy::render(sf::RenderWindow& window, float interpolationFactor)
{
	sf::CircleShape target(5.f);
	target.setFillColor(sf::Color::Transparent);
	target.setOutlineColor(sf::Color::Red);
	target.setOutlineThickness(1.f);
	target.setPosition(currentTargetPixels);
	window.draw(target);

	shape.setPosition(Utility::interpolate(previousPosition, currentPosition, interpolationFactor));
	window.draw(shape);
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

//bool Enemy::isAtPatrolTarget() const
//{
//	sf::Vector2f position = Utility::tileToWorldCoords(currentPatrolTarget) + sf::Vector2f(TileMap::TILE_SIZE / 2.f - 2.f, TileMap::TILE_SIZE / 2.f - 2.f);
//	sf::FloatRect bounds(position, { 4.f, 4.f });
//
//	if (bounds.findIntersection(getBounds()).has_value())
//		return true;
//	return false;
//}

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

void Enemy::initializePatrolPositions()
{
	if (this->patrolPositions.empty())
	{
		std::cerr << "Error: Failed to create Enemy - patrol positions cannot be empty." << std::endl;
		return;
	}

	if (this->patrolPositions.size() > 1)
		currentTargetTile = this->patrolPositions.at(1);
	else
		currentTargetTile = this->patrolPositions.at(0);

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
	if (currentTargetTile == patrolPositions.back())
		return patrolPositions.front();
	for (int i = 0; i < patrolPositions.size() - 1; ++i)
	{
		if (currentTargetTile == patrolPositions.at(i))
			return patrolPositions.at(i + 1);
	}
	return currentTargetTile;
}