// ================================================================================================
// File: Game.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <SFML/Graphics.hpp>
#include "Game.hpp"
#include "../state/PlayState.hpp"

Game::Game()
{
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 8;
	window.create(sf::VideoMode(sf::Vector2u(1200, 800)), PROJECT_NAME, sf::Style::Default, sf::State::Windowed, settings);
	window.setVerticalSyncEnabled(true);

	stateManager.push(std::make_unique<PlayState>(stateManager));
}

int Game::run()
{
	const float FIXED_TIME_STEP = 1.f / 60.f; // Fixed time step per update
	sf::Clock clock;						  // Clock to measure time
	float timeSinceLastUpdate = 0.f;		  // Time accumulator for fixed timestep
	float interpolationFactor = 0.f;		  // Interpolation factor for rendering

	while (window.isOpen())
	{
		timeSinceLastUpdate += clock.restart().asSeconds();
		processInput();

		while (timeSinceLastUpdate >= FIXED_TIME_STEP)
		{
			update(FIXED_TIME_STEP);
			timeSinceLastUpdate -= FIXED_TIME_STEP;
		}

		interpolationFactor = timeSinceLastUpdate / FIXED_TIME_STEP;
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

		events.push_back(*event);
	}

	stateManager.processInput(window, events);
}

void Game::update(float fixedTimeStep)
{
	stateManager.update(fixedTimeStep);
}

void Game::render(float interpolationFactor)
{
	window.clear();

	stateManager.top()->applyView(window);
	stateManager.render(window, interpolationFactor);

	window.display();
}