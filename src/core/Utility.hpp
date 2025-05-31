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
#include "../world/TileMap.hpp"

namespace Utility
{
	// Custom hash function for sf::Vector2i
	struct Vector2iHasher
	{
		std::size_t operator()(const sf::Vector2i& v) const
		{
			std::hash<int> hasher;
			std::size_t seed = 0;
			seed ^= hasher(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hasher(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};

	bool isKeyReleased(sf::Keyboard::Key key);
	bool isButtonReleased(sf::Mouse::Button button);

	// Generates a random pitch value based on a given variation percentage.
	// Example use: variationPercent 0.15f == 15% variation
	float randomPitch(float variationPercent, float basePitch = 1.f);

	sf::Vector2f getMidpoint(sf::Vector2f from, sf::Vector2f to, float backOffset = 5.f);	
	void drawArrowhead(sf::RenderTarget& target, sf::Vector2f base, sf::Vector2f tip, sf::Color color, float size = 10.f);
	void drawArrowheadAtMidpoint(sf::RenderTarget& target, sf::Vector2f from, sf::Vector2f to, sf::Color color, float size = 10.f);
	void drawAnimatedArrowhead(sf::RenderTarget& target, sf::Vector2f base, sf::Vector2f tip, sf::Color color, float time, float offset = 2.f, float size = 10.f);
	void drawAnimatedArrowheadAtMidpoint(sf::RenderTarget& target, sf::Vector2f from, sf::Vector2f to, sf::Color color, float time, float offset = 2.f, float size = 10.f);

	void drawDashedLine(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end, sf::Color color, float dashLength = 10.f, float gapLength = 10.f);
	void drawAnimatedDashedLine(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end, sf::Color color, float time, float speed = 10.f, float dashLength = 10.f, float gapLength = 10.f);

	// Calculates a breathing alpha value based on the time elapsed.
	std::uint8_t getBreathingAlpha(float timeSeconds, std::uint8_t minAlpha, std::uint8_t maxAlpha, float cycleDuration = 2.5f);

	// Interpolates between two vectors based on a given factor (0.0 to 1.0).
	sf::Vector2f interpolate(sf::Vector2f a, sf::Vector2f b, float factor);

	// Normalizes a vector to a unit length.
	sf::Vector2f normalize(sf::Vector2f vector);

	// Compute normal vector (perpendicular) of edge from p1 to p2
	//sf::Vector2f edgeNormal(sf::Vector2f p1, sf::Vector2f p2);

	// Project polygon points onto an axis and get min/max
	//void projectPolygon(std::vector<sf::Vector2f> points, sf::Vector2f axis, float& min, float& max);

	// Project rectangle corners onto axis and get min/max
	//void projectRect(sf::FloatRect rect, sf::Vector2f axis, float& min, float& max);

	// Check if two projection intervals overlap
	//inline bool doIntervalsOverlap(float minA, float maxA, float minB, float maxB)
	//{
	//	return !(maxA < minB || maxB < minA);
	//}

	// The actual polygon (convex quad) vs axis-aligned rect test
	//bool doesRectIntersectPolygon(sf::FloatRect rect, std::vector<sf::Vector2f> polygon);

	// Check if there is a line of sight between two points, considering tile collisions.
	bool hasLineOfSight(sf::Vector2f from, sf::Vector2f to, const TileMap& tileMap);

	// Check if there is a line of sight from a point to any corner of a rectangle.
	bool hasLineOfSight(sf::Vector2f from, sf::FloatRect to, const TileMap& tileMap);

	// Check if there is a line of sight between two points, considering tile collisions and a clearance size.
	bool hasLineOfSightWithClearance(sf::Vector2f from, sf::Vector2f to, sf::Vector2f size, const TileMap& tileMap);

	// Converts world pixel position to tile coordinates.
	sf::Vector2i worldToTileCoords(sf::Vector2f worldPos);

	// Converts tile coordinates to world pixel position.
	sf::Vector2f tileToWorldCoords(sf::Vector2i tileCoords);

	// Check if there is a line of sight between two points, considering tile collisions and a clearance distance.
	//bool hasLineOfSightWithClearance(sf::Vector2f from, sf::Vector2f to, const TileMap& tileMap, float clearance);

	//class LoSWithHysteresis
	//{
	//public:
	//	LoSWithHysteresis() = default;
	//	LoSWithHysteresis(float clearance, float hysteresis)
	//		: clearanceThreshold(clearance), hysteresisMargin(hysteresis) {
	//	}

	//	bool check(sf::Vector2f from, sf::Vector2f to, const TileMap& tileMap)
	//	{
	//		// Run your existing LOS with clearance check
	//		bool currentLoS = Utility::hasLineOfSightWithClearance(from, to, tileMap, clearanceThreshold);

	//		// If currently true, we allow switching false only if definitely blocked with margin
	//		if (lastLoS)
	//		{
	//			// To turn false, do a stricter check:
	//			// For example, run a version of hasLineOfSightWithClearance with clearance + hysteresisMargin
	//			bool stricterCheck = Utility::hasLineOfSightWithClearance(from, to, tileMap, clearanceThreshold + hysteresisMargin);

	//			if (!stricterCheck)
	//			{
	//				lastLoS = false;
	//			}
	//			// else keep lastLoS = true (don't switch off LOS too quickly)
	//		}
	//		else
	//		{
	//			// If last was false, only switch true if the check passes with a tighter clearance - margin
	//			bool looserCheck = Utility::hasLineOfSightWithClearance(from, to, tileMap, std::max(0.f, clearanceThreshold - hysteresisMargin));

	//			if (looserCheck)
	//			{
	//				lastLoS = true;
	//			}
	//			// else keep lastLoS = false (don't switch on LOS too quickly)
	//		}

	//		return lastLoS;
	//	}

	//	float clearanceThreshold = 0.f;      // your clearance radius
	//	float hysteresisMargin = 2.f;        // epsilon margin to avoid flicker

	//private:
	//	bool lastLoS = false; // store previous LOS state
	//};
}