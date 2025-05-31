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
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
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

sf::Vector2f Utility::getMidpoint(sf::Vector2f from, sf::Vector2f to, float backOffset)
{
	sf::Vector2f direction = to - from;
	float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length == 0.f)
		return from;

	direction /= length;
	return from + direction * (length * 0.5f - backOffset);
}

void Utility::drawArrowhead(sf::RenderTarget& target, sf::Vector2f base, sf::Vector2f tip, sf::Color color, float size)
{
	sf::Vector2f direction = tip - base;
	float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length == 0) return;

	direction /= length;

	sf::Vector2f normal(-direction.y, direction.x);

	sf::ConvexShape arrow;
	arrow.setPointCount(3);
	arrow.setPoint(0, tip);
	arrow.setPoint(1, tip - direction * size + normal * size * 0.5f);
	arrow.setPoint(2, tip - direction * size - normal * size * 0.5f);
	arrow.setFillColor(color);
	target.draw(arrow);
}

void Utility::drawArrowheadAtMidpoint(sf::RenderTarget& target, sf::Vector2f from, sf::Vector2f to, sf::Color color, float size)
{
	//sf::Vector2f direction = to - from;
	//float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	//if (length == 0.f)
	//	return;

	//direction /= length; // normalize

	//// Compute midpoint
	//sf::Vector2f mid = from + direction * (length / 2.f);

	sf::Vector2f base = getMidpoint(from, to);
	sf::Vector2f tip = base + (to - from) * 0.01f; // tiny forward nudge

	// Now draw the arrow pointing in the direction from->to
	drawArrowhead(target, base, tip, color, size);
}

void Utility::drawAnimatedArrowhead(sf::RenderTarget& target, sf::Vector2f base, sf::Vector2f tip, sf::Color color, float time, float offset, float size)
{
	sf::Vector2f direction = tip - base;
	float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length == 0.f)
		return;

	direction /= length;

	// Bob amount sliding along the line direction (back and forth)
	float bob = std::sin(time * 3.f + offset) * 2.f;

	// Move both base and tip along the direction by bob amount
	base += direction * bob;
	tip += direction * bob;

	drawArrowhead(target, base, tip, color, size);
}

void Utility::drawAnimatedArrowheadAtMidpoint(sf::RenderTarget& target, sf::Vector2f from, sf::Vector2f to, sf::Color color, float time, float offset, float size)
{
	//sf::Vector2f direction = to - from;
	//float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	//if (length == 0.f)
	//	return;

	//direction /= length; // normalize

	//// Compute midpoint
	//sf::Vector2f mid = from + direction * (length / 2.f);

	sf::Vector2f base = getMidpoint(from, to);
	sf::Vector2f tip = base + (to - from) * 0.01f; // tiny forward nudge

	// Now draw the arrow pointing in the direction from->to
	drawAnimatedArrowhead(target, base, tip, color, time, offset, size);
}

void Utility::drawDashedLine(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end, sf::Color color, float dashLength, float gapLength)
{
	sf::Vector2f delta = end - start;
	float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	sf::Vector2f direction = delta / length;

	float total = 0.f;
	while (total + dashLength < length)
	{
		sf::Vector2f p1 = start + direction * total;
		sf::Vector2f p2 = start + direction * (total + dashLength);

		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line.append(sf::Vertex{ p1, color });
		line.append(sf::Vertex{ p2, color });

		target.draw(line);

		total += dashLength + gapLength;
	}
}

void Utility::drawAnimatedDashedLine(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end, sf::Color color, float time, float speed, float dashLength, float gapLength)
{
	sf::Vector2f delta = end - start;
	float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	sf::Vector2f direction = delta / length;

	float total = std::fmod(time * speed, dashLength + gapLength); // Animate offset

	while (total < length)
	{
		float segLength = std::min(dashLength, length - total);
		sf::Vector2f p1 = start + direction * total;
		sf::Vector2f p2 = start + direction * (total + segLength);

		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line.append(sf::Vertex{ p1, color });
		line.append(sf::Vertex{ p2, color });

		target.draw(line);

		total += dashLength + gapLength;
	}
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
}

bool Utility::hasLineOfSight(sf::Vector2f from, sf::FloatRect to, const TileMap& tileMap)
{
	std::vector<sf::Vector2f> corners =
	{
		{to.position.x, to.position.y},
		{to.position.x + to.size.x, to.position.y},
		{to.position.x, to.position.y + to.size.y},
		{to.position.x + to.size.x, to.position.y + to.size.y}
	};

	for (const auto& corner : corners)
		if (hasLineOfSight(from, corner, tileMap))
			return true; // If at least one corner can be seen, there is line of sight

	return false;
}

bool Utility::hasLineOfSightWithClearance(sf::Vector2f from, sf::Vector2f to, sf::Vector2f size, const TileMap& tileMap)
{
	sf::Vector2f delta = to - from;
	float distance = std::hypotf(delta.x, delta.y);
	if (distance < 1.f)
		return true; // Close enough — assume no obstacles

	sf::Vector2f direction = delta / distance;
	float stepSize = TileMap::TILE_SIZE / 3.f;
	float traveled = 0.f;

	while (traveled < distance)
	{
		sf::Vector2f currentPosition = from + direction * traveled;
		sf::FloatRect hitbox(currentPosition - size / 2.f, size);

		// Convert hitbox bounds to tile range
		int left = static_cast<int>(std::floor(hitbox.position.x / TileMap::TILE_SIZE));
		int top = static_cast<int>(std::floor(hitbox.position.y / TileMap::TILE_SIZE));
		int right = static_cast<int>(std::floor((hitbox.position.x + hitbox.size.x) / TileMap::TILE_SIZE));
		int bottom = static_cast<int>(std::floor((hitbox.position.y + hitbox.size.y) / TileMap::TILE_SIZE));

		for (int y = top; y <= bottom; ++y)
		{
			for (int x = left; x <= right; ++x)
			{
				sf::Vector2i tilePos(x, y);
				if (!tileMap.isWithinBounds(tilePos))
					return false;

				if (tileMap.getTile(tilePos).type == Tile::Type::Solid)
					return false;
			}
		}

		traveled += stepSize;
	}

	return true;
}

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
