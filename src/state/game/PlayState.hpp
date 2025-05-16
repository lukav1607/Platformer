// ================================================================================================
// File: PlayState.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the PlayState class, which represents the main gameplay state of the game.
//              The PlayState is responsible for handling the core game logic, including player
//              movement, enemy AI, and game events. It is the primary state of the game.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include "../State.hpp"
#include "../../world/TileMap.hpp"

class PlayState : public State
{
public:
	PlayState(StateManager& stateManager);

	void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events) override;
	void update(float fixedTimeStep) override;
	void render(sf::RenderWindow& window, float interpolationFactor) override;

	void applyView(sf::RenderWindow& window) override;

private:
	TileMap map;
};