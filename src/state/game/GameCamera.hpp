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
#include <SFML/Graphics/RectangleShape.hpp>

class Player;

class GameCamera
{
public:
	GameCamera(sf::RenderWindow& window);

	void updateVerticalLook(float fixedTimeStep, bool isLookingUp, bool isLookingDown);
	void preRenderUpdate(float fixedTimeStep, float interpolationFactor, const Player& player);

	const sf::View& getView() const { return view; }
	inline bool contains(sf::Vector2f point) const { return view.getViewport().contains(point); }

	sf::RectangleShape deadZoneShape;

private:
	sf::RenderWindow& window;
	sf::View view;

	const float MAX_VERTICAL_OFFSET = 12.f;
	sf::Vector2f center;
	sf::Vector2f verticalOffset;
};