// ================================================================================================
// File: Player.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 16, 2025
// Description: Defines the Player class, which represents the player character in the game. The Player
//              class is responsible for handling player input, movement, and interactions with the game world.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/System/Vector2.hpp>
#include "../../world/TileMap.hpp"

class Player
{
public:
	Player();

	void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events);
	void update(float fixedTimeStep, const TileMap& tileMap);
	void render(sf::RenderWindow& window, float interpolationFactor);

	void setPosition(sf::Vector2i tileCoords);

	inline sf::Vector2f getRenderPosition() const { return shape.getPosition(); }
	sf::Vector2f getInterpolatedRenderPosition(float interpolationFactor) const;
	inline sf::Vector2f getLogicPosition() const { return currentPosition; }
	inline sf::FloatRect getBounds() const { return sf::FloatRect(currentPosition, size); }
	inline sf::Vector2f getSize() const { return size; }
	inline sf::Color getColor() const { return color; }
	inline bool isLookingUp() const { return m_isLookingUp; }
	inline bool isLookingDown() const { return m_isLookingDown; }

private:
	void applyPhysics(float fixedTimeStep);
	void resolveCollisions(float fixedTimeStep, const TileMap& tileMap);

	const float MOVE_SPEED = 200.f;
	const float ACCELERATION = 500.f;
	const float DECELERATION = 1500.f;

	const float JUMP_VELOCITY = -750.f;
	const float GRAVITY = 1500.f;

	const float GRAVITY_JUMP_CUT_MULT = 3.0f; // Applied when player releases jump early
	const float GRAVITY_FALL_MULT = 1.15f;     // Applied when falling normally
	const float MAX_FALL_SPEED = 500.f;

	const float JUMP_BUFFER_TIME = 0.1f;
	const float COYOTE_TIME = 0.075f;

	sf::Vector2f currentPosition;
	sf::Vector2f previousPosition;
	sf::Vector2f velocity;
	sf::Vector2f direction;
	float currentSpeed;
	bool isOnGround;
	float jumpBufferTimer;
	float coyoteTimer;
	bool jumpKeyPressed;
	bool jumpKeyHeld;

	bool m_isLookingUp;
	bool m_isLookingDown;

	sf::RectangleShape shape;
	sf::Color color;
	sf::Vector2f size;
};