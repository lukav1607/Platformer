// ================================================================================================
// File: Player.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 16, 2025
// Description: Defines the Player class, which represents the player character in the game. The Player
//              class is responsible for handling player input, movement, and interactions with the game world.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <algorithm>
#include <iostream>
#include "Player.hpp"
#include "../../core/Utility.hpp"

Player::Player() :
	isOnGround(false),
	currentSpeed(0.f),
	jumpBufferTimer(0.f),
	coyoteTimer(0.f),
	jumpKeyPressed(false),
	jumpKeyHeld(false),
	isCrouching(false),
	color(sf::Color(255, 200, 100)),
	currentSize({ 56.f, HEIGHT_WHEN_STANDING }),
	previousSize({ 56.f, HEIGHT_WHEN_STANDING }),
	m_isLookingDown(false),
	m_isLookingUp(false)
{
	shape.setSize(currentSize);
	shape.setFillColor(color);
	//shape.setOrigin(shape.getSize() / 2.f);
	shape.setPosition({ 100.f, 100.f });
}

void Player::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	direction = { 0.f, 0.f };

	for (const auto& event : events)
		if (const auto& keyPressed = event.getIf<sf::Event::KeyPressed>())
			if (keyPressed->code == sf::Keyboard::Key::Space && !jumpKeyHeld)
 				jumpKeyPressed = true;

	jumpKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		direction.x = -1.f;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		direction.x = 1.f;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
	{
		m_isLookingUp = true;
		m_isLookingDown = false;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
	{
		m_isLookingDown = true;
		m_isLookingUp = false;
	}
	else
	{
		m_isLookingUp = false;
		m_isLookingDown = false;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
		isCrouching = true;
	else
		isCrouching = false;
}

void Player::update(float fixedTimeStep, const TileMap& tileMap)
{
	applyPhysics(fixedTimeStep, tileMap);
	resolveCollisions(fixedTimeStep, tileMap);
}

void Player::render(sf::RenderWindow& window, float interpolationFactor)
{
	shape.setPosition(Utility::interpolate(previousPosition, currentPosition, interpolationFactor));
	shape.setSize(Utility::interpolate(previousSize, currentSize, interpolationFactor));
	window.draw(shape);

	/*sf::RectangleShape futureShape(futureBounds.size);
	futureShape.setPosition(futureBounds.position);
	futureShape.setFillColor(sf::Color::Transparent);
	futureShape.setOutlineThickness(1.f);
	futureShape.setOutlineColor(sf::Color::Magenta);
	window.draw(futureShape);*/
}

void Player::setPosition(sf::Vector2i tileCoords)
{
	currentPosition = { 
		tileCoords.x * TileMap::TILE_SIZE + (TileMap::TILE_SIZE / 2.f - currentSize.x / 2.f),
		tileCoords.y * TileMap::TILE_SIZE + (TileMap::TILE_SIZE - currentSize.y)};
	previousPosition = currentPosition;
	shape.setPosition(currentPosition);
	velocity = { 0.f, 0.f };
	jumpBufferTimer = 0.f;
	coyoteTimer = 0.f;
	isOnGround = false;
	jumpKeyPressed = false;
	jumpKeyHeld = false;
}

sf::Vector2f Player::getInterpolatedRenderPosition(float interpolationFactor) const
{
	return Utility::interpolate(previousPosition, currentPosition, interpolationFactor);
}

void Player::applyPhysics(float fixedTimeStep, const TileMap& tileMap)
{
	static float lastDirection = 0.f;

	previousSize = currentSize;
	if (isOnGround && isCrouching)
	{
		currentSize.y -= 250.f * fixedTimeStep;
		if (currentSize.y < HEIGHT_WHEN_CROUCHING)
			currentSize.y = HEIGHT_WHEN_CROUCHING;
	}
	else if (isOnGround && !isCrouching)
	{
		if (canStandUp(tileMap)) // Only grow if headspace is clear
		{
			currentSize.y += 250.f * fixedTimeStep;
			if (currentSize.y > HEIGHT_WHEN_STANDING)
				currentSize.y = HEIGHT_WHEN_STANDING;
		}
	}

	if (direction.x != 0.f)
	{
		lastDirection = direction.x;

		if ((velocity.x > 0.f && direction.x < 0.f) ||
			(velocity.x < 0.f && direction.x > 0.f))
		{
			currentSpeed -= (DECELERATION + ACCELERATION) * fixedTimeStep;
		}

		currentSpeed += ACCELERATION * fixedTimeStep;
		currentSpeed = std::clamp(currentSpeed, 0.f, isCrouching && isOnGround ? MOVE_SPEED * CROUCHED_SPEED_MULTIPLIER : MOVE_SPEED);
		velocity.x = currentSpeed * direction.x;
	}
	else
	{
		currentSpeed -= DECELERATION * fixedTimeStep;
		currentSpeed = std::max(currentSpeed, 0.f);

		if (currentSpeed > 0.f)
			velocity.x = currentSpeed * lastDirection;
		else
			velocity.x = 0.f;
	}

	if (isOnGround)
		coyoteTimer = COYOTE_TIME;
	else
		coyoteTimer = std::max(0.f, coyoteTimer -= fixedTimeStep); // Clamp to 0
	
	if (jumpKeyPressed)
	{
		jumpBufferTimer = JUMP_BUFFER_TIME;
		jumpKeyPressed = false;
	}
	jumpBufferTimer = std::max(0.f, jumpBufferTimer -= fixedTimeStep); // Clamp to 0

	if (jumpBufferTimer > 0.f && (isOnGround || coyoteTimer > 0.f))
	{
		velocity.y = isCrouching && isOnGround ? JUMP_VELOCITY * CROUCHED_JUMP_MULTIPLIER : JUMP_VELOCITY;
		isOnGround = false;
		coyoteTimer = 0.f;
		jumpBufferTimer = 0.f;
	}		
	
	if (velocity.y < 0.f) // Ascending
	{
		if (!jumpKeyHeld)
			velocity.y += GRAVITY * GRAVITY_JUMP_CUT_MULT * fixedTimeStep;
		else
			velocity.y += GRAVITY * fixedTimeStep;
	}
	else // Falling
	{
		velocity.y = std::max(velocity.y + GRAVITY * GRAVITY_FALL_MULT * fixedTimeStep, MAX_FALL_SPEED);
	}
}

void Player::resolveCollisions(float fixedTimeStep, const TileMap& tileMap)
{
	isOnGround = false;

	sf::Vector2f futurePosition = currentPosition + velocity * fixedTimeStep;
	/*sf::FloatRect */futureBounds = { futurePosition, currentSize };

	sf::Vector2f verticalFuturePos = { currentPosition.x, currentPosition.y + velocity.y * fixedTimeStep };
	sf::FloatRect verticalBounds(verticalFuturePos, currentSize);

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
	sf::FloatRect horizontalBounds(horizontalFuturePos, currentSize);

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

bool Player::canStandUp(const TileMap& tileMap) const
{
	// Check the space *above* the current position up to HEIGHT_WHEN_STANDING
	if (currentSize.y >= HEIGHT_WHEN_STANDING)
		return true; // Already full height

	float heightDifference = HEIGHT_WHEN_STANDING - currentSize.y;

	// Check a box above the player to see if it collides
	sf::FloatRect headCheckBounds(
		sf::Vector2f(currentPosition.x,	currentPosition.y - heightDifference), // space above current head
		sf::Vector2f(currentSize.x, heightDifference)); // height to test

	// Iterate over tiles overlapping that region
	for (int x = (int)(headCheckBounds.position.x) / TileMap::TILE_SIZE - 1;
		x <= (int)(headCheckBounds.position.x + headCheckBounds.size.x) / TileMap::TILE_SIZE + 1;
		++x)
	{
		for (int y = (int)(headCheckBounds.position.y) / TileMap::TILE_SIZE - 1;
			y <= (int)(headCheckBounds.position.y + headCheckBounds.size.y) / TileMap::TILE_SIZE + 1;
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

			auto intersection = headCheckBounds.findIntersection(tileBounds);
			if (intersection.has_value())
				return false; // Blocked from standing up
		}
	}

	return true; // All clear to grow
}