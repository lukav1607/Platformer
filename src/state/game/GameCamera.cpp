// ================================================================================================
// File: GameCamera.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 16, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <cmath>
#include "GameCamera.hpp"
#include "Player.hpp"

GameCamera::GameCamera(sf::RenderWindow& window) :
	window(window),
	view(window.getDefaultView())
{
}

void GameCamera::preRenderUpdate(float fixedTimeStep, float interpolationFactor, const Player& player)
{
	sf::Vector2f targetPosition = player.getInterpolatedRenderPosition(interpolationFactor);

	const float DIR_EPSILON = 0.001f;
	float dx = targetPosition.x - lastPlayerPosition.x;

	static sf::Vector2f currentLookahead = { 0.f, 0.f };
	sf::Vector2f desiredLookahead = { 0.f, 0.f };

	if (dx > DIR_EPSILON)
		desiredLookahead = LOOKAHEAD_OFFSET;
	else if (dx < -DIR_EPSILON)
		desiredLookahead = -LOOKAHEAD_OFFSET;

	currentLookahead += (desiredLookahead - currentLookahead) * (1.f - std::exp(-SMOOTHING_FACTOR * fixedTimeStep));
	
	lastPlayerPosition = targetPosition;

	sf::Vector2f desiredPosition = targetPosition + currentLookahead;
	sf::Vector2f currentPosition = view.getCenter();
	sf::Vector2f positionDifference = desiredPosition - currentPosition;

	view.setCenter(currentPosition + positionDifference * (1.f - std::exp(-SMOOTHING_FACTOR * fixedTimeStep)));
	view.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
}

// ---------------------------------------- //
// -------- VERSION WITH DEAD ZONE -------- //
// ---------------------------------------- //

//sf::Vector2f playerPos = player.getInterpolatedRenderPosition(interpolationFactor);
//
//const float DIR_EPSILON = 0.001f;
//float dx = playerPos.x - lastPlayerPosition.x;
//
//sf::Vector2f desiredLookahead = { 0.f, 0.f };
//sf::Vector2f currentLookahead = { 0.f, 0.f };
//if (dx > DIR_EPSILON)
//desiredLookahead = LOOKAHEAD_OFFSET;
//else if (dx < -DIR_EPSILON)
//	desiredLookahead = -LOOKAHEAD_OFFSET;
//
//currentLookahead += (desiredLookahead - currentLookahead) * (1.f - std::exp(-SMOOTHING_FACTOR * fixedTimeStep));
//lastPlayerPosition = playerPos;
//
//const sf::Vector2f DEAD_ZONE_SIZE = sf::Vector2f(
//	window.getSize().x * 0.4f, window.getSize().y * 0.4f);
//
//sf::FloatRect deadZone(
//	sf::Vector2f(targetCenter.x - DEAD_ZONE_SIZE.x / 2.f,
//		targetCenter.y - DEAD_ZONE_SIZE.y / 2.f),
//	sf::Vector2f(DEAD_ZONE_SIZE.x,
//		DEAD_ZONE_SIZE.y)
//);
//
//bool hasMoved = false;
//
//if (playerPos.x < deadZone.position.x)
//{
//	targetCenter.x = playerPos.x + DEAD_ZONE_SIZE.x / 2.f;
//	hasMoved = true;
//}
//else if (playerPos.x > deadZone.position.x + deadZone.size.x)
//{
//	targetCenter.x = playerPos.x - DEAD_ZONE_SIZE.x / 2.f;
//	hasMoved = true;
//}
//
//if (playerPos.y < deadZone.position.y)
//{
//	targetCenter.y = playerPos.y + DEAD_ZONE_SIZE.y / 2.f;
//	hasMoved = true;
//}
//else if (playerPos.y > deadZone.position.y + deadZone.size.y)
//{
//	targetCenter.y = playerPos.y - DEAD_ZONE_SIZE.y / 2.f;
//	hasMoved = true;
//}
//
//sf::Vector2f finalTarget = hasMoved ? targetCenter + currentLookahead : targetCenter;
//
//sf::Vector2f currentCenter = view.getCenter();
//sf::Vector2f smoothedCenter = currentCenter + (finalTarget - currentCenter) * (1.f - std::exp(-SMOOTHING_FACTOR * fixedTimeStep));
//
//view.setCenter(smoothedCenter);
//view.setSize(static_cast<sf::Vector2f>(window.getSize()));