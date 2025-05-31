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
	static Game& getInstance()
	{
		static Game instance;
		return instance;
	}

	void restartGlobalClock()
	{
		deltaTime = globalClock.restart().asSeconds();
		totalTime += deltaTime;
	}

	float getTime() const { return totalTime; }
	float getDeltaTime() const { return deltaTime; }

	int run();

	void toggleDebugMode() { m_isDebugModeOn = !m_isDebugModeOn; }
	void setDebugMode(bool enabled) { m_isDebugModeOn = enabled; }
	bool isDebugModeOn() const { return m_isDebugModeOn; }

private:
	Game();
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;

	void processInput();
	void update(float fixedTimeStep);
	void render(float interpolationFactor);

	const std::string PROJECT_NAME = "Platformer";

	bool m_isDebugModeOn;

	sf::RenderWindow window;
	StateManager stateManager;
	sf::Font font;

	sf::Clock globalClock;
	float totalTime = 0.f;
	float deltaTime = 0.f;

	bool isOutOfFocus;
};