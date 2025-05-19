// ================================================================================================
// File: GameCamera.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 16, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <algorithm>
#include <cmath>
#include "GameCamera.hpp"
#include "Player.hpp"
#include "../../core/Utility.hpp"

GameCamera::GameCamera(sf::RenderWindow& window) :
	window(window),
	view(window.getDefaultView())
{
	deadZoneShape.setFillColor(sf::Color::Transparent);
	deadZoneShape.setOutlineThickness(1.f);
	deadZoneShape.setOutlineColor(sf::Color::Magenta);
}

void GameCamera::preRenderUpdate(float fixedTimeStep, float interpolationFactor, const Player& player)
{
	auto getSmoothing = [](float distance, float minSmooth, float maxSmooth, float maxDistance) {
		distance = std::min(distance, maxDistance);
		float t = distance / maxDistance; // 0 to 1
		float eased = t < 0.5f ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
		return minSmooth + eased * (maxSmooth - minSmooth);
		};

	deadZoneShape.setSize({ window.getSize().x * 0.25f, window.getSize().y * 0.05f });
	sf::Vector2f playerPos = player.getInterpolatedRenderPosition(interpolationFactor) + player.getSize() / 2.f;
	sf::FloatRect deadZone(center - deadZoneShape.getSize() / 2.f, deadZoneShape.getSize());

	deadZoneShape.setPosition(deadZone.position);

	if (playerPos.x < deadZone.position.x)
	{
		float targetX = playerPos.x + deadZone.size.x / 2.f;
		float distance = deadZone.position.x - playerPos.x;
		float smoothing = getSmoothing(distance, 0.25f, 1.f, 300.f);
		center.x += (targetX - center.x) * smoothing * fixedTimeStep;
	}
	else if (playerPos.x > deadZone.position.x + deadZone.size.x)
	{
		float targetX = playerPos.x - deadZone.size.x / 2.f;
		float distance = playerPos.x - (deadZone.position.x + deadZone.size.x);
		float smoothing = getSmoothing(distance, 0.25f, 1.f, 300.f);
		center.x += (targetX - center.x) * smoothing * fixedTimeStep;
	}
	if (playerPos.y < deadZone.position.y)
	{
		float targetY = playerPos.y + deadZone.size.y / 2.f;
		float distance = deadZone.position.y - playerPos.y;
		float smoothing = getSmoothing(distance, 0.75f, 1.25f, 300.f);
		center.y += (targetY - center.y) * smoothing * fixedTimeStep;
	}
	else if (playerPos.y > deadZone.position.y + deadZone.size.y)
	{
		float targetY = playerPos.y - deadZone.size.y / 2.f;
		float distance = playerPos.y - (deadZone.position.y + deadZone.size.y);
		float smoothing = getSmoothing(distance, 0.75f, 1.25f, 300.f);
		center.y += (targetY - center.y) * smoothing * fixedTimeStep;
	}
	center.y += verticalOffset.y;

	view.setCenter(center);
	view.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
}

void GameCamera::updateVerticalLook(float fixedTimeStep, bool isLookingUp, bool isLookingDown)
{
	float targetY = 0.f;
	if (isLookingUp)
		targetY = -MAX_VERTICAL_OFFSET;
	else if (isLookingDown)
		targetY = MAX_VERTICAL_OFFSET;

	float smoothing = 2.f; // higher = faster response
	verticalOffset.y += (targetY - verticalOffset.y) * smoothing * fixedTimeStep;
	verticalOffset.y = std::clamp(verticalOffset.y, -MAX_VERTICAL_OFFSET, MAX_VERTICAL_OFFSET);
}