// ================================================================================================
// File: Utility.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the Utility namespace, which contains useful utility functions.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>

namespace Utility
{
	bool isKeyReleased(sf::Keyboard::Key key);
	bool isButtonReleased(sf::Mouse::Button button);

	// Generates a random pitch value based on a given variation percentage.
	// Example use: variationPercent 0.15f == 15% variation
	float randomPitch(float variationPercent, float basePitch = 1.f);

	// Interpolates between two vectors based on a given factor (0.0 to 1.0).
	sf::Vector2f interpolate(sf::Vector2f a, sf::Vector2f b, float factor);

	// Normalizes a vector to a unit length.
	sf::Vector2f normalize(sf::Vector2f vector);

	// Converts world pixel position to tile coordinates.
	sf::Vector2i worldToTileCoords(sf::Vector2f worldPos);

	// Converts tile coordinates to world pixel position.
	sf::Vector2f tileToWorldCoords(sf::Vector2i tileCoords);
}