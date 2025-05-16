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

	inline sf::Vector2f getRenderPosition() const { return shape.getPosition(); }
	inline sf::Vector2f getLogicPosition() const { return currentPosition; }

private:
	void applyPhysics(float fixedTimeStep);
	void resolveCollisions(const TileMap& tileMap);

	const float MOVE_SPEED = 200.f;
	const float JUMP_VELOCITY = -400.f;
	const float GRAVITY = 980.f;
	const float MAX_FALL_SPEED = 800.f;

	sf::RectangleShape shape;
	sf::Vector2f currentPosition;
	sf::Vector2f previousPosition;
	sf::Vector2f velocity;
	bool onGround;
};