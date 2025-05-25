// ================================================================================================
// File: Utility.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <map>
#include <random>
#include <cmath>
#include "Utility.hpp"
#include "../world/TileMap.hpp"

bool Utility::isKeyReleased(sf::Keyboard::Key key)
{
	static std::map<sf::Keyboard::Key, bool> keyStates;
	if (keyStates.find(key) == keyStates.end())
		keyStates[key] = false;
	bool isPressedNow = sf::Keyboard::isKeyPressed(key);
	bool wasPressedLastFrame = keyStates[key];
	keyStates[key] = isPressedNow;
	return !isPressedNow && wasPressedLastFrame;
}

bool Utility::isButtonReleased(sf::Mouse::Button button)
{
	static std::map<sf::Mouse::Button, bool> buttonStates;
	if (buttonStates.find(button) == buttonStates.end())
		buttonStates[button] = false;
	bool isPressedNow = sf::Mouse::isButtonPressed(button);
	bool wasPressedLastFrame = buttonStates[button];
	buttonStates[button] = isPressedNow;
	return !isPressedNow && wasPressedLastFrame;
}

float Utility::randomPitch(float variationPercent, float basePitch)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

	float randomFactor = dist(gen);
	return basePitch + randomFactor * 2.0f * variationPercent;
}

std::uint8_t Utility::getBreathingAlpha(float timeSeconds, std::uint8_t minAlpha, std::uint8_t maxAlpha, float cycleDuration)
{
	float halfCycle = cycleDuration / 2.0f;
	float t = std::fmod(timeSeconds, cycleDuration);

	float phaseT = (t < halfCycle)
		? (t / halfCycle)                                  // 0 to 1
		: (1.0f - ((t - halfCycle) / halfCycle));          // 1 to 0

	float eased = -(std::cos(phaseT * 3.14159265f) - 1.f) / 2.f;

	return static_cast<std::uint8_t>(minAlpha + eased * (maxAlpha - minAlpha));
}

sf::Vector2f Utility::interpolate(sf::Vector2f a, sf::Vector2f b, float factor)
{
	return a * (1.f - factor) + b * factor;
}

sf::Vector2f Utility::normalize(sf::Vector2f vector)
{
	float length = std::hypotf(vector.x, vector.y);
	if (length == 0.f)
		return { 0.f, 0.f };
	return vector / length;
}

//sf::Vector2f Utility::edgeNormal(sf::Vector2f p1, sf::Vector2f p2)
//{
//	sf::Vector2f edge = p2 - p1;
//	sf::Vector2f normal(-edge.y, edge.x);
//	float length = std::hypot(normal.x, normal.y);
//	if (length > 0.f)
//		normal /= length;
//	return normal;
//}
//
//void Utility::projectPolygon(std::vector<sf::Vector2f> points, sf::Vector2f axis, float& min, float& max)
//{
//	min = max = (points[0].x * axis.x + points[0].y * axis.y);
//	for (size_t i = 1; i < points.size(); ++i)
//	{
//		float proj = points[i].x * axis.x + points[i].y * axis.y;
//		if (proj < min) min = proj;
//		if (proj > max) max = proj;
//	}
//}
//
//void Utility::projectRect(sf::FloatRect rect, sf::Vector2f axis, float& min, float& max)
//{
//	std::vector<sf::Vector2f> corners = {
//		{rect.position.x, rect.position.y},
//		{rect.position.x + rect.size.x, rect.position.y},
//		{rect.position.x + rect.size.x, rect.position.y + rect.size.y},
//		{rect.position.x, rect.position.y + rect.size.y}
//	};
//
//	min = max = (corners[0].x * axis.x + corners[0].y * axis.y);
//	for (size_t i = 1; i < corners.size(); ++i)
//	{
//		float proj = corners[i].x * axis.x + corners[i].y * axis.y;
//		if (proj < min) min = proj;
//		if (proj > max) max = proj;
//	}
//}
//
//bool Utility::doesRectIntersectPolygon(sf::FloatRect rect, std::vector<sf::Vector2f> polygon)
//{// 1) Polygon edges normals
//	size_t n = polygon.size();
//	for (size_t i = 0; i < n; ++i)
//	{
//		sf::Vector2f p1 = polygon[i];
//		sf::Vector2f p2 = polygon[(i + 1) % n];
//		sf::Vector2f axis = edgeNormal(p1, p2);
//
//		float minPoly, maxPoly, minRect, maxRect;
//		projectPolygon(polygon, axis, minPoly, maxPoly);
//		projectRect(rect, axis, minRect, maxRect);
//
//		if (!doIntervalsOverlap(minPoly, maxPoly, minRect, maxRect))
//			return false; // Separation found
//	}
//
//	// 2) Rectangle axes: (1,0) and (0,1)
//	std::vector<sf::Vector2f> rectAxes = { {1.f, 0.f}, {0.f, 1.f} };
//	for (const auto& axis : rectAxes)
//	{
//		float minPoly, maxPoly, minRect, maxRect;
//		projectPolygon(polygon, axis, minPoly, maxPoly);
//		projectRect(rect, axis, minRect, maxRect);
//
//		if (!doIntervalsOverlap(minPoly, maxPoly, minRect, maxRect))
//			return false; // Separation found
//	}
//
//	return true; // No separation axis found, collision!
//}

