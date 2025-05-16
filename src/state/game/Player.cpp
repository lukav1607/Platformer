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

#include "Player.hpp"
#include "../../core/Utility.hpp"

Player::Player()
{
	shape.setSize({ 32.f, 48.f });
	shape.setFillColor(sf::Color::Cyan);
	shape.setOrigin(shape.getSize() / 2.f);
	shape.setPosition({ 100.f, 100.f });
}

void Player::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	velocity.x = 0.f;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		velocity.x = -MOVE_SPEED;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		velocity.x = MOVE_SPEED;

	if (onGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
	{
		velocity.y = JUMP_VELOCITY;
		onGround = false;
	}
}

void Player::update(float fixedTimeStep, const TileMap& tileMap)
{
	applyPhysics(fixedTimeStep);
	resolveCollisions(tileMap);
}

void Player::render(sf::RenderWindow& window, float interpolationFactor)
{
	shape.setPosition(Utility::interpolate(previousPosition, currentPosition, interpolationFactor));
	window.draw(shape);
}

void Player::applyPhysics(float fixedTimeStep)
{
	velocity.y += GRAVITY * fixedTimeStep;
	if (velocity.y > MAX_FALL_SPEED)
		velocity.y = MAX_FALL_SPEED;

	previousPosition = currentPosition;
	currentPosition += velocity * fixedTimeStep;
}

void Player::resolveCollisions(const TileMap& tileMap)
{
	// TODO: Implement collision resolution with the tile map

	if (currentPosition.y >= tileMap.getSize().y * TileMap::TILE_SIZE - shape.getSize().y / 2.f)
	{
		currentPosition.y = tileMap.getSize().y * TileMap::TILE_SIZE - shape.getSize().y / 2.f;
		velocity.y = 0.f;
		onGround = true;
	}
}
