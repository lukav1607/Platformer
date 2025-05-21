// ================================================================================================
// File: EditorCamera.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 15, 2025
// Description: Defines the EditorCamera struct, which represents the camera used in the editor state.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class EditorCamera
{
public:
	void handleInput(sf::Vector2f mouseWorldPosition);
	void update(float fixedTimeStep, float mouseWheelDelta, sf::Vector2f mouseWorldPosition);
	void applyInterpolatedPosition(float interpolationFactor);

	void resize(sf::Vector2u windowSize);

	const sf::View& getView() const { return view; }

private:
	void handleMovingInput();
	void handlePanningInput(sf::Vector2f mouseWorldPosition);

	sf::View view;

	static constexpr float MOVE_SPEED = 500.f;
	sf::Vector2f direction;
	sf::Vector2f currentCenter;
	sf::Vector2f previousCenter;

	static constexpr float PAN_SPEED = 20.f;
	sf::Vector2f anchorPoint;
	bool isPanning = false;

	static constexpr float ZOOM_SPEED = 5.f;
	static constexpr float ZOOM_MIN = 0.5f;
	static constexpr float ZOOM_MAX = 4.f;
	float zoomLevel = 1.f;
};