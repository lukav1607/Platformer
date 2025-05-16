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

sf::Vector2i Utility::worldToTileCoords(sf::Vector2f worldPos)
{
	return 
	{
		   static_cast<int>(worldPos.x) / TileMap::TILE_SIZE,
		   static_cast<int>(worldPos.y) / TileMap::TILE_SIZE
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
