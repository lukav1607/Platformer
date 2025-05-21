// ================================================================================================
// File: Game.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <algorithm>
#include <SFML/Graphics.hpp>
#include "Game.hpp"
#include "../state/game/PlayState.hpp"

Game::Game() :
	isOutOfFocus(false),
	font("assets/fonts/consola.ttf")
{
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 8;
	window.create(sf::VideoMode(sf::Vector2u(1200, 800)), PROJECT_NAME, sf::Style::Default, sf::State::Windowed, settings);
	window.setVerticalSyncEnabled(true);
	//window.setFramerateLimit(60U);

	stateManager.push(std::make_unique<PlayState>(stateManager, window, font));
}

int Game::run()
{
	const float FIXED_TIME_STEP = 1.f / 20.f; // Fixed time step per update
	sf::Clock clock;						  // Clock to measure time
	float timeSinceLastUpdate = 0.f;		  // Time accumulator for fixed timestep
	float interpolationFactor = 0.f;		  // Interpolation factor for rendering
	float lastFrameTime = 0.f;				  // Time of the last frame

	while (window.isOpen())
	{
		lastFrameTime = clock.restart().asSeconds();
		timeSinceLastUpdate += lastFrameTime;
		/*if (lastFrameTime > 0.01f)
			lastFrameTime = 0.01f;*/

		processInput();

		while (timeSinceLastUpdate >= FIXED_TIME_STEP)
		{
			update(FIXED_TIME_STEP);
			timeSinceLastUpdate -= FIXED_TIME_STEP;
		}

		interpolationFactor = std::clamp(timeSinceLastUpdate / FIXED_TIME_STEP, 0.f, 1.f);
		render(interpolationFactor);
	}
	return 0;
}

void Game::processInput()
{
	// Handle window events regardless of current game state
	std::vector<sf::Event> events;
	while (const std::optional event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
			window.close();

		if (event->is<sf::Event::FocusLost>())
			isOutOfFocus = true;

		if (event->is<sf::Event::FocusGained>())
			isOutOfFocus = false;

		events.push_back(*event);
	}

	// Ignore input if the window is out of focus
	if (isOutOfFocus)
		return;

	// Process input for the current (topmost) state
	stateManager.processInput(window, events);
}

void Game::update(float fixedTimeStep)
{
	stateManager.update(fixedTimeStep);
}

void Game::render(float interpolationFactor)
{
	window.clear(sf::Color(30, 25, 60));

	stateManager.top()->applyView(window);
	stateManager.render(window, interpolationFactor);

	window.display();
}