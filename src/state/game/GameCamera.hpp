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

	void update(float fixedTimeStep, const Player& player);
	void preRenderUpdate(float interpolationFactor);

	const sf::View& getView() const { return view; }
	inline bool contains(sf::Vector2f point) const { return view.getViewport().contains(point); }

private:
	void applyAxisSmoothing(float& currentCenter, float playerPosition, float deadZoneLeft, float deadZoneRight, float deadZoneCenterOffset, float fixedTimeStep);
	void updateVerticalLook(float fixedTimeStep, bool isLookingUp, bool isLookingDown);

	sf::RenderWindow& window;
	sf::View view;

	const float MAX_VERTICAL_OFFSET = 80.f;
	const float VERTICAL_LOOK_SPEED = 1.f;
	float verticalOffset;
	sf::Vector2f currentCenter;
	sf::Vector2f previousCenter;
	sf::FloatRect deadZone;
};