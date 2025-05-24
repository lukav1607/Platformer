// ================================================================================================
// File: Game.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the Game class, which contains the main game loop and manages the game state.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <string>
#include <SFML/Graphics/Font.hpp>
#include "../state/StateManager.hpp"

class Game
{
public:
	Game();
	int run();

	static bool isDebugModeOn() { return m_isDebugModeOn; }

private:
	const std::string PROJECT_NAME = "Platformer";

	void processInput();
	void update(float fixedTimeStep);
	void render(float interpolationFactor);

	static bool m_isDebugModeOn;

	sf::RenderWindow window;
	StateManager stateManager;
	sf::Font font;

	bool isOutOfFocus;
};