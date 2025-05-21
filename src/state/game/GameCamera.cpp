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
	view(window.getDefaultView()),
	verticalOffset(0.f)
{
	deadZone.size = sf::Vector2f(window.getSize().x * 0.25f, window.getSize().y * 0.05f);
}

void GameCamera::update(float fixedTimeStep, const Player& player)
{
	previousCenter = currentCenter;
	sf::Vector2f playerPosition = player.getLogicPosition() + player.getSize() / 2.f;

	applyAxisSmoothing(currentCenter.x, playerPosition.x, deadZone.position.x, deadZone.position.x + deadZone.size.x, deadZone.size.x / 2.f, fixedTimeStep);
	applyAxisSmoothing(currentCenter.y, playerPosition.y, deadZone.position.y, deadZone.position.y + deadZone.size.y, deadZone.size.y / 2.f, fixedTimeStep);

	updateVerticalLook(fixedTimeStep, player.isLookingUp(), player.isLookingDown());
}


void GameCamera::preRenderUpdate(float interpolationFactor)
{
	view.setCenter(Utility::interpolate(previousCenter, currentCenter, interpolationFactor));
	view.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
	deadZone.position = { view.getCenter() - deadZone.size / 2.f };
}

void GameCamera::applyAxisSmoothing(float& currentCenter, float playerPosition, float deadZoneStart, float deadZoneEnd, float deadZoneCenterOffset, float fixedTimeStep)
{
	auto getSmoothing = [](float distance, float minSmooth, float maxSmooth, float maxDistance) {
		distance = std::min(distance, maxDistance);
		float t = distance / maxDistance; // 0 to 1
		float eased = t < 0.5f ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
		return minSmooth + eased * (maxSmooth - minSmooth);
		};

	if (playerPosition < deadZoneStart)
	{
		float target = playerPosition + deadZoneCenterOffset;
		float distance = deadZoneStart - playerPosition;
		float smoothing = getSmoothing(distance, 0.05f, 0.1f, 300.f);
		float t = 1.f - std::pow(1.f - smoothing, fixedTimeStep * 60.f);
		currentCenter += (target - currentCenter) * t;
	}
	else if (playerPosition > deadZoneEnd)
	{
		float target = playerPosition - deadZoneCenterOffset;
		float distance = playerPosition - deadZoneEnd;
		float smoothing = getSmoothing(distance, 0.05f, 0.1f, 300.f);
		float t = 1.f - std::pow(1.f - smoothing, fixedTimeStep * 60.f);
		currentCenter += (target - currentCenter) * t;
	}
}

void GameCamera::updateVerticalLook(float fixedTimeStep, bool isLookingUp, bool isLookingDown)
{
	float verticalOffsetTarget = 0.f;

	if (isLookingUp)
		verticalOffsetTarget = -MAX_VERTICAL_OFFSET;
	else if (isLookingDown)
		verticalOffsetTarget = MAX_VERTICAL_OFFSET;
	else
		verticalOffsetTarget = 0.f;

		verticalOffset += (verticalOffsetTarget - verticalOffset) * (1.f - std::pow(0.001f, fixedTimeStep * VERTICAL_LOOK_SPEED));
		currentCenter.y += verticalOffset;
}