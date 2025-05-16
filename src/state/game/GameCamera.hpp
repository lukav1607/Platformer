// ================================================================================================
// File: GameCamera.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 16, 2025
// Description: Defines the GameCamera class, which represents the camera that follows the player ingame.
//              The GameCamera class is responsible for handling smooth camera movement.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class Player;

class GameCamera
{
public:
	GameCamera(sf::RenderWindow& window);

	void update(float fixedTimeStep, const Player& player);

	const sf::View& getView() const { return view; }

private:
	sf::RenderWindow& window;
	sf::View view;

	const float SMOOTHING = 5.f;
	const sf::Vector2f LOOKAHEAD_OFFSET = { 96.f, 0.f };
	sf::Vector2f lastPlayerPosition;
	sf::Vector2f velocity;
};