bool Utility::hasLineOfSight(sf::Vector2f from, sf::Vector2f to, const TileMap& tileMap)
{
	sf::Vector2f delta = to - from;
	float distance = std::hypotf(delta.x, delta.y);
	if (distance < 1.f)
		return true; // Too close to check, assume line of sight

	sf::Vector2f direction = delta / distance;
	float stepSize = TileMap::TILE_SIZE / 2.f; // Check every half tile
	sf::Vector2f currentPosition = from;

	float traveled = 0.f;
	sf::Vector2i lastTile = worldToTileCoords(from);

	while (traveled < distance)
	{
		currentPosition += direction * stepSize;
		traveled += stepSize;

		sf::Vector2i currentTile = worldToTileCoords(currentPosition);
		if (!tileMap.isWithinBounds(currentTile))
			return false;

		// Solid tile hit
		if (tileMap.getTile(currentTile).type == Tile::Type::Solid)
			return false;

		// Check for diagonal corner clipping
		int dx = currentTile.x - lastTile.x;
		int dy = currentTile.y - lastTile.y;

		if (dx != 0 && dy != 0) // Diagonal movement
		{
			sf::Vector2i side1(lastTile.x + dx, lastTile.y); // horizontal neighbor
			sf::Vector2i side2(lastTile.x, lastTile.y + dy); // vertical neighbor

			if ((tileMap.isWithinBounds(side1) && tileMap.getTile(side1).type == Tile::Type::Solid) &&
				(tileMap.isWithinBounds(side2) && tileMap.getTile(side2).type == Tile::Type::Solid))
			{
				return false; // Block LOS through corner
			}
		}

		lastTile = currentTile;
	}

	return true;
	//sf::Vector2f delta = to - from;
	//float distance = std::hypotf(delta.x, delta.y);
	//if (distance < 1.f)
	//	return true; // Too close to check, assume line of sight

	//sf::Vector2f direction = delta / distance;
	//float stepSize = TileMap::TILE_SIZE / 2.f; // Check every half tile
	//sf::Vector2f currentPosition = from;

	//float traveled = 0.f;
	//while (traveled < distance)
	//{
	//	sf::Vector2i tile = worldToTileCoords(currentPosition);
	//	if (!tileMap.isWithinBounds(tile) || tileMap.getTile(tile).type == Tile::Type::Solid)
	//	{
	//		return false; // Hit a solid tile
	//	}
	//	currentPosition += direction * stepSize;
	//	traveled += stepSize;
	//}
	//return true;
}

