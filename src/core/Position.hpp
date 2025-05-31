// ================================================================================================
// File: Position.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 28, 2025
// Description: Defines the Position class, which is a wrapper for managing the position of game objects.
//              Most notably it provides functionality for smooth rendering by keeping track of both
//              the current and previous positions and providing easy interpolation between them.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/System/Vector2.hpp>

namespace lv
{
	class Position
	{
	public:
		Position() = default;

		// Sync the current position with the previous
		//  (useful when teleporting, spawning, etc.)
		void sync()
		{
			previous = current;
		}
		void set(sf::Vector2f position)
		{
			previous = current;
			current = position;
		}
		void set(float x, float y)
		{
			previous = current;
			current = { x, y };
		}
		void set(const Position& other)
		{
			previous = current;
			current = other.current;
		}
		sf::Vector2f get() const
		{
			return current;
		}
		// Returns the interpolated position based on the
		//  interpolation factor for smooth rendering.
		sf::Vector2f getInterpolated(float interpolationFactor) const
		{
			return current + (current - previous) * interpolationFactor;
		}

	private:
		sf::Vector2f current;
		sf::Vector2f previous;
	};
}