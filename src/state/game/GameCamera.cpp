// ================================================================================================
// File: GameCamera.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 16, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include "GameCamera.hpp"
#include "Player.hpp"

GameCamera::GameCamera(sf::RenderWindow& window) :
	window(window),
	view(window.getDefaultView())
{
}

void GameCamera::update(float fixedTimeStep, const Player& player)
{
	sf::Vector2f targetPosition = player.getLogicPosition();

	sf::Vector2f lookahead = { 0.f, 0.f };
	if (targetPosition.x > lastPlayerPosition.x)
		lookahead = LOOKAHEAD_OFFSET;
	else if (targetPosition.x < lastPlayerPosition.x)
		lookahead = -LOOKAHEAD_OFFSET;

	lastPlayerPosition = targetPosition;

	sf::Vector2f desired = targetPosition + lookahead;
	sf::Vector2f current = view.getCenter();
	sf::Vector2f difference = desired - current;

	view.setCenter(current + difference * (1.f - std::exp(-SMOOTHING * fixedTimeStep)));
	window.setView(view);
}