//bool Utility::hasLineOfSightWithClearance(sf::Vector2f from, sf::Vector2f to, const TileMap& tileMap, float clearance)
//{
//	sf::Vector2f delta = to - from;
//	float distance = std::hypotf(delta.x, delta.y);
//
//	if (distance < 1.f)
//		return true;
//
//	sf::Vector2f direction = delta / distance;
//	sf::Vector2f perp(-direction.y, direction.x);
//
//	sf::Vector2f p1 = from + perp * clearance;
//	sf::Vector2f p2 = from - perp * clearance;
//	sf::Vector2f p3 = to - perp * clearance;
//	sf::Vector2f p4 = to + perp * clearance;
//
//	std::vector<sf::Vector2f> polygon = { p1, p2, p3, p4 };
//
//	// Bounding box for the polygon
//	float minX = std::min({ p1.x, p2.x, p3.x, p4.x });
//	float maxX = std::max({ p1.x, p2.x, p3.x, p4.x });
//	float minY = std::min({ p1.y, p2.y, p3.y, p4.y });
//	float maxY = std::max({ p1.y, p2.y, p3.y, p4.y });
//
//	int tileMinX = std::max(0, (int)(minX / TileMap::TILE_SIZE));
//	int tileMaxX = std::min(tileMap.getSize().x - 1, (int)(maxX / TileMap::TILE_SIZE));
//	int tileMinY = std::max(0, (int)(minY / TileMap::TILE_SIZE));
//	int tileMaxY = std::min(tileMap.getSize().y - 1, (int)(maxY / TileMap::TILE_SIZE));
//
//	for (int x = tileMinX; x <= tileMaxX; ++x)
//	{
//		for (int y = tileMinY; y <= tileMaxY; ++y)
//		{
//			if (tileMap.getTile({ x, y }).type != Tile::Type::Solid)
//				continue;
//
//			sf::FloatRect tileRect(
//				sf::Vector2f(x * TileMap::TILE_SIZE, y * TileMap::TILE_SIZE),
//				sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));
//
//			if (doesRectIntersectPolygon(tileRect, polygon))
//				return false; // blocked
//		}
//	}
//
//	return true; // clear LoS
//
//	//sf::Vector2f delta = to - from;
//	//float distance = std::hypotf(delta.x, delta.y);
//
//	//if (distance < 1.f)
//	//	return true; // Too close to meaningfully check, assume LOS
//
//	//sf::Vector2f direction = delta / distance;
//	//const float stepSize = std::max(TileMap::TILE_SIZE / 2.f, 1.f); // Avoid 0 step size
//	//const int numSteps = static_cast<int>(distance / stepSize) + 1;
//
//	//const int NUM_CHECKS = 8; // Circular checks around point
//
//	//for (int step = 0; step <= numSteps; ++step)
//	//{
//	//	sf::Vector2f currentPosition = from + direction * (step * stepSize);
//
//	//	for (int i = 0; i < NUM_CHECKS; ++i)
//	//	{
//	//		float angle = static_cast<float>(i) * (2.f * 3.14159265f / NUM_CHECKS);
//	//		sf::Vector2f offset = { std::cos(angle) * clearance, std::sin(angle) * clearance };
//	//		sf::Vector2f samplePoint = currentPosition + offset;
//	//		sf::Vector2i tile = Utility::worldToTileCoords(samplePoint);
//
//	//		if (!tileMap.isWithinBounds(tile) || tileMap.getTile(tile).type == Tile::Type::Solid)
//	//		{
//	//			return false;
//	//		}
//	//	}
//	//}
//	//return true;
//
//	//sf::Vector2f delta = to - from;
//	//float distance = std::hypotf(delta.x, delta.y);
//	//if (distance < 1.f)
//	//	return true; // Too close to check, assume LOS
//
//	//sf::Vector2f direction = delta / distance;
//	//float stepSize = TileMap::TILE_SIZE / 2.f;
//	//sf::Vector2f currentPosition = from;
//
//	//float traveled = 0.f;
//	//while (traveled < distance)
//	//{
//	//	// Check a circle of points around currentPosition to simulate clearance
//	//	const int CHECK_STEPS = 8;
//	//	for (int i = 0; i < CHECK_STEPS; ++i)
//	//	{
//	//		float angle = i * (2 * 3.14159265f / CHECK_STEPS);
//	//		sf::Vector2f offset = { std::cos(angle) * clearance, std::sin(angle) * clearance };
//	//		sf::Vector2f samplePoint = currentPosition + offset;
//	//		sf::Vector2i tile = Utility::worldToTileCoords(samplePoint);
//
//	//		if (!tileMap.isWithinBounds(tile) || tileMap.getTile(tile).type == Tile::Type::Solid)
//	//		{
//	//			return false;
//	//		}
//	//	}
//
//	//	currentPosition += direction * stepSize;
//	//	traveled += stepSize;
//	//}
//
//	//return true;
//}

sf::Vector2i Utility::worldToTileCoords(sf::Vector2f worldPos)
{
	return 
	{
		   static_cast<int>(worldPos.x / TileMap::TILE_SIZE),
		   static_cast<int>(worldPos.y / TileMap::TILE_SIZE)
	};
}

sf::Vector2f Utility::tileToWorldCoords(sf::Vector2i tileCoords)
{
	return
	{
		static_cast<float>(tileCoords.x) * TileMap::TILE_SIZE,
		static_cast<float>(tileCoords.y) * TileMap::TILE_SIZE
	};
}
