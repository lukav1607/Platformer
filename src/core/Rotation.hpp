// ================================================================================================
// File: Rotation.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 28, 2025
// Description: Defines the Rotation class, which is a wrapper for managing the angle of game objects.
//              Most notably it provides functionality for smooth rendering by keeping track of both
//              the current and previous angle and providing easy interpolation between them.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/System/Angle.hpp>

namespace lv
{
	class Rotation
	{
	public:
		Rotation() = default;

		// Sync the current angle with the previous
		//  (useful when teleporting, spawning, etc.)
		void sync()
		{
			previous = current;
		}
		void set(sf::Angle angle)
		{
			previous = current;
			current = angle;
		}
		void set(float radians)
		{
			previous = current;
			current = sf::radians(radians);
		}
		void set(const Rotation& other)
		{
			previous = current;
			current = other.current;
		}
		sf::Angle get() const
		{
			return current;
		}
		// Returns the interpolated angle based on the
		//  interpolation factor for smooth rendering.
		sf::Angle getInterpolated(float interpolationFactor) const
		{
			return current + (current - previous) * interpolationFactor;
		}

	private:
		sf::Angle current;
		sf::Angle previous;
	};
}